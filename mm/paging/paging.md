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

## Exploring Page Table Walks in Practice





