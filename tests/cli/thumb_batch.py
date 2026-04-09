import subprocess
import re
import sys

GREFFE_PATH = "../greffe_cli/build/greffe"

GREEN  = "\033[32m"
RED    = "\033[31m"
YELLOW = "\033[33m"
RESET  = "\033[0m"

commands = (
    ["0x1000"]
    # + [f"add 0x{ea:08x}" for ea in range(0x280, 0x284 + 1, 2)]
    + [f"add 0x{ea:08x}" for ea in range(0x280, 0x2E4 + 1, 2)]
    # + [f"add 0x{ea:08x}" for ea in range(0x230, 0x272 + 1, 2)]
    + ["patch", "y"]
)

stdin = "\n".join(commands) + "\n"
result = subprocess.run(
    [GREFFE_PATH],
    input=stdin,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    text=True,
)

print(result.stdout)

if result.returncode:
    print(f"{RED}greffe exited with code {result.returncode}{RESET}")
    sys.exit(1)

if "written to" in re.sub(r"\x1b\[[0-9;]*m", "", result.stdout):
    print(f"{GREEN}OK{RESET}")
else:
    print(f"{RED}FAIL: patch did not complete{RESET}")
    sys.exit(1)