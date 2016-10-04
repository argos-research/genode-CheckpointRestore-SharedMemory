/*
 * \brief  Intercepting (TODO insert name) session
 * \author
 * \date
 */

// TODO insert specific session name
#ifndef _RTCR_TEMPLATE_SESSION_H_
#define _RTCR_TEMPLATE_SESSION_H_

/* Genode includes */
// TODO include correct session connection
// #include <template_session/connection.h>
#include <root/component.h>
#include <util/list.h>

// TODO rename ALL class names and ctor/dtor names and debug variables
namespace Rtcr {
	struct Templates_rpc_object_info;
	struct Template_session_info;
	class Template_session_component;
	class Template_root;

	constexpr bool template_verbose_debug = false;
	constexpr bool template_root_verbose_debug = false;
}


/**
 * List element for monitoring Rpc objects created by the virtual session object
 */
struct Rtcr::Templates_rpc_object_info : Genode::List<Templates_rpc_object_info>::Element
{
	/**
	 * Reference to the rpc object,
	 * encapsulates a capability which is the main reason for storing it
	 */
	Template_rpc_object_component &rpc_object;

	Template_rpc_object_info(Template_rpc_object_component &rpc_object)
	:
		rpc_object(rpc_object)
	{ }

	/**
	 * Find list element by Capability of the virtual session
	 *
	 * \param cap Capability to search for
	 *
	 * \return List element with the specified Capability
	 */
	// TODO adjust intercepted session capability
	Template_rpc_object_info *find_by_cap(Genode::Capability<Genode::Intercepted_session> cap)
	{
		if(cap == rpc_object.cap())
			return this;
		Template_rpc_object_info *info = next();
		return info ? info->find_by_cap(cap) : 0;
	}
};

/**
 * List element for monitoring session objects.
 * Each new connection from client to server is monitored here.
 */
struct Rtcr::Template_session_info : Genode::List<Template_session_info>::Element
{
	/**
	 * Reference to the session object,
	 * encapsulates a capability which is the main reason for storing it
	 */
	Template_session_component &session;
	/**
	 * Arguments provided for creating the session object
	 */
	const char *args;

	Template_session_info(Template_session_component &comp, const char* args)
	:
		session(comp),
		args(args)
	{ }

	Template_session_info *find_by_ptr(Template_session_component *ptr)
	{
		if(ptr == &session)
			return this;
		Template_session_info *info = next();
		return info ? info->find_by_ptr(ptr) : 0;
	}
};

/**
 * Virtual session object to intercept Rpc object creation and
 * state modifications of the wrapped, parent session object
 */
class Rtcr::Template_session_component : public Genode::Rpc_object<Genode::Log_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = template_verbose_debug;
	/**
	 * Allocator for Rpc objects created by this session and also for monitoring list elements
	 */
	Genode::Allocator             &_md_alloc;
	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint            &_ep;
	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	// TODO use correct connection to parent's session and rename ALL occurences of this variable
	Genode::Template_connection    _parent_template;

public:
	Template_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *args)
	:
		_md_alloc   (md_alloc),
		_ep         (ep),
		_parent_template (env, args)
	{
		if(verbose_debug) Genode::log("\033[33m", "Template_session_component", "\033[0m created");
	}

	~Template_session_component() { }

	// TODO adjust comment
	/************************************
	 ** Template session Rpc interface **
	 ************************************/

	/*
	 * TODO Implement session's methods
	 * you want to create monitoring list elements, whenever an Rpc object is created by a method
	 * also, you want to delete monitoring list elements, whenever an Rpc object is deleted by a method
	 * Compare other intercepted sessions (e.g. Rm_session.h)
	 */

	Genode::size_t method_with_result(String const &arg1, String const &arg2)
	{
		if(verbose_debug) Genode::log("Log::\033[33m", "method_with_result", "\033[0m(str1=", arg1.string(), ", arg2=", arg2.string(), ")");
		auto result = _parent_template.method_with_result(arg1, arg2);
		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

	void method_without_result(String const &arg1, String const &arg2)
	{
		if(verbose_debug) Genode::log("Log::\033[33m", "method_without_result", "\033[0m(str1=", arg1.string(), ", arg2=", arg2.string(), ")");
		_parent_template.method_without_result(arg1, arg2);
	}

};

/**
 * Virtual Root session object to intercept session object creation
 * This enables the Rtcr component to monitor capabilities created for session objects
 */
class Rtcr::Template_root : public Genode::Root_component<Template_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = template_root_verbose_debug;

	/**
	 * Environment of Rtcr; is forwarded to a created session object
	 */
	Genode::Env                    &_env;
	/**
	 * Allocator for session objects and monitoring list elements
	 */
	Genode::Allocator              &_md_alloc;
	/**
	 * Entrypoint for managing session objects
	 */
	Genode::Entrypoint             &_ep;
	/**
	 * Lock for infos list
	 */
	Genode::Lock                    _infos_lock;
	/**
	 * List for monitoring session objects
	 */
	Genode::List<Template_session_info>  _session_infos;

protected:
	/**
	 * Create session object and its monitoring list element
	 */
	Template_session_component *_create_session(const char *args)
	{
		Genode::log("Log_root::\033[33m", "_create_session", "\033[0m(", args,")");

		// TODO prepare args for Template_session_component

		// Create virtual session object
		Template_session_component *new_session =
				new (md_alloc()) Template_session_component(_env, _md_alloc, _ep, args);

		// Create and insert list element
		Template_session_info *new_session_info =
				new (md_alloc()) Template_session_info(*new_session, args);
		Genode::Lock::Guard guard(_infos_lock);
		_session_infos.insert(new_session_info);

		return new_session;
	}

	/**
	 * Destroy session object and its monitoring list element
	 */
	void _destroy_session(Template_session_component *session)
	{
		if(verbose_debug) Genode::log("Log_root::\033[33m", "_destroy_session", "\033[0m(ptr=", session,")");
		// Find and destroy list element and its session object
		Template_session_info *info = _session_infos.first();
		if(info) info = info->find_by_ptr(session);
		if(info)
		{
			// Remove and destroy list element
			_session_infos.remove(info);
			destroy(_md_alloc, info);

			// Destroy virtual session object
			destroy(_md_alloc, session);
		}
		// No list element found
		else
		{
			// TODO adjust string
			Genode::error("Template_root: Session not found in the list");
		}
	}

public:
	Template_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep)
	:
		Root_component<Template_session_component>(session_ep, md_alloc),
		_env           (env),
		_md_alloc      (md_alloc),
		_ep            (session_ep),
		_infos_lock    (),
		_session_infos ()
	{
		// TODO adjust string
		if(verbose_debug) Genode::log("\033[33m", "Template_root", "\033[0m created");
	}
};

// TODO adjust comment
#endif /* _RTCR_TEMPLATE_SESSION_H_ */
