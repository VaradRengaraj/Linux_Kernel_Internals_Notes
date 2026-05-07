# Paging: Typical Code Flow

When a virtual address (VA) is accessed, the kernel follows a sequence of steps to translate it into a physical address (PA):

---

## 1. TLB Lookup (Fastest Path)

The CPU first checks the Translation Lookaside Buffer (TLB), a small and fast cache that stores recent VA → PA translations.

- If a valid entry is found (**TLB hit**), the physical address is obtained immediately.
- No further work is needed.

---

## 2. TLB Miss → Exception Handling

If the translation is not present in the TLB (**TLB miss**), the CPU raises an exception and transfers control to the kernel’s exception handler.

At this point, execution can follow two paths:

---

### 2.1 Fast Path (TLB Refill)

- A low-level (often assembly) handler checks whether the VA → PA mapping already exists in the page tables in memory.
- If the mapping is present:
  - The TLB is updated (refilled) with the translation.
  - Execution resumes without invoking the full page fault handler.

---

### 2.2 Slow Path (Page Fault Handling)

If the mapping is not readily available, control flows to:

```c
do_page_fault()

```
where kernel:
- Locates the corresponding Virtual memory area (VMA)
- Verifies access permissions (read/write/execute)

if the access is valid: 
```c
handle_mm_fault()

```
is invoked.

Inside **handle_mm_fault**:
- A physical page is allocated (if needed)
- Page table entries (PTEs) are updated with the new mapping
- The mapping becomes visible for future accesses

#### Note
Details such as:
- Anonymous page allocation
- Page table updates
- TLB refill mechanisms
will be covered in a seperate article.

## 3. Exploring Page Table Walks in Practice
To better understand how a virtual address is translated through multi-level paging, we use a simple userspace experiment.

### 3.1 Userspace Setup
A page of memory is allocated using

```c
mmap(..., MAP_ANONYMOUS, ...)

```
The returned virtual address is examined:
- Before accessing the memory
- After touching (writing to) the memory
This helps illustrate how mappings are created lazily.

### 3.2 Kernel-Side Inspection
A custom kernel module is used to inspect the page tables for the given virtual address.

The virtual address is broken down into offsets corresponding to each paging level:
- PGD (Page Global Directory) 
- P4D (Page 4th Directory)
- PUD (Page Upper Directory)
- PMD (Page Middle Directory)
- PTE (Page Table Entry)

Each level is traversed to observer how the final physical mapping is resolved.

### 3.3 Logs and Observation

before memory allocation:

```
---- VMA DUMP START ----
VMA: 400000-48c000 | flags: 75
VMA: 498000-49c000 | flags: 100071
VMA: 49c000-4a4000 | flags: 100073
VMA: 7d8000-800000 | flags: 100073
VMA: 7fa58000-7fa7c000 | flags: 100177
VMA: 7fee8000-7feec000 | flags: 75
VMA: 7ff24000-7ff34000 | flags: 4044411
VMA: 7ff34000-7ff38000 | flags: 40075
---- VMA DUMP END ----
```

After memory allocation:

```
---- VMA DUMP START ----
VMA: 400000-48c000 | flags: 75
VMA: 498000-49c000 | flags: 100071
VMA: 49c000-4a4000 | flags: 100073
VMA: 7d8000-800000 | flags: 100073
VMA: 77c78000-77c7c000 | flags: 100073
VMA: 7fa58000-7fa7c000 | flags: 100177
VMA: 7fee8000-7feec000 | flags: 75
VMA: 7ff24000-7ff34000 | flags: 4044411
VMA: 7ff34000-7ff38000 | flags: 40075
---- VMA DUMP END ----
```

Before touching the memory:

```
Inspecting VA: 0x77c78000
PGD: 824f8074 val=80b04000
P4D: 824f8074 val=80b04000
PUD: 824f8074 val=80b04000
PMD not present
```

After touching the memory:

```
Inspecting VA: 0x77c78000
PGD: 824f8074 val=824f4000
P4D: 824f8074 val=824f4000
PUD: 824f8074 val=824f4000
PMD: 824f8074 val=824f4000
PTE: 824f7c78 val=110c58f
PFN: 443 | PHYS: 0x110c000
```

Observations to be filled.



