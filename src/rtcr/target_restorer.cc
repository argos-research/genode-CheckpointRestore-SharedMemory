/*
 * \brief  Restore mechanism for Target_child and Target_copy
 * \author Denis Huber
 * \date   2016-10-19
 */

#include "target_restorer.h"

using namespace Rtcr;

Target_restorer::Target_restorer(Target_child& child, Target_copy& copy)
:
	_child(child), _copy(copy)
{

}

void Target_restorer::restore(Target_child &child, Target_copy &copy)
{
	// Restore Threads
	//_restore_threads();

	// Restore Capabilities

	// Restore Region maps

}
