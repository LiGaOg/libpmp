# Computer-Security-Proposal

[//]: # (## Title and Author List)

**Title:** `libpmp`

**Author List:** 

| SID      | Name   |
| -------- | ------ |
| 11913008 | 谢岳臻 |
| 11910104 | 王奕童 |
| 11912614 | 张睿豪 |
| 12012319 | 刘晟淇 |
| 11913003 | 李家奥 |

## Problem Statement

### What the problem is?
Memory protection is more related to manage the access of memory pages, either to avoid bugs or prevent malicious behaviour.
It is usually implemented with the system call, for instance the `mprotect` on Linux, because the modification of page table
requires privileged access. To implement it, Intel has proposed a new mechanism called **Memory Protection Key (MPK)** for x86 architecture, which uses a register
and corresponding instructions to maintain 16 protection keys. Those protection keys are used to control the access of page groups.


However, there exist three issues in MPK:
* Protection-key-use-after-free problem.
* Protection key limitation.
* Inter-thread synchronization for multi-thread program.

To fix those issues, an open-source project called `libmpk` provides a abstraction of MPK, which fixes those three issues above.

However, this is the overall solution for x86 architecture. We want to explore the memory protection mechanisms in other architecture.
In RISC-V, there's also a similar design called **Physical Memory Protection (PMP)**. PMP uses several PMP entries to control the privilege
of memory access (PMP is composed with configuration and address registers, which are per-hardware-thread machine-mode control registers). 
After the exploration of PMP, we found that the second issue also exists in PMP. So we want to provide a abstraction layer to fix the hidden problems in PMP.

### Why it is important?

**For Protection key limitation:** If 

## Related Work

> [libmpk: Software Abstraction for Intel Memory Protection Keys](https://arxiv.org/pdf/1811.07276.pdf)

This work introduce the software abstraction of MPK, which is used to fix the three hidden issues.

**Solution to Protection-key-use-after-free:**
**Solution to Protection key:**
**Solution to Inter-thread synchronization:**

## Proposed New Solution

## Evaluation Plan

### 1. Functional Evaluation

Our hypothesis when building `libmpk` was that `libmpk`, a software abstraction for PMP should implement the same function as the original PMP. Libmpk should limit untrusted code to access its own memory space.

### 2. Performance Evaluation

### 3. Security Evaluation
