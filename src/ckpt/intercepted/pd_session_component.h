/*
 * \brief Intercepting Pd session
 * \author Denis Huber
 * \date 2016-08-03
 */

#ifndef _RTCR_PD_SESSION_COMPONENT_H_
#define _RTCR_PD_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/rpc_server.h>
#include <base/log.h>
#include <pd_session/connection.h>

/* RTCR includes */
#include "intercepted/region_map_component.h"

namespace Rtcr {
	class Pd_session_component;
	using namespace Genode;
}

class Rtcr::Pd_session_component : public Genode::Rpc_object<Genode::Pd_session>
{
private:
	static constexpr bool verbose = true;

	Env           &_env;
	Allocator     &_md_alloc;
	Entrypoint    &_ep;
	Pd_connection  _pd;

	Rtcr::Region_map_component _address_space;
	Rtcr::Region_map_component _stack_area;
	Rtcr::Region_map_component _linker_area;

public:
	/**
	 * Constructor
	 */
	Pd_session_component(Env &env, Allocator &md_alloc, Entrypoint &ep)
	:
		_env(env),  _md_alloc(md_alloc), _ep(ep),
		_pd(env, "rtcr pd session component"),
		_address_space(_env, _md_alloc, _ep, _pd.address_space()),
		_stack_area   (_env, _md_alloc, _ep, _pd.stack_area()),
		_linker_area  (_env, _md_alloc, _ep, _pd.linker_area())
	{
		_ep.manage(*this);
	}

	~Pd_session_component()
	{
		_ep.dissolve(*this);
	}

	/**
	 * Accessor
	 */
	Rtcr::Region_map_component &region_map() { return _address_space; }
	Pd_session_capability core_pd_cap() { return _pd.cap(); }

	/**************************
	 ** Pd_session interface **
	 **************************/
	void assign_parent(Capability<Parent> parent) override {
		if(verbose) log(__PRETTY_FUNCTION__);
		_pd.assign_parent(parent); }

	bool assign_pci(addr_t addr, uint16_t bdf) override {
		if(verbose) log(__PRETTY_FUNCTION__);
		return _pd.assign_pci(addr, bdf); }

	Signal_source_capability alloc_signal_source() {
		if(verbose) log(__PRETTY_FUNCTION__);
		return _pd.alloc_signal_source(); }

	void free_signal_source(Signal_source_capability cap) override {
		if(verbose) log(__PRETTY_FUNCTION__);
		_pd.free_signal_source(cap); }

	Capability<Signal_context> alloc_context(Signal_source_capability source,
			unsigned long imprint) override {
		if(verbose) log(__PRETTY_FUNCTION__);
		return _pd.alloc_context(source, imprint); }

	void free_context(Capability<Signal_context> cap) override {
		if(verbose) log(__PRETTY_FUNCTION__);
		_pd.free_context(cap); }

	void submit(Capability<Signal_context> context, unsigned cnt) override {
		if(verbose) log(__PRETTY_FUNCTION__);
		_pd.submit(context, cnt); }

	Native_capability alloc_rpc_cap(Native_capability ep) override {
		if(verbose) log(__PRETTY_FUNCTION__);
		return _pd.alloc_rpc_cap(ep); }

	void free_rpc_cap(Native_capability cap) override {
		if(verbose) log(__PRETTY_FUNCTION__);
		_pd.free_rpc_cap(cap); }

	Capability<Region_map> address_space() override {
		if(verbose) log(__PRETTY_FUNCTION__);
		return _address_space.Rpc_object<Region_map>::cap(); }

	Capability<Region_map> stack_area() override {
		if(verbose) log(__PRETTY_FUNCTION__);
		return _stack_area.Rpc_object<Region_map>::cap(); }

	Capability<Region_map> linker_area() override {
		if(verbose) log(__PRETTY_FUNCTION__);
		return _linker_area.Rpc_object<Region_map>::cap(); }

	Capability<Native_pd> native_pd() override {
		if(verbose) log(__PRETTY_FUNCTION__);
		return _pd.native_pd(); }

};

#endif /* _RTCR_PD_SESSION_COMPONENT_H_ */
