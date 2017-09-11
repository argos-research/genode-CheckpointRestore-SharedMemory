#include <base/component.h>
#include <timer_session/connection.h>
#include <base/log.h>
#include <rm_session/connection.h>
#include <region_map/client.h>

namespace Rtcr {
	class Main;
}

class Rtcr::Main
{
	enum {
		ROOT_STACK_SIZE = 16*1024,
		WRITE_ADDRESS_COUNT = 3,
		PRIME = 1423,
		TESTED_DATASPACE_SIZE = 5 * 4096,
		HASH_OFFSET = 4
	};

	Genode::Env              &env;

public:
	Main(Genode::Env &env_) : env(env_)
	{
		using namespace Genode;
		log("Creating Timer session.");
		Timer::Connection timer(env);

		log("Allocating and attaching memory and its dataspace.");

		Dataspace_capability ds_cap = env.ram().alloc(TESTED_DATASPACE_SIZE);
		unsigned int *addr = env.rm().attach(ds_cap);
		unsigned volatile int &n = addr[0];
		n = 1;

		while(1)
		{
			/*
			 * Busy waiting is used here to avoid pausing this component during an RPC call to the timer component.
			 * This could cause a resume call to this component to fail.
			 */
			for(int i = 0; i < 100000000; i++)
				__asm__("NOP");

			bool correct = verifyTestValues(n - 1, (char *) addr);
			if (correct)
				log("Memory content verified!");
			else
				log("ERROR! Memory content is not correct!");

			log("writing test values...");
			writeTestValues(n, (char *) addr);
			log(n, " sheep. zzZ");
			n++;
		}
	}

	void writeTestValues(int n, char *baseAddress) {
		for(int i = 0; i < WRITE_ADDRESS_COUNT; i++) {
			char *address = baseAddress + hashFunction(TESTED_DATASPACE_SIZE, n * WRITE_ADDRESS_COUNT + i);
			Genode::log("write to ", (void*) address);
			*((int *) address) = i + n + 1;
		}
	}

	bool verifyTestValues(int n, char *baseAddress) {
		//Genode::log("verify test values for n == ", n);

		for(int i = 0; i < WRITE_ADDRESS_COUNT; i++) {
			char *address = baseAddress + hashFunction(TESTED_DATASPACE_SIZE, n * WRITE_ADDRESS_COUNT + i);
			Genode::log("read from", (void*) address, "...");
			if (*((int *) (address)) != i + n + 1) {
				Genode::log("not correct!");
				return false;
			}

			Genode::log("correct!");
		}

		return true;
	}

	int hashFunction(int upperBound, int i) {
		return (PRIME * i) % (upperBound - HASH_OFFSET) + HASH_OFFSET;
	}
};

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}

