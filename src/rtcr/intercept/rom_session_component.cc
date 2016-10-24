/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \date   2016-10-24
 */

#include "rom_session_component.h"

using namespace Rtcr;


Rtcr::Rom_session_component::Rom_session_component(Genode::Env& env,
		Genode::Allocator& md_alloc, Genode::Entrypoint& ep, const char *label)
:
	_env          (env),
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_rom   (env, label),
	_parent_state ()
{
	if(verbose_debug) Genode::log("\033[33m", "Rom", "\033[0m(parent ", _parent_rom,")");
}


Rtcr::Rom_session_component::~Rom_session_component()
{
	if(verbose_debug) Genode::log("\033[33m", "~Rom", "\033[0m ", _parent_rom);
}


Genode::Rom_dataspace_capability Rtcr::Rom_session_component::dataspace()
{
	if(verbose_debug) Genode::log("Rom::\033[33m", __func__, "\033[0m()");
	auto result = _parent_rom.dataspace();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


bool Rtcr::Rom_session_component::update()
{
	if(verbose_debug) Genode::log("Rom::\033[33m", __func__, "\033[0m()");
	auto result = _parent_rom.update();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


void Rtcr::Rom_session_component::sigh(Genode::Signal_context_capability sigh)
{
	if(verbose_debug) Genode::log("Rom::\033[33m", __func__, "\033[0m()");
	_parent_state.sigh = sigh;
	_parent_rom.sigh(sigh);
}
