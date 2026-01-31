# Integrated Debugger System: Development Summary

This document outlines the architectural changes and implementation steps completed to integrate the **Interactive Debugger** across Labs 1, 4, and 5.

## 1. Project Goal
The objective was to transform standalone lab components into a **Monolithic Integrated System**. The Shell (Lab 1) now directly invokes the VM (Lab 4) and GC (Lab 5) through a shared memory space, satisfying the requirement that "data flows across components through well-defined interfaces."

---

## 2. System Architecture
The integration uses a **Fork-and-Execute** model within the same binary. This ensures that the Debugger has full access to the VM's internal structures while keeping the main Shell protected from VM crashes.



### Flow of Control:
1. **Shell:** Captures the `debug <pid>` directive.
2. **Process Manager:** Locates the process metadata and forks.
3. **Child Process:** Initializes a fresh VM state (`vm_init`) and hands control to the Debugger.
4. **Debugger:** Executes an interactive loop that calls internal VM functions (`vm_step`, `gc_collect`).

---

## 3. Implementation Phases

### Phase 1: Shell Directive & Integrated Linking
* **Directive Implementation:** Added `handle_debug` to `execute.c` to validate user input.
* **Linker Configuration:** Updated the `Makefile` to link VM source files (from `Folder 5`) directly into the Shell binary. This allows the Shell to call VM functions without needing an external `execvp` call.
* **Header Resolution:** Fixed `pid_t` and `fork` errors by including `<sys/types.h>` and `<unistd.h>`.



### Phase 2: Virtual Machine Control (Lab 4)
* **Instruction Stepping:** Integrated the `step` command to trigger `vm_step(p)`.
* **Breakpoints:** Implemented a breakpoint management system allowing users to set (`break <addr>`) and clear (`delete`) stops at specific bytecode addresses.
* **Execution Flow:** Implemented `continue` logic to run the VM until a breakpoint or `HALT` is reached.



### Phase 3: Memory & GC Integration (Lab 5)
* **Heap Analysis:** Added the `memstat` command inside the debugger to inspect global memory and the stack.
* **Manual GC:** Integrated the `gc` command to allow the user to manually trigger the Mark-and-Sweep collector during a debug session.

---

## 4. Key File Changes

| File | Purpose |
| :--- | :--- |
| `1.mini-shell/mini-shell.c` | Added `handle_debug` to the primary command loop. |
| `1.mini-shell/process/process_mgmt.c` | Implemented `debug_program` to fork and initialize the VM context. |
| `1.mini-shell/Makefile` | Configured to "absorb" VM `.c` files into the Shell executable. |
| `5.VM(ASS)withGC/debugger.c` | The core interactive loop managing breakpoints and stepping. |
| `5.VM(ASS)withGC/debugger.h` | The modular interface for teammate collaboration. |

---

## 5. Testing Protocol
To verify the integration, follow these steps from the shell:

1. **Submit:** `submit tests/valid/test.byc` (Note the PID).
2. **Launch:** `debug <PID>`.
3. **Control:**
    * `break 10` -> Set a breakpoint.
    * `continue` -> Run until the breakpoint.
    * `step` -> Observe the PC increment.
4. **Analyze:**
    * `memstat` -> View the Lab 5 memory state.
    * `gc` -> Trigger the Lab 5 Garbage Collector.

---


### the part which are not done yet are, debugger memstat , leak , gc 
- execution way
in folder 1 go and make
from ./mini-shell -> give input as submit filename(eg;- tests/valid/01_var_decl.txt) -> .asm file -> assembler -> byc ->vm


debugger is not done yet