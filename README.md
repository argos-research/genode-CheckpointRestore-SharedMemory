# Genode real-time capable checkpoint/restore mechanism

Workflow

1. Checkpoint component A

2. Serialize data

3. Transfer to new node

4. Deserialize data

5. Restore state of component A and restart it


Approach
* Checkpoint in userland
 * Checkpoint as an RPC to checkpointer
 * Parent is checkpointer
 * Child is the target
 * Pause target during checkpoint
 * Intercept PD session for Region_map
 * Intercept CPU session for thread information
 * Intercept PD session for created capabilities
 * Intercept session requests for obtained capabilities
 * Store data in RAM of checkpointer by using own RAM quota or in a filesystem (needs driver)
 * Resume target after checkpoint
* Restore in userland
 * Restore as an RPC to checkpointer
 * Recreate the child using stored data from checkpoint
 * Recreate PD, CPU, RAM, ROM session
 * Recreate created/obtained capabilities
 * Load data of the target component into checkpointer to allow incremental checkpoints
* Incremental checkpointing as optimization
 * At checkpoint time store only the changes to the last checkpoint
 * Marking of "dirty pages" through the use of a custom RAM service with managed dataspaces
 * Intercept RAM session to create managed dataspaces instead of normal dataspaces
 * Managed dataspaces: 

1. Custom RAM session object creates RAM dataspaces from parent's RAM service (usually core's service)

2. It also creates a dataspace from a Region map from an RM session (called a managed dataspace)

3. It does not attach the RAM dataspaces to the Region map yet

4. The Region map gets a fault handler which can mark the accessed data


 * detach -> page fault -> mark & attach mechanism:


1. On the first access on a managed dataspace a page fault is triggered, because no RAM dataspace is attached in the corresponding Region map

2. A fault handler catches the page fault and marks the dataspace as "used" and attaches a RAM dataspace (supposed for this location) to this location

3. After a checkpoint is performed the RAM dataspaces are unmarked (set to "not used") and detached from the Region map, thus a new page fault can be caught

 * The RAM dataspace size shall be a multiple of 1 PAGESIZE; a benchmark can be made to find out the optimal dataspace size for a given component
