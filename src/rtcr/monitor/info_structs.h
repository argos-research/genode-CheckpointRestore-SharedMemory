/*
 * \brief  Structs for monitor's objects
 * \author Denis Huber
 * \date   2016-11-21
 */

#ifndef _RTCR_INFO_STRUCTS_H_
#define _RTCR_INFO_STRUCTS_H_

/* Genode includes */
#include <util/list.h>

namespace Rtcr {
	struct General_info;
	struct Session_rpc_info;
	struct Normal_rpc_info;
	struct Normal_obj_info;
}

struct Rtcr::General_info
{
	bool const bootstrapped;

	General_info() : bootstrapped(false)
	{ }

	General_info(bool bootstrapped) : bootstrapped(bootstrapped)
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;
		using Genode::print;

		print(output, "bootstrapped=", bootstrapped);
	}
};


/**
 * Struct to monitor session RPC objects
 */
struct Rtcr::Session_rpc_info : General_info
{
	const char* const creation_args;
	const char* upgrade_args;

	Session_rpc_info() : General_info(), creation_args(""), upgrade_args("")
	{ }

	Session_rpc_info(const char* creation_args, const char* upgrade_args, bool bootstrapped)
	:
		General_info  (bootstrapped),
		creation_args (creation_args),
		upgrade_args  (upgrade_args)
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;
		using Genode::print;

		General_info::print(output);
		print(output, ", cargs='", creation_args, "', uargs='", upgrade_args, "'");
	}
};


/**
 * Struct to monitor normal RPC object (this is, no session or root RPC objects)
 */
struct Rtcr::Normal_rpc_info : General_info
{
	// Insert common members

	Normal_rpc_info() : General_info()
	{ }

	Normal_rpc_info(bool bootstrapped)
	:
		General_info(bootstrapped)
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;
		using Genode::print;

		General_info::print(output);
	}
};


/**
 * Struct to store information about RPC function invokation (e.g. parameter, return value)
 */
template<typename T>
struct Rtcr::Normal_obj_info : General_info
{
	// Insert common members

	Normal_obj_info() : General_info()
	{ }

	Normal_obj_info(bool bootstrapped)
	:
		General_info(bootstrapped)
	{ }

};


#endif /* _RTCR_INFO_STRUCTS_H_ */
