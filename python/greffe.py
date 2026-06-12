"""
Install: copy this file to ~/.idapro/python/

Usage:
    import ida_auto, idc, greffe

    ida_auto.auto_wait()
    greffe.set_region(0x08001000, 0x08002000)
    greffe.add_instr([0x080011A4, 0x080011B8])
    greffe.apply_patches()
"""

import json
import idc
from dataclasses import dataclass


@dataclass
class Greffe:
    name: str
    handler: str
    ea: int

def _idc(expr: str) -> int:
    result = idc.eval_idc(expr)
    if isinstance(result, str) and result.startswith("IDC_FAILURE"):
        raise RuntimeError(result)
    if result is None:
        raise RuntimeError(f"IDC call failed: {expr}")
    return int(result)

def _idc_str(expr: str) -> str:
    result = idc.eval_idc(expr)
    if isinstance(result, str) and result.startswith("IDC_FAILURE"):
        raise RuntimeError(result)
    if result is None:
        raise RuntimeError(f"IDC call failed: {expr}")
    return result

def set_region(start: int, end: int) -> bool:
    """Register [start, end) as a patch region."""
    return bool(_idc(f"GreffeSetRegion({start}, {end})"))

def add_instr(eas: list[int], handler: str = "") -> bool:
    """Hook the instructions at the given addresses, optionally reusing an existing handler by name."""
    args = ", ".join(str(ea) for ea in eas)
    if handler:
        return bool(_idc(f'GreffeAddEx("{handler}", {args})'))
    return bool(_idc(f"GreffeAdd({args})"))

def del_instr(eas: list[int]) -> bool:
    """Delete the greffes at the given addresses."""
    args = ", ".join(str(ea) for ea in eas)
    return bool(_idc(f"GreffeDel({args})"))

def clear() -> bool:
    """Clears all the modifications"""
    return bool(_idc(f"GreffeClear()"))

def apply_patches() -> bool:
    """Compile all handlers and write patches to the IDB (equivalent of Shift+P)."""
    return bool(_idc("GreffeApplyPatches()"))

def get_greffes() -> list[Greffe]:
    """Return all registered greffes."""
    raw = _idc_str("GreffeGetArray()")
    return [Greffe(name=g["name"], handler=g["handler"], ea=int(g["ea"], 16))
            for g in json.loads(raw)]
