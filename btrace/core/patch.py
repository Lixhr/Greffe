import subprocess
import tempfile
import os
from btrace.ProjectInfo import ProjectInfo
from btrace.target import Target
from btrace.ProjectInfo import Segment
from btrace.core.asm import AsmInstr
from prompt_toolkit import prompt
from pathlib import Path
from btrace.core.asm.AsmEngine import AsmEngine
from btrace.context import BTraceContext

class Img:
    raw_bytes: bytes
    base_segment: Segment

    def __init__(self, pinfo: ProjectInfo):
        self.raw_bytes = self._get_image(pinfo.bin_path)
        self.base_segment = pinfo.get_image_segment()
        pass

    def _get_image(self, infile: str):
        try:
            with open(infile, "rb") as img:
                return img.read()
        except OSError as e:
            raise Exception(f"{e.filename}: {e.strerror}")
        except Exception:
            raise

    def addr_to_offset(self, addr: int):
        return (addr - self.base_segment.start)

    def offset_to_addr(self, offset: int):
        return (self.base_segment.start + offset)

class AMethod:
    target_dir: str

    def __init__(self, workdir: Path, asm: AsmEngine):
        self.workdir  = workdir
        self.asm      = asm
        self.src_path = workdir / self.target_dir
        self.bin_path = self.src_path / "build" / "payload.bin"
        self.elf_path = self.src_path / "build" / "elf.bin"

    def make(self):
        makeflags = ["make", "-C", str(self.workdir), f"MODE={self.target_dir}"]
        arch_flags = self.asm.arch.gcc_flags()
        if arch_flags:
            joined = " ".join(arch_flags)
            makeflags.append(f"CPU_SPECIFIC={joined}")

        subprocess.run(
            makeflags,
            check=True,
            close_fds=False,
        )

    def get_bin(self) -> bytes:
        with open(self.bin_path, "rb") as f:
            return f.read()


class TraceMethod(AMethod):
    target_dir = "trace"


class CoverageMethod(AMethod):
    target_dir = "coverage"

class Patch:
    pinfo: ProjectInfo
    targets: list[Target]
    img: Img
    patch_base: int

    def __init__(self, pinfo : ProjectInfo, asm: AsmEngine, targets: list[Target]):
        self.pinfo = pinfo
        self.img = Img(self.pinfo)
        self._check_base_segment(targets)
        self.patch_base = self._ask_patch_address()

        TraceMethod(pinfo.btrace_workdir, asm).make()

    def _ask_patch_address(self) -> int:
        if self.pinfo.patch_base is not None:
            while True:
                print(f"Current patch base: {hex(self.pinfo.patch_base)}. Use it ? [y/n]")
                resp = prompt(" > ")
                if resp == "y": 
                    return self.pinfo.patch_base
                elif resp == "n":
                    break

        print("Please select the patch's base address (hex, e.g., 0x123DD8):")
        base = int(prompt(" > "), 16)

        aligned_base = (base + 0xF) & ~0xF
        if aligned_base != base:
            print(f"Aligned patch base: {hex(aligned_base)}")

        self.pinfo.patch_base = aligned_base
        return aligned_base


    def _check_base_segment(self, targets: list[Target]):
        for i, t in enumerate(targets):
            if i == 3: ##  enough ?
                break
            for instr in t.asm_ctx:
                offset = self.img.addr_to_offset(instr.ea)
                img_slice = self.img.raw_bytes[offset: offset + instr.size]

                if (img_slice != instr.raw_bytes):
                    raise Exception(f"Ida / infile mismatch at address {hex(instr.ea)}. You may have chosen the wrong base segment")

