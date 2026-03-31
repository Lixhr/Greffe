import subprocess
import re
import pytest
from CliTest import CliTest

GREFFE_PATH = "../greffe_cli/build/greffe"
NM          = "arm-none-eabi-nm"
BIN_DIR     = "bin/thumb"
ELF         = BIN_DIR + "/build/test.elf"
BIN         = BIN_DIR + "/build/__greffe_workdir/test.bin.greffe"

GREEN  = "\033[32m"
RED    = "\033[31m"
YELLOW = "\033[33m"
RESET  = "\033[0m"

TEST: list[CliTest] = [
    CliTest("set patch_base 0x00010000", r"patch_base\s*=\s*0x10000"),
    # CliTest("set patch_base 0x1100000", r"patch_base\s*=\s*0x1100000"),

    # =================== Just after a CODE XREF (OK) ===================
    # ROM:00000248 loc_248                                 ; CODE XREF: sub_230+38↓j
    # ROM:00000248                 MOV             R3, R10
    CliTest("add 0x00000248", r"> 0x00000248"),



    # =================== PC-Relative instruction (OK) ===================
    # ROM:0000023E                 LDR.W           R9, =0x9E3779B9
    CliTest("add 0x0000023E", r"> 0x0000023e"),



    # =================== CODE XREF overwrite (KO) ===================
    # ROM:00000246                 LDR             R7, =0xC6EF3720
    # ROM:00000248 loc_248                                 ; CODE XREF: sub_230+38↓j
    # ROM:00000248                 MOV             R3, R10
    CliTest("add 0x00000246", r"which is a code xref target"),


    # =================== Conditionnal branch (OK) ===================
    # Two target instructions
    # ROM:00000286                 SUBS            R5, R1, #0
    # ROM:00000288                 BLE             loc_2E2
    CliTest("add 0x00000286", r"> 0x00000286"),


    # =================== End of function: overlap (KO) ===================
    # ROM:000002F6                 POP             {R4,PC}
    CliTest("add 0x000002F6", r"exceeds branch instruction"),



    # =================== End of function: instr big enough (OK) ===================
    # ROM:00000272                 POP.W           {R3-R11,PC}
    CliTest("add 0x00000272", r"> 0x00000272"),


    # =================== Data (KO) ===================
    # ROM:000002F8 aCrcOk          DCB "CRC OK",0
    CliTest("add 0x000002F8", r"does not exist"),
    CliTest("patch\ny", r"written to "),


    CliTest("patch\ny", r"written to "),
]


SESSIONS: dict[str, list[CliTest]] = {
    "TEST": TEST,
}

# =============================================================================

ALL_TESTS: list[tuple[str, CliTest]] = [
    (name, tc) for name, tests in SESSIONS.items() for tc in tests
]


def get_patch_base(nm: str, elf_path: str) -> str:
    result = subprocess.run([nm, "--defined-only", elf_path], capture_output=True)
    if result.returncode:
        raise Exception(result.stderr.decode())
    for line in result.stdout.split(b"\n"):
        if b"patch_base" in line:
            return "0x" + line.decode().split()[0]
    raise RuntimeError(f"patch_base not defined in {elf_path}")


def run_greffe(tests: list[CliTest]) -> dict[str, list[str]]:
    stdin = "\n".join(t._input for t in tests) + "\n"
    output = subprocess.run([GREFFE_PATH], input=stdin, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if output.returncode:
        raise Exception(f"greffe exited: {output.returncode}")
    return parse_output(output.stdout)


def parse_output(text: str) -> dict[str, list[str]]:
    ansi_stripped = re.sub(r"\x1b\[[0-9;]*m", "", text)
    lines = ansi_stripped.split("\n")

    segments: dict[str, list[str]] = {}
    current_cmd = None
    for line in lines:
        if "greffe>" in line:
            current_cmd = line.split("greffe>", 1)[1].strip()
            segments[current_cmd] = []
        elif current_cmd is not None:
            segments[current_cmd].append(line)
    return segments


@pytest.fixture(scope="module")
def greffe_output():
    patch_base = get_patch_base(NM, ELF)
    print(f"\n  {YELLOW}elf        :{RESET} {ELF}")
    print(f"  {YELLOW}patched    :{RESET} {BIN}")
    print(f"  {YELLOW}patch_base :{RESET} {patch_base}")
    return {name: run_greffe(tests) for name, tests in SESSIONS.items()}


@pytest.mark.parametrize(
    "session_name,tc",
    ALL_TESTS,
    ids=[f"{name}::{tc._input}" for name, tc in ALL_TESTS],
)
def test_cli_output(session_name: str, tc: CliTest, greffe_output: dict[str, dict[str, list[str]]]):
    lines = greffe_output[session_name].get(tc._input.splitlines()[0], [])
    matched = any(re.search(tc._expected, line) for line in lines)
    if matched:
        print(f"\n  {GREEN}PASS{RESET}  [{tc._input}] matched /{tc._expected}/")
    else:
        output_dump = "\n".join(f"    {line}" for line in lines)
        pytest.fail(
            f"\n  {RED}FAIL{RESET}  [{tc._input}]\n"
            f"  pattern : /{tc._expected}/\n"
            f"  output  :\n{output_dump}"
        )
