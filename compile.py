import subprocess
import glob
import os
import sys
import platform

"""
Batch compiler for the CnR open.mp project.

    python compile.py [workspace_dir]

Compiles gamemodes/CnR.pwn (the gamemode) and every filterscripts/*.pwn
with the bundled QAWNO pawncc. Only prints compiler output when a file
produces warnings or errors. On Windows the WINDOWS_COMPILER define is
added automatically (it switches the module include paths to backslashes).
"""

IS_MAC = platform.system() == "Darwin"
IS_WINDOWS = platform.system() == "Windows"


def pawncc_path(workspace_dir):
    pawncc_dir = os.path.join(workspace_dir, "qawno", "mac") if IS_MAC else os.path.join(workspace_dir, "qawno")
    return os.path.join(pawncc_dir, "pawncc.exe" if IS_WINDOWS else "pawncc")


def compile_file(workspace_dir, pwn_file, include_dirs, out_base):
    command = [
        pawncc_path(workspace_dir),
        "-;+", "-(+", "-\\", "-Z-",
        *[f"-i{d}" for d in include_dirs],
        "-d3", "-t4",
        f"-o{out_base}",
        pwn_file,
    ]
    if IS_WINDOWS:
        command.append("WINDOWS_COMPILER=1")

    name = os.path.basename(pwn_file)
    try:
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        output = (result.stdout or "") + (result.stderr or "")
        if result.returncode != 0 or "warning" in output.lower() or "error" in output.lower():
            print(f"Compiling: {name}")
            print(output)
            return result.returncode == 0 and "error" not in output.lower()
        return True
    except Exception as e:
        print(f"An error occurred while trying to compile {name}: {e}")
        return False


def main(workspace_dir):
    qawno_include = os.path.join(workspace_dir, "qawno", "include")
    gamemodes_dir = os.path.join(workspace_dir, "gamemodes")
    ok = True

    # Gamemode
    gamemode = os.path.join(gamemodes_dir, "CnR.pwn")
    if os.path.isfile(gamemode):
        ok &= compile_file(
            workspace_dir,
            gamemode,
            [gamemodes_dir, qawno_include],
            os.path.join(gamemodes_dir, "CnR"),
        )
    else:
        print("gamemodes/CnR.pwn not found.")
        ok = False

    # Filterscripts (none yet — compiled if/when added)
    filterscripts_dir = os.path.join(workspace_dir, "filterscripts")
    for pwn_file in glob.glob(os.path.join(filterscripts_dir, "*.pwn")):
        base = os.path.splitext(pwn_file)[0]
        ok &= compile_file(workspace_dir, pwn_file, [base, qawno_include], base)

    print("Done." if ok else "Finished with errors.")
    return 0 if ok else 1


if __name__ == "__main__":
    workspace = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.abspath(__file__))
    sys.exit(main(workspace))
