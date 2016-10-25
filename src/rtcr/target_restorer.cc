/*
 * \brief  Restore mechanism for Target_child and Target_copy
 * \author Denis Huber
 * \date   2016-10-19
 */

#include "target_restorer.h"
#include "target_child.h"

using namespace Rtcr;

Target_restorer::Target_restorer(Genode::Env &env, Genode::Allocator &alloc, Target_child& child, Target_copy& copy)
:
	_env   (env),
	_alloc (alloc),
	_child (child),
	_copy  (copy)
{ }

void Target_restorer::restore()
{
	// Restore Threads
	//_restore_threads();

	// Restore Capabilities

	// Restore Region maps

}
