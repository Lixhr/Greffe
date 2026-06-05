"""
Install: copy this file to ~/.idapro/python/

Usage:
    import ida_auto, idc, greffe

    ida_auto.auto_wait()
    greffe.set_region(0x08001000, 0x08002000)
    greffe.add_instr(0x080011A4)
    greffe.add_instr(0x080011B8)
    greffe.apply_patches()
"""

import idc


def _idc(expr: str) -> int:
    result = idc.eval_idc(expr)
    if isinstance(result, str) and result.startswith("IDC_FAILURE"):
        raise RuntimeError(result)
    if result is None:
        raise RuntimeError(f"IDC call failed: {expr}")
    return int(result)


def set_region(start: int, end: int) -> bool:
    """Register [start, end) as a patch region."""
    return bool(_idc(f"GreffSetRegion({start}, {end})"))


def add_instr(ea: int, handler: str = "") -> bool:
    """Hook the instruction at ea, optionally reusing an existing handler by name."""
    if handler:
        return bool(_idc(f'GreffAddInstrEx({ea}, "{handler}")'))
    return bool(_idc(f"GreffAddInstr({ea})"))


def apply_patches() -> bool:
    """Compile all handlers and write patches to the IDB (equivalent of Shift+P)."""
    return bool(_idc("GreffApplyPatches()"))
