#!/usr/bin/env python3
# Deterministic converter: legacy scriptfiles data -> SQL seed for `houses` + `LSvehicles`.
# Run:  python scriptfiles/_seed_world.py   (from repo root)
import re, os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SF   = os.path.join(ROOT, "scriptfiles")
PROP = os.path.join(SF, "properties")
VEH  = os.path.join(SF, "vehicles")

def lead_num(s):
    m = re.match(r'\s*(-?\d+(?:\.\d+)?)', s or '')
    return m.group(1) if m else None
def esc(s): return s.replace("'", "''")

# ---- vehicle model names (model-400 index) parsed from the gamemode ----
def load_vehicle_names():
    txt = open(os.path.join(ROOT, "gamemodes", "CnR", "server", "misc.inc"), encoding="utf-8", errors="replace").read()
    m = re.search(r'VehicleNames\[\d+\]\[\]\s*=\s*\{(.*?)\};', txt, re.S)
    if not m: return {}
    names = re.findall(r'"([^"]*)"', m.group(1))
    return {400 + i: n for i, n in enumerate(names)}

# ---- interiors.txt: index -> (samp_interior_id, x, y, z, name) ----
def load_interiors():
    interiors = {}
    for line in open(os.path.join(PROP, "interiors.txt"), encoding="utf-8", errors="replace"):
        line = line.strip().rstrip(';').strip()
        if not line: continue
        p = line.split()
        if len(p) < 6: continue
        try:
            idx = int(p[0]); samp = int(p[1]); x=float(p[2]); y=float(p[3]); z=float(p[4])
        except ValueError: continue
        name = " ".join(p[6:]) if len(p) > 6 else ""
        interiors[idx] = (samp, x, y, z, name)
    return interiors

# ---- house price by interior class (spread + small per-house variation) ----
def house_price(interior_idx, name, seq):
    n = name.lower()
    if interior_idx == 55 or "mansion" in n:            base = 2500000
    elif interior_idx == 43 or "ryder" in n:            base = 1200000
    elif "safe house" in n or "safehouse" in n:         base = 900000
    elif "burglary" in n:                               base = 400000
    else:                                               base = 300000
    return base + (seq % 6) * 25000

def build_houses(interiors):
    rows = []
    seq = 0
    for line in open(os.path.join(PROP, "houses.txt"), encoding="utf-8", errors="replace"):
        raw = line.split(';')[0].strip()          # drop trailing "; // comment"
        if not raw: continue
        p = raw.split(',')
        if len(p) < 7: continue
        try:
            x=float(lead_num(p[1])); y=float(lead_num(p[2])); z=float(lead_num(p[3]))
            iidx=int(lead_num(p[5]))
        except (TypeError, ValueError): continue
        it = interiors.get(iidx)
        if not it:  # skip a house whose interior we can't resolve
            print(f"  ! house at {x:.1f},{y:.1f} references unknown interior index {iidx} - skipped")
            continue
        samp, ix, iy, iz, iname = it
        price = house_price(iidx, iname, seq); seq += 1
        rows.append((x, y, z, samp, ix, iy, iz, price))
    return rows

# ---- LS + surrounding-countryside vehicle files (SF/LV city cores excluded: those cities are inactive) ----
VEH_FILES = [
    ("ls_gen_inner.txt", 0, "Los Santos"),
    ("ls_gen_outer.txt", 0, "Los Santos"),
    ("ls_law.txt",       2, "Los Santos PD"),
    ("ls_airport.txt",   0, "LS Airport"),
    ("red_county.txt",   0, "Red County"),
    ("flint.txt",        0, "Flint County"),
    ("whetstone.txt",    0, "Whetstone"),
    ("tierra.txt",       0, "Tierra Robada"),
    ("bone.txt",         0, "Bone County"),
]
def build_vehicles(names):
    rows = []
    for fname, vtype, area in VEH_FILES:
        path = os.path.join(VEH, fname)
        if not os.path.isfile(path): continue
        cnt = 0
        for line in open(path, encoding="utf-8", errors="replace"):
            raw = line.split(';')[0].strip()
            if not raw: continue
            p = raw.split(',')
            if len(p) < 7: continue
            try:
                model=int(lead_num(p[0])); x=float(lead_num(p[1])); y=float(lead_num(p[2]))
                z=float(lead_num(p[3])); a=float(lead_num(p[4]))
                c1=int(lead_num(p[5])); c2=int(lead_num(p[6]))
            except (TypeError, ValueError): continue
            if not (400 <= model <= 611): continue
            rows.append((names.get(model, "Vehicle"), area, vtype, model, x, y, z, a, c1, c2))
            cnt += 1
        print(f"  {fname}: {cnt} vehicles ({'cop' if vtype==2 else 'civ'}, {area})")
    return rows

def main():
    names = load_vehicle_names()
    interiors = load_interiors()
    print(f"Parsed {len(names)} vehicle names, {len(interiors)} interiors.")
    houses = build_houses(interiors)
    print(f"Houses: {len(houses)}")
    vehicles = build_vehicles(names)
    print(f"Vehicles: {len(vehicles)} total")

    # Vehicles seed (LSvehicles). Houses come from _parse_odegay.py -> seed_houses_big.sql;
    # the base 141 from houses.txt above are only reported, not emitted (the 839-set supersedes them).
    out = os.path.join(SF, "seed_vehicles.sql")
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("-- Auto-generated LS-region vehicle seed from scriptfiles/vehicles/ (stock SA-MP set).\n")
        f.write("-- Regenerate: python scriptfiles/_seed_world.py\n")
        f.write("-- Load: mysql cnr < scriptfiles/seed_vehicles.sql  (TRUNCATEs then re-inserts LSvehicles)\n\n")
        f.write("TRUNCATE TABLE `LSvehicles`;\n")
        f.write("INSERT INTO `LSvehicles` (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES\n")
        f.write(",\n".join(
            f"('{esc(nm)}','{esc(area)}',{vt},{model},{x:.4f},{y:.4f},{z:.4f},{a:.4f},{c1},{c2})"
            for (nm,area,vt,model,x,y,z,a,c1,c2) in vehicles) + ";\n")
    print(f"\nWrote {out}\n  {len(vehicles)} vehicles ({len(houses)} local base houses reported only; use seed_houses_big.sql).")

if __name__ == "__main__":
    main()
