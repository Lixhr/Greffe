import idaapi
import idautils
import idc
import importlib
import sys
import ida_greffe.core
import ida_greffe.gui
import ida_greffe.ida_reloader
import threading

PLUGIN_NAME    = "Greffe"
PLUGIN_VERSION = "1.0.0"
PLUGIN_AUTHOR  = "Lixhr"

PLUGIN_INSTANCE = None

class ReloadAction(idaapi.action_handler_t):
    ACTION_ID = "greffe:reload"

    def activate(self, ctx):
        global PLUGIN_INSTANCE

        if PLUGIN_INSTANCE and PLUGIN_INSTANCE.hook:
            PLUGIN_INSTANCE.hook.unhook()
            PLUGIN_INSTANCE.hook.term()
            PLUGIN_INSTANCE.hook = None
        ida_greffe.core.Greffe().term()
        ida_greffe.ida_reloader.reload_package("ida_greffe")

        ## restart services
        ida_greffe.core.Greffe() 
        PLUGIN_INSTANCE.hook = ida_greffe.gui.GUITracerHook()
        PLUGIN_INSTANCE.hook.ready_to_run()
        

    def update(self, ctx):
        return idaapi.AST_ENABLE_ALWAYS

class GreffePlugin(idaapi.plugin_t):
    flags = idaapi.PLUGIN_KEEP
    wanted_name = PLUGIN_NAME
    # wanted_hotkey = "
    # Shift-b"
    hook = None

    def init(self):
        global PLUGIN_INSTANCE
        PLUGIN_INSTANCE = self
        idaapi.register_action(idaapi.action_desc_t(
            ReloadAction.ACTION_ID,
            "Greffe: Reload",
            ReloadAction(),
            "Shift+r",
            "Dev: reload plugin",
            -1
        ))
        self.hook = ida_greffe.gui.GUITracerHook()


        print(f"[{PLUGIN_NAME}] v{PLUGIN_VERSION} loaded")
        return idaapi.PLUGIN_KEEP


    def term(self):
        idaapi.unregister_action(ReloadAction.ACTION_ID)
        if self.hook:
            self.hook.term()
        print(f"[{PLUGIN_NAME}] unloaded")

def PLUGIN_ENTRY():
    return GreffePlugin()