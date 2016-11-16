/*
 * \brief  Fiasco.OC specific thread bootstrap code
 * \author Stefan Kalkowski
 * \author Martin Stein
 * \date   2011-01-20
 */

/*
 * Copyright (C) 2011-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <util/construct_at.h>
#include <base/thread.h>
#include <base/sleep.h>
#include <foc/native_capability.h>
#include <foc_native_pd/client.h>
#include <base/env.h>

/* base-internal includes */
#include <base/internal/native_utcb.h>
#include <base/internal/cap_map.h>


/*****************************
 ** Startup library support **
 *****************************/

void prepare_init_main_thread()
{
	using namespace Genode;
	enum { THREAD_CAP_ID = 1 };
	Cap_index * ci(cap_map()->insert(THREAD_CAP_ID, Fiasco::MAIN_THREAD_CAP));
	Fiasco::l4_utcb_tcr()->user[Fiasco::UTCB_TCR_BADGE] = (unsigned long)ci;
	Fiasco::l4_utcb_tcr()->user[Fiasco::UTCB_TCR_THREAD_OBJ] = 0;
}


void prepare_reinit_main_thread()
{
	using namespace Genode;
	construct_at<Capability_map>(cap_map());
	cap_idx_alloc()->reinit();
	prepare_init_main_thread();
}


/**
 * Sets cap_map_info in Foc_native_pd
 */
void init_pd()
{
	using namespace Genode;

	Capability<Pd_session::Native_pd> native_pd_cap =  env()->pd_session()->native_pd();
	// Avoid core capability
	if(native_pd_cap.valid())
	{
		Capability<Foc_native_pd> foc_pd_cap = static_cap_cast<Foc_native_pd>(native_pd_cap);
		Foc_native_pd_client(foc_pd_cap).cap_map_info((addr_t) cap_idx_alloc());
		log("from thread_bootstrap: ", cap_idx_alloc());
	}

}


/************
 ** Thread **
 ************/

void Genode::Thread::_thread_bootstrap() { }


void Genode::Thread::_thread_start()
{
	using namespace Genode;

	Thread::myself()->_thread_bootstrap();
	Thread::myself()->entry();
	Thread::myself()->_join_lock.unlock();
	sleep_forever();
}
