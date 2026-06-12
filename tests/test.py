import idaapi
import greffe
import ida_segment, ida_segregs, ida_auto, ida_idp
import idc, ida_name, ida_funcs, ida_bytes
import shutil

def set_patch_region():
    seg = ida_segment.get_segm_by_name(".rwx")
    greffe.set_region(seg.start_ea, seg.end_ea)

def export_patched(out_path):
    shutil.copy2(idc.get_input_file_path(), out_path)
    with open(out_path, "r+b") as f:
        def patch(ea, fpos, org_val, new_val):
            if fpos >= 0:
                f.seek(fpos)
                f.write(bytes([new_val & 0xFF]))
            return 0
        ida_bytes.visit_patched_bytes(0, idc.BADADDR, patch)

def ensure_arm_mode(func):
    t_reg = ida_idp.str2reg("T")
    if t_reg < 0 or ida_segregs.get_sreg(func.start_ea, t_reg) == 0:
        return
    ida_segregs.split_sreg_range(func.start_ea, t_reg, 0, ida_segregs.SR_user)
    ida_auto.plan_and_wait(func.start_ea, func.end_ea)

def instrument_function(func_name, force_arm=False):
    print(f"---> {func_name}")
    ea = ida_name.get_name_ea(idc.BADADDR, func_name)
    if ea == idc.BADADDR:
        print(f"[!] symbol not found: {func_name}, skipping")
        return
    func = ida_funcs.get_func(ea)

    if func is None:
        print(f"[~] no function at {hex(ea)}, forcing analysis...")
        idc.create_insn(ea)
        ida_funcs.add_func(ea)
        ida_auto.plan_and_wait(ea, ea + 1)
        ida_auto.auto_wait()
        func = ida_funcs.get_func(ea)
    if func is None:
        print(f"[!] could not create function at {hex(ea)} ({func_name}), skipping")
        return

    if force_arm:
        ensure_arm_mode(func)

    instr_addr = []
    while ea < func.end_ea:
        print(f"EA: {hex(ea)}")
        instr_addr.append(ea)
        ea = idc.next_head(ea, func.end_ea)

    greffe.add_instr(instr_addr)

def main():
    # argv[1]: output path
    # argv[2]: "--force-arm" to force ARM (non-thyumb) analysis 
    out_path  = idc.ARGV[1] if len(idc.ARGV) > 1 else "/tmp/patched_out"
    force_arm = len(idc.ARGV) > 2 and idc.ARGV[2] == "--force-arm"

    idaapi.auto_wait()
    set_patch_region()

    instrument_function("checksum", force_arm)
    instrument_function("xtea_block", force_arm)
    instrument_function("xtea_round", force_arm)

    greffe.apply_patches()
    export_patched(out_path)

main()
