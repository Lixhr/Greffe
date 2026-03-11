
from capstone import CsInsn

class AsmInstr:
    def __init__(self, cs_instr: CsInsn, mode: str | None = None, patched: bool = False):
        self._instr   = cs_instr
        self.mode     = mode
        self.patched  = patched
        self.ea       = cs_instr.address
        self.raw      = cs_instr.bytes.hex()
        self.mnemonic = cs_instr.mnemonic
        self.op_str   = cs_instr.op_str

    def __getattr__(self, name):
        return getattr(self._instr, name)

    def __str__(self):
        return f"{self.mnemonic} {self.op_str}"
    
    def to_dict(self):
        return {
            "ea": self.ea,
            "raw": self.raw,
            "mode": self.mode
        }
