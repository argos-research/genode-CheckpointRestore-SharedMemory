/*
 * \brief Core-specific instance of the PD session interface
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
//#include "region_map_component.h"

namespace Rtcr {
	class Pd_session_component;
	using namespace Genode;
}

class Rtcr::Pd_session_component : public Genode::Rpc_object<Genode::Pd_session>
{
	private:
		Genode::Pd_connection _pd;

	public:
		/**
		 * Constructor
		 */
		Pd_session_component(Genode::Env &env)
		:
			_pd(env, "rtcr_pd_session_component")
		{ }

		~Pd_session_component()
		{ }

		/**
		 * Accessor
		 */
		Genode::Pd_session_capability core_pd_cap() { return _pd.cap(); }

		/**************************
		 ** Pd_session interface **
		 **************************/
		void assign_parent(Capability<Parent> parent) override {
			log(__PRETTY_FUNCTION__);
			_pd.assign_parent(parent); }

		bool assign_pci(addr_t addr, uint16_t bdf) override {
			log(__PRETTY_FUNCTION__);
			return _pd.assign_pci(addr, bdf); }

		Signal_source_capability alloc_signal_source() {
			log(__PRETTY_FUNCTION__);
			return _pd.alloc_signal_source(); }

		void free_signal_source(Signal_source_capability cap) override {
			log(__PRETTY_FUNCTION__);
			_pd.free_signal_source(cap); }

		Capability<Signal_context> alloc_context(Signal_source_capability source,
				unsigned long imprint) override {
			log(__PRETTY_FUNCTION__);
			return _pd.alloc_context(source, imprint); }

		void free_context(Capability<Signal_context> cap) override {
			log(__PRETTY_FUNCTION__);
			_pd.free_context(cap); }

		void submit(Capability<Signal_context> context, unsigned cnt) override {
			log(__PRETTY_FUNCTION__);
			_pd.submit(context, cnt); }

		Native_capability alloc_rpc_cap(Native_capability ep) override {
			log(__PRETTY_FUNCTION__);
			return _pd.alloc_rpc_cap(ep); }

		void free_rpc_cap(Native_capability cap) override {
			log(__PRETTY_FUNCTION__);
			_pd.free_rpc_cap(cap); }

		Capability<Region_map> address_space() override {
			log(__PRETTY_FUNCTION__);
			return _pd.address_space(); }

		Capability<Region_map> stack_area() override {
			log(__PRETTY_FUNCTION__);
			return _pd.stack_area(); }

		Capability<Region_map> linker_area() override {
			log(__PRETTY_FUNCTION__);
			return _pd.linker_area(); }

		Capability<Native_pd> native_pd() override {
			log(__PRETTY_FUNCTION__);
			return _pd.native_pd(); }

};

#endif /* _RTCR_PD_SESSION_COMPONENT_H_ */
