from typing import Callable

from capstone.arm import ARM_OP_REG, ARM_OP_MEM, ARM_REG_PC, ARM_INS_LDR, ARM_OP_IMM
from btrace.core.asm.AsmInstr import AsmInstr
from capstone import CS_MODE_ARM, CS_ARCH_ARM, CS_MODE_THUMB
from keystone import KS_MODE_ARM, KS_MODE_THUMB, KS_ARCH_ARM
from btrace.core.asm.AArch import AArch

def reloc_get_target(instr: AsmInstr) -> int:
    for op in instr._instr.operands:
        if op.type == ARM_OP_MEM and op.mem.base == ARM_REG_PC:
            pc = (instr.ea + 4)
            if instr.mode == "thumb":
                pc &= ~3
            return pc + op.mem.disp

        if op.type == ARM_OP_IMM:
            return op.imm
        
def relocate_ldr(instr: AsmInstr) -> tuple[str, int]:
    target = reloc_get_target(instr)
    dst    = instr._instr.reg_name(instr._instr.operands[0].reg)
    alignment = "2" if instr.mode == "thumb" else "4"

    asm = (
        f"ldr {dst}, pool\n"
        f"b end\n"
        f".align {alignment}\n"
        f"pool: .word 0x{target:08x}\n"
        f"end:\n"
    )
    return asm

class Arm(AArch):
    CS_ARCH      = CS_ARCH_ARM
    KS_ARCH      = KS_ARCH_ARM
    DEFAULT_MODE = "arm"
    BASE_MODES   = {
        "arm":   (CS_MODE_ARM,   KS_MODE_ARM,   4, ["-mthumb-interwork"]),
    }
    SUB_MODES    = {
        "thumb": (CS_MODE_THUMB, KS_MODE_THUMB, 4, ["-mthumb", "-mthumb-interwork"]),
    }

    relocators = {
            ARM_INS_LDR: relocate_ldr,
    }

    def _jmp_instr(self, pc_offset: int) -> str:
        return f"b {pc_offset}"
    
    def gcc_flags(self) -> list[str] | None:
        return self._get_mode("thumb").gcc_flags

    def save_context(self, mode: bool | str = False) -> bytes:
        return self._get_mode(mode).assemble("push {r0-r12, lr}")

    def restore_context(self, mode: bool | str = False) -> bytes:
        return self._get_mode(mode).assemble("pop {r0-r12, lr}")

    def is_pc_relative(self, instr : AsmInstr) -> bool:
        for op in instr._instr.operands:
            if op.type == ARM_OP_REG and op.reg == ARM_REG_PC:
                return True
            if op.type == ARM_OP_MEM and op.mem.base == ARM_REG_PC:
                return True
        return False
