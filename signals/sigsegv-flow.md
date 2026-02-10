# The Life of a SIGSEGV — Part 1 (User Space Perspective)

## Introduction

In this article, we explore how the signal **SIGSEGV** is delivered to a Linux user-space process and what happens before the kernel steps in. This part focuses purely on the **user-space perspective**. The kernel internals of signal delivery will be covered in Part 2.

To make things concrete, we will:
- Write a simple user-space program that triggers SIGSEGV
- Verify that the fault happens due to writing into a read-only memory region
- Inspect the ELF sections to confirm memory permissions
- Look at the generated assembly to identify the exact crashing instruction

---

## Triggering SIGSEGV from User Space

Below is a simple program that intentionally triggers SIGSEGV:

```c
#include <stdio.h>

int main()
{
    char *ptr = "Hello";
    *(ptr) = 's';
    return 0;
}

```

### Why does this crash?

The string literal `"Hello"` is stored in a **read-only memory section** (typically `.rodata`).  
When we attempt to modify it, the CPU raises a protection fault, which eventually results in SIGSEGV being delivered to the process.

Specifically, this type of segmentation fault is typically classified as:
- **SEGV_ACCERR** → Invalid permissions for mapped memory

---

## Verifying That the String Resides in Read-Only Memory

This analysis is performed for the **MIPS architecture**.

### Step 1 — Compile the Program

```bash
mips-linux-gnu-gcc -static -o signal_test2 signal2.c
```

---

### Step 2 — Dump Section Contents

```bash
mips-linux-gnu-objdump -s signal_test2 > all_sections.txt
```

```
Contents of section .rodata:
 46e900 00020001 00000000 00000000 00000000  ................
 46e910 48656c6c 6f000000 00000000 00000000  Hello...........
```

From the objdump output, you can locate the string `"Hello"` inside the `.rodata` section.

Observation:
- String location: `0x46E910`
- Section: `.rodata`

---

### Step 3 — Verify Section Permissions

```bash
mips-linux-gnu-readelf -S signal_test2
```

```
  [16] .rodata           PROGBITS        004007c0 0007c0 000020 00   A  0   0 16
```

Look for the `.rodata` section flags.  
You will typically see something like:

- Allocatable (A)
- Not writable

This confirms the memory region is mapped read-only.

---

## Disassembling the Binary

To understand how the crash happens at instruction level:

```bash
mips-linux-gnu-objdump -d signal_test2 > disassembly.txt
```

Let us analyze the relevant portion of `main()`. We can ignore the function prologue and epilogue from this analysis.

```
00400790 <main>:
  400790:       27bdfff0        addiu   sp,sp,-16
  400794:       afbe000c        sw      s8,12(sp)
  400798:       03a0f025        move    s8,sp
  40079c:       3c020047        lui     v0,0x47
  4007a0:       2442e910        addiu   v0,v0,-5872
  4007a4:       afc20004        sw      v0,4(s8)
  4007a8:       8fc20004        lw      v0,4(s8)
  4007ac:       24030073        li      v1,115
  4007b0:       a0430000        sb      v1,0(v0)
  4007b4:       00001025        move    v0,zero
  4007b8:       03c0e825        move    sp,s8
  4007bc:       8fbe000c        lw      s8,12(sp)
  4007c0:       27bd0010        addiu   sp,sp,16
  4007c4:       03e00008        jr      ra
  4007c8:       00000000        nop
  4007cc:       00000000        nop

```
---

## Key MIPS Assembly Instructions (Simplified Explanation)

Below is a simplified logical interpretation of the important instructions.

### Constructing Address of the String

```
lui v0, 0x47
```
Loads upper 16 bits →  
```
v0 = 0x00470000
```

```
addiu v0, v0, -5872
```
5872 decimal = 0x16F0

```
v0 = 0x00470000 - 0x16F0
v0 = 0x46E910
```

Now `v0` holds the address of `"Hello"`.

---

### Saving Pointer to Stack (Local Variable Storage)

```
sw v0, 4(s8)
lw v0, 4(s8)
```

This corresponds to storing and reloading the local variable `ptr`.

---

### Preparing Value to Write

```
li v1, 115
```
ASCII value of `'s'`.

---

### The Crashing Instruction

```
sb v1, 0(v0)
```

Equivalent C operation:
```c
*(ptr + 0) = 's';
```

But `ptr` points into `.rodata` → write not allowed → page fault → SIGSEGV.

---

## Running the Program

When run inside QEMU or a real Linux system:

```bash
./signal_test2
```

You will see:

```
Segmentation fault
```

---

## Default Behavior of SIGSEGV

If no signal handler is installed:
- Kernel sends SIGSEGV to process
- Process is terminated
- Core dump may be generated (if enabled)

---

## Summary

In this article, we:
- Created a program that triggers SIGSEGV
- Verified the string is stored in `.rodata`
- Confirmed `.rodata` is read-only
- Identified the exact crashing instruction in assembly

---

## What’s Next (Part 2)

In Part 2, we will explore:

- Page fault handling in kernel
- VMA permission checks
- How kernel converts page fault → SIGSEGV
- Signal frame construction (`setup_rt_frame`)
- How control returns to user-space signal handler

---

## Miscellaneous Notes

Architecture used: MIPS  
Execution environment: QEMU + custom rootfs  
Binary format: ELF static binary
