/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "restorer.h"
#include "target_child.h"

using namespace Rtcr;

Restorer::Restorer(Target_child &child, Target_state &state)
: _child(child), _state(state) { }

void Restorer::restore()
{

}
