# Greffe

IDA plugin for instrumenting bare-metal binaries without debugging capabilities.

Select any instruction in IDA, Greffe replaces it with a branch to a user-written C handler. The original instruction is relocated and executed transparently after the handler returns.

Instruction relocation is powered by [frida-gum](https://github.com/frida/frida-gum). Currently supported architectures: Thumb.

---

## Installation

```
git clone --recursive https://github.com/Lixhr/Greffe.git
```

**IDA plugin** 

Copy `ida_greffe/` and `ida_entry.py` into your IDA plugins directory.

**CLI**
```sh
sudo apt-get install make g++ libreadline-dev libzmq3-dev nlohmann-json3-dev libelf-dev libglib2.0-dev
cd greffe_cli && make
```

Depending on your target, a cross-compiler must be available in your `$PATH`.

---

## Usage

### 1. Start

Open your binary in IDA, then load the plugin: **Edit -> Plugins -> Greffe**.

This starts the IDA-side socket server. Then, in a separate terminal, run the CLI:

```sh
./greffe_cli/build/greffe
```

The CLI connects to IDA and creates `__greffe_workdir/` next to the binary.

### 2. Add a greffe

**From IDA** - right-click any instruction -> *Add a Greffe* (or `Shift+G`). The greffe then appears in the CLI.

**From the CLI** - by function name or address:
```
greffe> add my_func
greffe> add 0x8004A2C
```

### 3. Write the handler

A C stub is automatically created in `__greffe_workdir/handlers/`:
```c
// handlers/my_func.c
void handler_my_func(void)
{
    // bare-metal context - no libc
}
```

You can add extra `.c` files in `handlers/`, all of them are compiled and linked into the same blob.

### 4. Patch

```
greffe> patch
```

Compiles all handlers and writes the patched binary alongside the original: `firmware.bin.greffe`. Flash it to your target.

Use `save` to persist the greffes list. It is automatically reloaded on the next CLI launch.

---

## Commands

| Command | Description |
|---|---|
| `add <name\|0xaddr>` | Register a greffe |
| `del <name\|0xaddr>` | Remove a greffe |
| `list` | List registered greffes |
| `save` | Save state to `workdir/.greffe` |
| `patch` | Compile handlers and apply all greffes |

---

## Constraints

**Flashing** - Greffe produces a modified binary that must be flashed to the target. Any secure boot chain must be bypassed or disabled before.

**Executable spare region** - trampolines and compiled handlers are injected at `patch_base`. This region must be mapped as executable at runtime. Typical candidates: padding between sections, unused code, ...

**Output channel** - Greffe only patches the binary, it provides no data collection mechanism. Handlers need a way to exfiltrate results: UART, PWM, a very attentive oscilloscope, ... Whatever the target exposes.

**No libc, no OS** - handlers are compiled with `-nostdlib`. Any helper must be self-contained or reference existing firmware code explicitly. (beware of the ABI)

---

## Workdir layout

```
__greffe_workdir/
├── Makefile          auto-generated
├── .greffe           saved configuratin
├── handlers/
│   ├── my_func.c     <- edit this
│   └── usr_utils.c
└── build/
    ├── handlers.elf
    └── handlers.bin
```
