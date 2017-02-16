# Genode real-time capable checkpoint/restore mechanism

General workflow in migration
1. Checkpoint component A
2. Serialize data
3. Transfer to new node
4. Deserialize data
5. Restore state of component A and restart it


Accessing target component's resources
* Parent/child approach
* Target component = child component
* Parent provides custom services which are used by the child (i.e. parent intercepts services used by the child)
 * Child creation: PD, CPU, and RAM sessions
 * Child runtime: All other sessions like RM, LOG, Timer sessions
* Custom services use the real services in the background
* Parent stores information about the state of each session
 * Creation arguments
 * Update arguments
 * Method invokations
 * Parent restores the inner state of used sessions through these information


Checkpoint/Restore
* Component's name: Rtcr (Real-time checkpointer/restorer)
* Checkpoint in userland
 * Service method: checkpoint() => it uses Checkpointer::checkpoint
 * Pause target during checkpoint
 * Read information about the capability space and map
 * Store intercepted session information (not dataspace content) to parent's address space
 * Store dataspace content to parent's address space
 * Resume target after checkpoint
* Restore in userland
 * Service method: restore() => it uses Restorer::restore
 * Recreate empty child without sessions
 * Recreate sessions and their RPC objects
 * Restore state of sessions and their RPC objects
 * Restore capability space and map with new capabilities
* Incremental checkpointing as optimization
 * Approach
  * At checkpoint time store only the changes to the last checkpoint
  * Marking/tracing "dirty pages" by using page faults exceptions
  * Parent provides a custom RAM session to the child which allocates managed dataspaces (=region maps) instead of usual dataspaces
  * The managed dataspace is filled with usual dataspaces (called designated dataspaces)
  * Designated dataspaces occupy an exclusive area in the managed dataspace, thus, the whole space of the managed dataspace is filled
  * To mark an accessed dataspace, all dataspaces from the managed dataspace are detached
  * When a region in the managed dataspace is accessed a page fault is triggered
  * The page fault is resolved by a thread which attaches the corresponding designated dataspace to the faulting region
  * Now the target component can use (read, write, execute) the region in the managed dataspace without disruption
  * When a checkpoint is performed this designated dataspace is stored to parent's address space and detached from the managed dataspace
  * Now the managed dataspace is ready to mark/trace accessed regions again
 * Tweaks
  * The granularity of the marking mechanism can be modified by changing the size of designated dataspaces
  * Increasing the designated dataspace size
   * Decreasing the chance a page fault occurs which lowers the overhead while the target component is running (runtime overhead)
   * Increasing the duration of the checkpoint, because the dataspace is larger and needs more time for copying (checkpoint overhead)
  * Decreasing the designated dataspace size
   * Increasing the chance a page fault occurs which increases the runtime overhead
   * Decreasing the duration of the checkpoint, because the dataspace is smaller
  * A balance between runtime and checkpoint overhead has to be found out in regard to the locality of target's memory usage
   * Accessing adjacent memory regions profits from large designated dataspaces
   * Accessing spread memory regions profits from small designated dataspaces
