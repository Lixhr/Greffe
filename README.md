# Greffe

> IDA Pro plugin for non-intrusive binary patching. Especially useful on bare-metal targets without debugging capabilities.

Right-click any instruction in IDA's disassembly view. Greffe replaces it with a branch to a user-written C handler, transparently relocating the original instruction so execution continues correctly after the handler returns.

Instruction relocation is powered by [frida-gum](https://github.com/frida/frida-gum). Currently supported architectures: **Thumb (ARM32)**.

> [!NOTE]
> Supported from IDA 9.2

---

## Build

Get the [IDA Sdk](https://github.com/HexRaysSA/ida-sdk/releases) corresponding to your version.


```sh
IDA_SDK=/path_to_ida_sdk \
IDA_DIR=/path_to_ida \
make
```

Output: `build/greffe.so`. Move it into your IDA's plugin directory.

---

## Usage

### 1. Load the plugin

Open your binary in IDA, then: **Edit → Plugins → Greffe**.

### 2. Define patch regions

In the disassembly view, select a range of bytes where trampolines and handlers can be injected, then right-click → **Set as greffe patch region** (or `Shift+R`).

The region must be mapped as executable at runtime. Typical candidates: padding between sections, unused functions, ...

### 3. Add a greffe

Right-click any instruction in the disassembly view → **Add a Greffe** (or `Shift+G`).

A C stub is automatically created at `__greffe_workdir/handlers/`:

```c
void handler_my_func(void)
{
}
```

You can add extra `.c` files under `handlers/`; all are compiled and linked into the same blob.

### 4. Write the handler

Edit the generated stub. Any helper code must be self-contained or reference existing firmware symbols directly.

### 5. Apply patches

Press `Shift+P`. Greffe compiles all handlers, resolves addresses, and writes the patches directly into IDA.


> [!WARNING]
> Greffe modifies the binary and may break IDA xrefs or labels.  
> Make sure to back up your project before using it.

---


## Constraints

**Executable spare regions** - trampolines are injected into patch regions that must be executable at runtime.

**No libc, no OS** - handlers are compiled with `-nostdlib -fno-pic`. Any helper must be self-contained or call into existing firmware code (mind the calling convention).

**Non-PIE only** - Greffe does not support position-independent binaries; Frida must have a known runtime address to generate relocated instructions.

> [!NOTE]
> A potential improvement would be to embed Frida directly into the patched binary to generate relocations at runtime.

---

## Workdir layout

```
__greffe_workdir/
├── Makefile              auto-generated
├── greffe_active.mk      lists active handler sources
├── handlers/
│   ├── my_func.c         ← edit this
│   └── usr_utils.c
└── build/
    ├── handlers.elf
    └── handlers.bin
```


