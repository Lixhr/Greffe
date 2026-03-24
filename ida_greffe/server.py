import threading
import socket
import struct
import json
import idaapi
import ida_funcs
from abc import ABC, abstractmethod
import idautils
import ida_xref
import idc
import ida_bytes
import ida_idp
import ida_segment
import time

SOCKET_PATH = "/tmp/greffe.sock"

class IDAException(Exception):
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)

def get_func_by_name(name: str):
    ea = idaapi.get_name_ea(idaapi.BADADDR, name)
    if ea == idaapi.BADADDR:
        return None
    return ida_funcs.get_func(ea)

def get_func_by_address(ea: int):
    return idaapi.get_func(ea)

def get_mode_context(ea: int) -> str | None:
    arch = ida_idp.get_idp_name().lower()

    match arch:
        case "arm":
            return ("thumb" if idc.get_sreg(ea, "T") else None)
    return None

def get_instruction(ea: int):
    insn = idaapi.insn_t()

    if idaapi.decode_insn(insn, int(ea)) == 0:
        raise RuntimeError(f"failed to decode instruction at {hex(ea)}")

    raw = ida_bytes.get_bytes(ea, insn.size)
    if raw is None:
        raise RuntimeError(f"failed to read bytes at {hex(ea)}")

    return {
        "ea": ea,
        "raw": raw.hex(),
        "size": insn.size, 
        "mode": get_mode_context(ea)
    }

def get_segments():
    segments = []
    for i in range(ida_segment.get_segm_qty()):
        seg = ida_segment.getnseg(i)

        segments.append({
            "name": ida_segment.get_segm_name(seg),
            "start":  seg.start_ea,
            "end": seg.end_ea
        })

    return segments

def get_asm_context(func, ea: int):
    before = []
    target = []
    after  = []

    prev_ea = idc.prev_head(ea, func.start_ea)
    if prev_ea != idaapi.BADADDR:
        before.append(get_instruction(prev_ea))

    target_instr = get_instruction(ea)
    target.append(target_instr)
    collected = target_instr["size"]

    cur_ea = int(ea + target_instr["size"])
    first_after = True
    while first_after or collected < 10:
        if cur_ea >= func.end_ea:
            break
        if ida_bytes.is_data(ida_bytes.get_full_flags(cur_ea)):
            break
        instr = get_instruction(cur_ea)
        after.append(instr)
        collected += instr["size"]
        cur_ea += instr["size"]
        first_after = False

    return before + target + after

def recv_msg(conn) -> dict:
    raw_len = conn.recv(4)
    if not raw_len or len(raw_len) < 4:
        raise ConnectionError("disconnected")
    length = struct.unpack(">I", raw_len)[0]

    data = b""
    while len(data) < length:
        chunk = conn.recv(length - len(data))
        if not chunk:
            raise ConnectionError("disconnected")
        data += chunk

    return json.loads(data.decode())

def send_msg(conn, msg: dict):
    data = json.dumps(msg).encode()
    conn.sendall(struct.pack(">I", len(data)) + data)

class AIPCCommand(ABC):
    action: str

    @abstractmethod
    def handle(self, msg: dict) -> dict: ...

class IPCAdd(AIPCCommand):
    action = "add"

    def handle(self, body: list[str]) -> dict:
        body_rsp = []
        for target in body:
            if target.startswith("0x"):
                addr = int(target, 16)
                func = get_func_by_address(addr)
                if func is None:
                    raise IDAException(f"{target} does not exist")
                target = f"{func.name}+{addr - func.start_ea}"
            else:
                func = get_func_by_name(target)
                if func is None:
                    raise IDAException(f"{target} does not exist")
                addr = func.start_ea

            if func is None:
                raise IDAException(f"no function at {target}")

            body_rsp.append({
                "name":    target,
                "ea":      addr,
                "end_ea":  func.end_ea,
                "context": get_asm_context(func, addr),
            })
        return {"ok": True, "body": body_rsp}

class IPCProjectInfo(AIPCCommand):
    action = "info"

    def handle(self, body):
        return {"ok": True, "body": {
            "bin_path":   idaapi.get_input_file_path(),
            "arch":       ida_idp.get_idp_name().lower(),
            "endianness": "be" if idaapi.inf_is_be() else "le",
            "bits":       64 if idaapi.inf_is_64bit() else 32,
            "bin_base":   idaapi.get_imagebase(),
            "segments":   get_segments()
        }}

class IPCRefresh(AIPCCommand):
    action = "refresh"

    def handle(self, body):
        from ida_greffe.core import Greffe
        pending = Greffe().pop_pending()
        if not pending:
            return {"ok": True, "body": {"targets": []}}

        targets = []
        for ea in pending:
            func = get_func_by_address(ea)
            if func is None:
                print(f"[greffe] refresh: no function at {hex(ea)}, skipping")
                continue
            base_name = idaapi.get_func_name(func.start_ea) or hex(func.start_ea)
            name = base_name if ea == func.start_ea else f"{base_name}+{ea - func.start_ea:#x}"
            try:
                targets.append({
                    "name":    name,
                    "ea":      ea,
                    "end_ea":  func.end_ea,
                    "context": get_asm_context(func, ea),
                })
            except Exception as e:
                print(f"[greffe] refresh: error for {hex(ea)}: {e}")

        return {"ok": True, "body": {"targets": targets}}


class Server(threading.Thread):
    def __init__(self):
        super().__init__(daemon=True)
        self._stop     = threading.Event()
        self._commands = {cmd.action: cmd for cmd in [
            IPCAdd(),
            IPCProjectInfo(),
            IPCRefresh(),
        ]}

    def stop(self):
        self._stop.set()

    def run(self):
        time.sleep(1)
        import os
        if os.path.exists(SOCKET_PATH):
            os.unlink(SOCKET_PATH)

        srv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        srv.bind(SOCKET_PATH)
        srv.listen(1)
        srv.settimeout(0.5)

        print("[greffe] server started")

        while not self._stop.is_set():
            try:
                conn, _ = srv.accept()
            except TimeoutError:
                continue

            print("[greffe] client connected")
            try:
                self._handle_client(conn)
            except ConnectionError:
                print("[greffe] client disconnected")
            finally:
                conn.close()

        srv.close()
        os.unlink(SOCKET_PATH)
        print("[greffe] server stopped")

    def _handle_client(self, conn):
        while not self._stop.is_set():
            try:
                msg = recv_msg(conn)

                response = {}
                idaapi.execute_sync(
                    lambda: response.update(self._dispatch(msg)),
                    idaapi.MFF_READ
                )
                send_msg(conn, response)

            except ConnectionError:
                raise

            except Exception as e:
                print(f"[greffe] internal error: {e}")
                try:
                    send_msg(conn, {"ok": False, "body": str(e)})
                except:
                    raise

    def _dispatch(self, msg: dict) -> dict:
        action = msg.get("action")
        body   = msg.get("body")
        if not isinstance(action, str):
            return {"ok": False, "body": "action must be a string"}
        try:
            cmd = self._commands.get(action)
            if not cmd:
                raise IDAException(f"unknown action: {action}")
            return cmd.handle(body)
        except IDAException as e:
            return {"ok": False, "body": e.message}
        except Exception as e:
            return {"ok": False, "body": str(e)}