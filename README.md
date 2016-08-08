# Genode real-time capable checkpoint/restore mechanism

Workflow

1. Checkpoint a component A

2. Serialize data

3. Transfer to new node

4. Deserialize data

5. Restore state of the component A and restart it


Approach
* Checkpoint in userland
 * Checkpoint as a service
 * Parent is checkpointer
 * Child is the target
 * Intercept PD session for Region_map
 * Intercept CPU sessoin for thread information
 * Intercept PD session for created capabilities
 * Intercept session requests for obtained capabilities
 * Store data in RAM of checkpointer by using own RAM quota or in a filesystem (needs driver)
* Restore in userland
 * Restore as a service
 * Recreate the child using stored data from checkpoint
 * Recreate PD, CPU, RAM, ROM session
 * Recreate created/obtained capabilities
 * Load data of the target component into checkpointer to allow incremental checkpoints
* Incremental checkpointing as optimization
 * At checkpoint time store only the changes to the last checkpoint
 * Marking of "dirty pages" through the use of a custom RAM service with managed dataspaces
 * Intercept RAM session to create managed dataspaces instead of normal dataspaces
 * Managed dataspaces: 

1. Custom RAM service creates dataspaces from RAM and creates a RM session

2. It does not attach the dataspace to the RM session, but it creates a dataspace from the RM session

3. The created dataspace is a managed dataspace which is returned to the RAM client for requesting a dataspace

 * detach -> page fault -> mark & attach mechanism:

1. On the first usage of the dataspace a page fault is triggered, because no dataspace is attached

2. The MRAM service catches the page fault and marks the dataspace as "used" and attaches a dataspace for the usage

3. After a checkpoint is performed the dataspaces are unmarked (set to "not used") and detached from the RM session, thus a new page fault can be caught

 * The managed dataspace size shall be a multiple of 1 PAGESIZE; a benchmark can be made to find out the optimal dataspace size for a given component