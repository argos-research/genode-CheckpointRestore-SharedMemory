# Genode real-time capable checkpoint/restore mechanism

Workflow

1. Checkpoint a component A

2. Serialize data

3. Transfer to new node

4. Deserialize data

5. Restore state of the component A and restart it


Approach
* Checkpoint in core
 * Checkpoint as a core service
 * Search PD session for target component (via label string or use PD session capability)
 * What to checkpoint: Dataspaces, thread's metadata, thread's registers, capabilities used
 * Store data on RAM or a filesystem (needs driver)
* Restore in core
 * Restore as a core service
 * Identify data of the target component (through label string)
 * Recreate missing capabilities
 * Load data to RAM to continue using it for periodic checkpoints
* Optimization via incremental checkpointing
 * Realization through "custom RAM service, managed dataspaces, detach -> page fault -> mark & attach"
 * Custom RAM service in core (with own service name: MRAM)
 * Managed dataspaces: 

1. MRAM service creates dataspaces from RAM and creates a RM session

2. It does not attach the dataspace to the RM session, but it creates a dataspace from the RM session

3. The created dataspace is a managed dataspace which is returned to the MRAM client for requesting a dataspace

 * detach -> page fault -> mark & attach mechanism:

1. On the first usage of the dataspace a page fault is triggered, because no dataspace is attached

2. The MRAM service catches the page fault and marks the dataspace as "used" and attaches a dataspace for the usage

3. After a checkpoint is performed the dataspaces are unmarked (set to "not used") and detached from the RM session, thus a new page fault can be caught

 * The managed dataspace size shall be a multiple of 1 PAGESIZE; a benchmark can be made to find out the optimal dataspace size for a given component