# Greffe

> IDA Pro plugin for non-intrusive binary patching. Especially useful on bare-metal targets without debugging capabilities.

Right-click any instruction in IDA's disassembly view. Greffe replaces it with a branch to a user-written C handler.
The original instruction is transparently relocated so execution resumes normally after the handler returns.

Instruction relocation is powered by [frida-gum](https://github.com/frida/frida-gum). Currently supported architectures (LE / BE): 

- ARM32
- ARM32-Thumb
- AArch64

> [!NOTE]
> Requires IDA 9.2 or later


### Use cases

Other instrumentation tools shine on OS-hosted targets. Greffe focuses on bare-metal firmware instead. 


Use cases include, but are not limited to:

- Function tracing
- Exploit development
- Code coverage instrumentation
- ...

---

## Build

Get the [IDA SDK](https://github.com/HexRaysSA/ida-sdk/releases) corresponding to your version.


```sh
IDA_SDK=/path_to_ida_sdk \
IDA_DIR=/path_to_ida \
make
```

Output: `build/greffe.so`. Move it into IDA's plugins directory.

---

## Usage

### 1. Load the plugin

Open your binary in IDA, then: **Edit → Plugins → Greffe**.

### 2. Define patch regions

In the disassembly view, select a range of bytes where trampolines and handlers can be injected, then right-click → **Set as greffe patch region** (or `Shift+R`).

The region must be mapped as executable at runtime. Typical candidates: padding between sections, unused functions, ...

### 3. Create handlers

Open the panel: **Edit → Plugins → Show greffe panel** (or `Shift+M`).

Click `Add handler` and give it a name. A **.c** file with a function skeleton is generated in the greffe directory.

```c
void handler_myfunc()
{
    // { YOUR CODE }
}
```


### 4. Add a greffe

Right-click the target instruction in the disassembly view → **Add a Greffe** (or `Shift+G`).

The target then appears in the greffe panel. Link it to the desired handler.

### 5. Apply patches

Press `Shift+P`. All handlers are compiled, addresses resolved, and patches written directly into IDA.

You can add extra `.c` files under `handlers/`; all are compiled and linked into the same blob.


> [!WARNING]
> Greffe modifies the binary and may break IDA xrefs or labels.  
> Make sure to use a copy of your database.

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
greffe_<binary_name>/
├── Makefile              auto-generated
├── handlers/
│   ├── my_func.c         ← edit this
│   └── usr_utils.c
└── build/
    ├── handlers.elf
    └── handlers.ld
    └── ...
```

---
