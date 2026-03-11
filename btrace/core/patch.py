import subprocess
import tempfile
import os

def bytes_to_obj(raw: bytes, symbol: str, output_path: str):
    hex_bytes = ", ".join(f"0x{b:02x}" for b in raw)
    asm = f"""
    .global {symbol}
    .type   {symbol}, %function
    .section .text
{symbol}:
    .byte {hex_bytes}
"""
    with tempfile.NamedTemporaryFile(suffix=".s", delete=False, mode="w") as f:
        f.write(asm)
        tmp_s = f.name

    tmp_o = tmp_s.replace(".s", ".o")
    try:
        subprocess.run(["as", "-o", tmp_o, tmp_s], check=True)
        os.rename(tmp_o, output_path)
    finally:
        os.unlink(tmp_s)
