# Genode real-time capable checkpoint/restore mechanism

Workflow

1. Checkpoint a component A

2. Serialize data

3. Transfer to new node

4. Deserialize data

5. Restore state of the component A and restart it


Approach
* Checkpoint in core. Checkpointing as a core service, or as a script which can be triggered by the scheduler.
 * Checkpoint the PD session
 * Capability space
 * Metadata of the capabilities
 * Managed dataspaces in the incremental checkpointer (see next step)


* Incremental Checkpointing needs: 
 * Service which provides a RAM service for distributing managed dataspaces instead of "normal" dataspaces
 * A client's (= component which shall be checkpointed) RAM access is rerouted to the custom RAM service
 * By requesting a dataspace from the RAM service, a managed dataspace with several small dataspaces (optimal: 1 PAGESIZE) is returned.
 * First they are not backed with real memory (detached)
 * After the client tries to use them, the CPU causes a page fault
 * The page fault is caught by the foc kernel, which forwards it to Genode
 * Genode tries to find a suitable dataspace in its core and fails
 * Upon failure it sends a Signal to its other components which can catch it through a Rm\_client::fault_handler
 * A thread catches this signal and attaches a dataspace at the requested location
 * It also marks the location for the next checkpoint
 * Now the client resumes its execution until a checkpoint is performed
 * After the checkpoint the marks are unset from all dataspaces and all dataspaces are detached again


* Restore
 * Object identities (through PD session's metadata)
 * Capability space for the component
 * Managed dataspaces in the incremental checkpointer
 * Inclusive Registers, e.g. instruction pointer

#TODO

Approach is deprecated.

Change *Approach* from hybrid (core and genode api) to core-only approach!