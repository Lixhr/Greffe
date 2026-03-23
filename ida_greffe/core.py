import idaapi
import idc
import ida_kernwin
import threading
from ida_greffe import server

class Greffe:
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        if hasattr(self, "_initialized"):
            return
        self._initialized  = True
        self._pending      = []
        self._pending_lock = threading.Lock()
        self.server = server.Server()
        self.server.start()
        print("Greffe init")

    def add_pending(self, ea: int):
        with self._pending_lock:
            if ea not in self._pending:
                self._pending.append(ea)

    def pop_pending(self) -> list:
        with self._pending_lock:
            pending = list(self._pending)
            self._pending.clear()
        return pending

    def term(self):
        self.server.stop()
        print("Greffe killed")
