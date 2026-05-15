import idaapi
import greffe
import ida_segment
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

def main():
    idaapi.auto_wait()

    set_patch_region()
    ea = ida_name.get_name_ea(idc.BADADDR, "checksum")
    func = ida_funcs.get_func(ea)

    instr_addr = []
    while ea < func.end_ea:
        print(f"EA: {hex(ea)}")
        instr_addr.append(ea)
        ea = idc.next_head(ea, func.end_ea)

    for i, ea in enumerate(instr_addr):
        print(f"============> {i}")
        # if i == 31: ##probleme a partir de 31;
            # break
        greffe.add_instr(ea)

    greffe.apply_patches()
    export_patched("/tmp/patched_out")

main()