"""
IDA_DIR=/path/to/ida python3 instrument.py

IDA_DIR must point at the IDA install containing idat (defaults to ~/.ida-pro-9.2).
"""

import os
import subprocess
import sys

try:
    import idaapi
    import ida_segment, ida_segregs, ida_auto, ida_idp
    import idc, ida_name, ida_funcs, ida_bytes
    import greffe
    import shutil

    IN_IDA = True
except ImportError:
    IN_IDA = False


if not IN_IDA:
    from targets import TARGETS, PATCHED_DIR

    def idat_path():
        ida_dir = os.environ.get("IDA_DIR", os.path.expanduser("~/.ida-pro-9.2"))
        path = os.path.join(ida_dir, "idat")
        if not os.path.isfile(path):
            sys.exit(f"idat not found at {path} (set IDA_DIR)")
        return path

    def instrument(target, idat):
        log_path = f"/tmp/greffe_instrument_{target.arch}.log"
        sarg = f"{os.path.abspath(__file__)} {target.patched}"
        if target.force_arm:
            sarg += " --force-arm"
        cmd = [idat, f"-L{log_path}", "-P-", "-A", f"-S{sarg}", target.binary]

        print(f"=== [{target.arch}] instrumenting ===")
        if not os.path.isfile(target.binary):
            print(f"[!] missing binary: {target.binary} (run `make -C tests re` first)")
            return False

        proc = subprocess.run(cmd)
        ok = proc.returncode == 0 and os.path.isfile(target.patched)
        if not ok:
            print(f"[!] instrumentation failed for {target.arch}, see {log_path}")
            if os.path.isfile(log_path):
                print(open(log_path).read())
        return ok

    def run():
        os.makedirs(PATCHED_DIR, exist_ok=True)
        idat = idat_path()
        return all([instrument(target, idat) for target in TARGETS])

    if __name__ == "__main__":
        sys.exit(0 if run() else 1)

else:

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
            instr_addr.append(ea)
            ea = idc.next_head(ea, func.end_ea)

        greffe.add_instr(instr_addr)

    def main():
        # argv[1]: output path
        # argv[2]: "--force-arm" to force ARM (non-thumb) analysis
        out_path = idc.ARGV[1] if len(idc.ARGV) > 1 else "/tmp/patched_out"
        force_arm = len(idc.ARGV) > 2 and idc.ARGV[2] == "--force-arm"

        idaapi.auto_wait()
        set_patch_region()

        instrument_function("checksum", force_arm)
        instrument_function("xtea_block", force_arm)
        instrument_function("xtea_round", force_arm)

        greffe.apply_patches()
        export_patched(out_path)

    main()