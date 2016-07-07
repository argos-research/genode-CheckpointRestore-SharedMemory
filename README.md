# Genode real-time capable checkpoint/restore mechanism

Workflow
1. Checkpoint a component A
2. Serialize data
3. Transfer to new node
4. Deserialize data
5. Restore state of the component A and restart it

To 1.:
Checkpointing shall be done without interferring the execution of the component
It shall be done fast: Incremental checkpointing: Only the changes are stored
* Need to know which data is modified
* Need to partition the data into page-sized segments
* MMU provides a dirty-bit per physical page; does Genode provide an API for this low-level mechanism?
* MMU resides on the CPU
* MMU translates virtual memory addresses to physical addresses
* MMU uses a page table to map virtual to physical pages
* A page table consists of page table entries (PTEs)
* A PTE MAY include information about (1) whether the page was written to (dirty bit) and (2) when it was last used (accessed bit)
* Need to find out, if Genode provides MMU information