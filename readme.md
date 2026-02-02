# Technical Specifications & Instruction Manual: Integrated VM Framework
**Version:** 1.0 (Final Integration)  
**Target Environment:** Linux (Smaug/Hobbit31)  
**Modules:** Shell (Lab 1), Parser (Lab 2/3), VM/Debugger (Lab 4), Garbage Collector (Lab 5)

---

## 1. System Architecture: The Unified Pipeline

The system is designed as an interdependent pipeline where the state of the **Virtual Machine (Lab 4)** is analyzed by the **Garbage Collector (Lab 5)** and managed by the **Shell (Lab 1)**.

### The Interdependence Model
* **Root Identification:** The GC uses the VM's `stack_top` to identify reachable objects.
* **Process Persistence:** The Shell monitors the VM's PID to ensure stability.
* **Contextual Debugging:** The Debugger maps the VM's Program Counter (PC) to instruction mnemonics.



---

## 2. Integrated Instruction Set (Debugger Commands)

When execution is paused at the `(debug pc=X) >` prompt, the following commands are utilized to manage the system state.

### A. Execution & Control
* **`step`**: Increments the `PC` by 1 and executes the next opcode.
* **`continue`**: Resumes the VM fetch-execute loop until a `HALT` instruction or a breakpoint address is met.
* **`break <addr>`**: Registers a specific address in the `bp_table`.
* **`info break`**: Displays the table of active breakpoints.
* **`delete <id>`**: Removes a breakpoint based on its Table ID.

### B. Memory & GC Analysis
* **`memstat`**: Performs a holistic audit.
    * **Logic:** `Live on Heap = Created - Freed`.
    * **Metadata:** Displays PC, instruction type, and current byte usage.
* **`gc`**: Manual invocation of the Mark-and-Sweep cycle.
* **`leaks`**: An audit of orphaned objects.
    * **Logic:** `Potential Leaks = Live on Heap - Live on Stack`.



---

## 3. The Garbage Collection Protocol (Lab 5)

The system manages memory through a "Mark-and-Sweep" algorithm that synchronizes the **VM Stack** with the **Dynamic Heap**.

### Phase I: The Mark Phase
1.  The GC scans the VM Operand Stack from index `0` to `stack_top`.
2.  Every pointer found is marked as "Reachable" (`o->marked = 1`).
3.  **Recursive Visit:** For `ObjPair` types, the GC recursively visits the `left` and `right` children. This ensures complex graphs (linked lists, trees) are preserved.

### Phase II: The Sweep Phase
1.  The GC traverses the global `heap_objects` linked list.
2.  Any object found with `marked == 0` is unlinked from the list and passed to `free()`.
3.  The global `no_of_object_freed` counter is incremented for every reclaimed object.



---

## 4. Implementation Invariants & Rules

To ensure a "Proper" integration, the following rules are enforced in the source code:

1.  **PC Non-Mutation:** Diagnostic commands (`memstat`, `gc`, `leaks`) must **never** modify the Program Counter. They are read-only observers.
2.  **Structural Consistency:** The `Breakpoint` struct must utilize the exact field name `addr` (or `address`) to match the debugger's print logic.
3.  **State Synchronization:** Before reporting memory, the system must call `checkstack()` to ensure the `stack_object_count` is synchronized with the actual VM stack pointer.
4.  **Signal Isolation:** The Shell must capture `SIGINT` so that a user hitting `Ctrl+C` in the debugger is returned to the `(debug) >` prompt rather than exiting to the Linux terminal.

---

## 5. Stress Testing & Validation Procedures

To verify the system's robustness, the following procedures should be performed:

| Test | Objective | Success Criteria |
| :--- | :--- | :--- |
| **Orphan Stress** | Generate 1,000 unreferenced Pairs. | `memstat` shows 1,000 "Objects to be Freed." |
| **Collector Delta** | Run `gc` twice at the same PC. | First run: $X$ freed; Second run: **0** freed. |
| **PC Persistence** | Run `gc` mid-execution. | PC remains at the current instruction; no skip occurs. |
| **Process Overflow** | Submit beyond `MAX_PROCESSES`. | Shell returns an error instead of crashing. |

---

## 6. Conclusion
This system represents a fully integrated virtual computer. The synergy between the Shell's process management and the VM's memory reclamation provides a transparent, industrial-grade environment for bytecode execution.