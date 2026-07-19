#!/usr/bin/env python3
# Parse odegay/samp-server fill_property.sql -> validate + classify houses; optionally emit a seed.
import re, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC  = os.path.join(HERE, "_house_research", "odegay_fill_property_houses.sql")

def rows_in_houses_blocks(text):
    """Yield the inner text of each (...) row that belongs to an `INSERT INTO houses` block."""
    for m in re.finditer(r'INSERT INTO `houses`.*?VALUES(.*?);', text, re.S):
        block = m.group(1)
        for rm in re.finditer(r'\(([^()]*)\)', block):
            yield rm.group(1)

def split_sql(inner):
    # split on commas not inside quotes
    return [p.strip().strip("'").strip() for p in re.split(r",(?=(?:[^']*'[^']*')*[^']*$)", inner)]

def region(x, y):
    if x < -1000:                                   return "SF"          # San Fierro (inactive)
    if 1300 < x < 2950 and 850 < y < 2950:          return "LV"          # Las Venturas (inactive)
    if 100 < x < 3000 and -3000 < y < -600:         return "LS"          # Los Santos (active hub)
    return "COUNTRY"                                                     # shared countryside

def main():
    text = open(SRC, encoding="utf-8", errors="replace").read()
    houses, bad, byreg, interiors = [], 0, {}, {}
    for inner in rows_in_houses_blocks(text):
        f = split_sql(inner)
        if len(f) < 18: continue
        try:
            ex, ey, ez = float(f[1]), float(f[2]), float(f[3])        # exterior door
            ix, iy, iz = float(f[4]), float(f[5]), float(f[6])        # interior spawn
            cost = int(float(f[14])); interior = int(float(f[17]))
            ang = float(f[-1])
        except (ValueError, IndexError):
            continue
        # validity: interior spawn must be a real interior (Z well above ground), not (0,0,0)/void
        if not (900.0 < iz < 1200.0) or (abs(ix) < 1.0 and abs(iy) < 1.0):
            bad += 1; continue
        r = region(ex, ey)
        houses.append((ex, ey, ez, ix, iy, iz, interior, cost, ang, r))
        byreg[r] = byreg.get(r, 0) + 1
        interiors[interior] = interiors.get(interior, 0) + 1

    print(f"Parsed valid houses: {len(houses)}   (rejected {bad} with void/invalid interior coords)")
    print("By region:", ", ".join(f"{k}={v}" for k, v in sorted(byreg.items())))
    print("Distinct interior ids used:", len(interiors), "->", dict(sorted(interiors.items(), key=lambda kv:-kv[1])[:12]))
    costs = sorted(h[7] for h in houses)
    print(f"Cost range: ${costs[0]:,} .. ${costs[-1]:,}   median ${costs[len(costs)//2]:,}")

    if len(sys.argv) > 1 and sys.argv[1] == "--emit":
        # keep ALL validated houses across San Andreas (LS + countryside + SF + LV) —
        # remote/hidden safehouses in the inactive cities are wanted (players like them).
        keep = list(houses)
        out = os.path.join(HERE, "seed_houses_big.sql")
        with open(out, "w", encoding="utf-8", newline="\n") as o:
            o.write("-- Expanded house set (odegay/samp-server fill_property.sql, ALL San Andreas, validated).\n")
            o.write("-- Regenerate: python scriptfiles/_parse_odegay.py --emit\n")
            o.write("TRUNCATE TABLE `houses`;\n")
            o.write("INSERT INTO `houses` (X,Y,Z,Interior,IntX,IntY,IntZ,Price,ForSale,OwnerAID) VALUES\n")
            def price(c):
                return max(150000, min(3000000, c if c > 0 else 300000))
            o.write(",\n".join(
                f"({ex:.4f},{ey:.4f},{ez:.4f},{interior},{ix:.4f},{iy:.4f},{iz:.4f},{price(cost)},1,0)"
                for (ex,ey,ez,ix,iy,iz,interior,cost,ang,r) in keep) + ";\n")
        print(f"\nEmitted {len(keep)} LS+countryside houses -> {out}")

if __name__ == "__main__":
    main()
