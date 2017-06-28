/*
 * validator_session.h
 *
 *  Created on: 13.06.2017
 *      Author: stephan
 */

#ifndef _RTCR_VALIDATOR_SESSION_H_
#define _RTCR_VALIDATOR_SESSION_H_

#include <root/component.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <ram_session/connection.h>
#include <rm_session/connection.h>
#include <region_map/client.h>
#include <dataspace/client.h>
#include <util/retry.h>
#include <util/misc_math.h>

#include "validator_session/connection.h"
#include "../target_state.h"

namespace Rtcr{
	class Validator_session_component;
	class Validator_root;

}

class Rtcr::Validator_session_component : public Genode::Rpc_object<Validator_session>,
                                    public Genode::List<Validator_session_component>::Element
{
private:

	/*
	 * Environment of Rtcr
	 */
	Genode::Env &_env;

	/*
	 * Allocator
	 */
	Genode::Allocator &_md_alloc;

	/*
	 * Creation arguments
	 */
	const char* _creation_args;

	/*
	 * Target state of checkpoint
	 */
	Target_state _ts;

	/*
	 * Size of allocated dataspace
	 */
	Genode::size_t _ds_size;


	Stored_ram_dataspace_info* _ds;


public:
	Validator_session_component(Genode::Env &env, Genode::Allocator &md_alloc, const char *creation_args, Target_state ts, Genode::size_t ds_size);
	~Validator_session_component();

	bool dataspace_available();
	Genode::Dataspace_capability get_dataspace();



};




class Rtcr::Validator_root : public Genode::Root_component<Validator_session_component>{

private:

	/*
	 * Environment of Rtcr
	 */
	Genode::Env	&_env;


	/*
	 * Allocator
	 */
	Genode::Allocator &_md_alloc;


	/*
	 * Session entrypoint
	 */
	Genode::Entrypoint &_ep;


	/*
	 * Rtcr Target_state
	 */
	Target_state &_ts;


	/*
	 * Dataspace size
	 */
	Genode::size_t _ds_size;


	/**
	 * Lock for infos list
	 */
	Genode::Lock        _objs_lock;

	/**
	 * List for monitoring session objects
	 */
	Genode::List<Validator_session_component> _session_rpc_objs;

protected:
	Validator_session_component *_create_session(const char *args);
	void _upgrade_session(Validator_session_component *session, const char *upgrade_args);
	void _destroy_session(Validator_session_component *session);

public:
	Validator_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep, Target_state &ts, Genode::size_t ds_size);
    ~Validator_root();

    Genode::List<Validator_session_component> &session_infos() { return _session_rpc_objs; }

};
#endif /* SRC_RTCR_MC_CHECKPOINT_VALIDATOR_SESSION_H_ */
