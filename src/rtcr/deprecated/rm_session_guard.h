/*
 * \brief  Guarding an Rm_session_client, thus it does not run out of memory and throws an exception
 * \author Denis Huber
 * \date   2016-08-25
 */

/* Genode includes */
#include <rm_session/client.h>
#include <parent/parent.h>
#include <base/snprintf.h>

namespace Rtcr {
	class Rm_session_guard;
}

class Rtcr::Rm_session_guard
{
private:
	enum { RAM_QUOTA = 64*1024, ARG_STRING_SIZE = Genode::Parent::Session_args::MAX_SIZE, RAM_CREATE = 12*1024 };

	Genode::Env    &_env;
	Genode::size_t  _avail;
	Genode::Rm_session_client _rm_client;

	Genode::Rm_session_capability _session_request(Genode::Env &env)
	{
		char args[ARG_STRING_SIZE];
		Genode::snprintf(args, sizeof(args), "ram_quota=%u", RAM_QUOTA);
		return env.parent().session<Genode::Rm_session>(args);
	}

public:

	Rm_session_guard(Genode::Env &env)
	:
		_env(env),
		_avail(RAM_QUOTA),
		_rm_client(_session_request(env))
	{ }

	Genode::Capability<Genode::Region_map> create(Genode::size_t size)
	{
		if(_avail < RAM_CREATE)
		{
			char args[ARG_STRING_SIZE];
			Genode::snprintf(args, sizeof(args), "ram_quota=%u", RAM_QUOTA);

			// linear upgrade
			_env.parent().upgrade(_rm_client, args);
			_avail += RAM_QUOTA;
		}

		_avail -= RAM_CREATE;
		return _rm_client.create(size);
	}

	void destroy(Genode::Capability<Genode::Region_map> rm)
	{
		_rm_client.destroy(rm);
		_avail += RAM_CREATE;
	}

};
