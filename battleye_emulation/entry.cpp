#include "emulator.hpp"
#include "loggr.hpp"

int main()
{
	loggr::log("Beginning BEClient<->BEService emulation");

	auto be_emulator = emulator();
	be_emulator.start();
}