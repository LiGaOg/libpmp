# Computer-Security-Project-Proposal

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
* Memory protection is more related to managing the access of memory pages to avoid bugs or prevent malicious behavior. It is usually implemented with the system call, for instance, the `mprotect` on Linux, because the modification of the page table requires privileged access. To implement it, Intel has proposed a new mechanism called **Memory Protection Key (MPK)** for x86 architecture, which uses the **protection key rights register (`PKRU`)** and corresponding instructions to maintain 16 protection keys. Those protection keys are used to control the access of page groups.

* However, there exist three issues in MPK:

  * Protection-key-use-after-free problem.

  * Protection key limitation.

  * Inter-thread synchronization for multi-thread programs.


* To fix those issues, an open-source project called `libmpk` provides an abstraction of MPK, which fixes those three issues above.

* However, this is the overall solution for x86 architecture. We want to explore the memory protection mechanisms in other architecture.

* In RISC-V, a similar design is called **Physical Memory Protection (PMP)**. PMP uses several PMP entries to control the privilege of memory access (PMP is composed of configuration and address registers, which are per-hardware-thread machine-mode control registers). **After the exploration of PMP, we found that the second issue (Protection key limitation) also exists in PMP**. So we want to provide an abstraction layer to fix the hidden problem in PMP.

### Why it is important?

**For Protection key limitation:** If 

## Related Work

> [libmpk: Software Abstraction for Intel Memory Protection Keys](https://arxiv.org/pdf/1811.07276.pdf)

This work introduces the software abstraction of MPK, which is used to fix the three hidden issues.

### The solution to Protection key limitation



## Proposed New Solution

## Evaluation Plan

### 1. Functional Evaluation

Our hypothesis when building `libmpk` was that `libmpk`, a software abstraction for PMP should implement the same function as the original PMP. Libmpk should limit untrusted code to access its own memory space.

### 2. Performance Evaluation

### 3. Security Evaluation
