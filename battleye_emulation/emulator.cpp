#include "emulator.hpp"
#include "loggr.hpp"
#include "spinner.hpp"
#include "process.hpp"
#include "battleye.hpp"
#include <windows.h>
#include <thread>

bool emulator::start()
{
	if (!this->setup_pipe())
	{
		loggr::log_error("Failed to setup pipe");
		return false;
	}

	// CONNECT TO PIPE
	loggr::log("Waiting for BEClient connection");

	auto spinner_object = spinner();
	spinner_object.start();

	while (!ConnectNamedPipe(this->m_pipe, nullptr) && GetLastError() != ERROR_PIPE_CONNECTED)
	{
		spinner_object.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	spinner_object.stop();

	loggr::log("BEClient connected!");

	// ALLOCATE BUFFER
	constexpr auto PAGE_SIZE = 0x1000;
	auto pipe_buffer = std::make_unique<std::uint8_t[]>(PAGE_SIZE);

	// LOOP READ FILE
	const auto process_id = native::process::id_from_name("RainbowSix.exe");
	loggr::log_indented<1, false>("Process id", process_id);

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		std::uint32_t read_count = 0;
		if (!ReadFile(
			this->m_pipe,									// PIPE HANDLE
			pipe_buffer.get(),								// PIPE BUFFER
			PAGE_SIZE,										// PIPE BUFFER SIZE
			reinterpret_cast<LPDWORD>(&read_count),			// COUNT OF BYTES READ FROM PIPE
			nullptr)										// OVERLAPPED (OPTIONAL)
			&& GetLastError() == ERROR_BROKEN_PIPE)			
		{
			break;
		}

		// ONLY HANDLE NEW INCOMING DATA
		if (read_count == 0)
			continue;

		const auto received_packet = reinterpret_cast<battleye::packet*>(pipe_buffer.get());


		switch (received_packet->id)
		{
		case 6:
		{
			loggr::log_indented<1>("Spoofed information packet");

			// PERSONAL INFORMATION REDACTED
			std::uint8_t information_packet[]
			{
				0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00
			};

			std::uint32_t written_count = 0;
			WriteFile(this->m_pipe, &information_packet, sizeof(information_packet), reinterpret_cast<LPDWORD>(&written_count), nullptr);

			break;
		}

		case 2:
		{
			loggr::log_indented<1>("Spoofed heartbeat packet");
			loggr::log_indented<2, true>("BEService->TimeDateStamp", 0x5DA8C193);
			loggr::log_indented<2, false>("Game", process_id);

			auto heartbeat_packet = battleye::heartbeat_packet();
			heartbeat_packet.id = 2;
			heartbeat_packet.heartbeat_data = 0x5DA8C193;

			std::uint32_t written_count = 0;
			if (!WriteFile(this->m_pipe, &heartbeat_packet, sizeof(heartbeat_packet), reinterpret_cast<LPDWORD>(&written_count), nullptr))
				break;
			
			heartbeat_packet.id = 2;
			heartbeat_packet.heartbeat_data = process_id;
			
			WriteFile(this->m_pipe, &heartbeat_packet, sizeof(heartbeat_packet), reinterpret_cast<LPDWORD>(&written_count), nullptr);

			break;
		}

		default:
			loggr::log_indented<1>("Ignored packet", received_packet->id);
			break;

		}
	}
}

bool emulator::setup_pipe()
{
	// SETUP SECURITY DESCRIPTOR
	auto security_descriptor = SECURITY_DESCRIPTOR();
	if (!InitializeSecurityDescriptor(&security_descriptor, SECURITY_DESCRIPTOR_REVISION))
	{
		loggr::log_error("Failed to initialize security descriptor");
		return false;
	}

	if (!SetSecurityDescriptorDacl(&security_descriptor, true, nullptr, false))
	{
		loggr::log_error("Failed to set security descriptor access control list");
		return false;
	}

	// SETUP PIPE
	auto security_attributes = SECURITY_ATTRIBUTES();
	security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	security_attributes.lpSecurityDescriptor = &security_descriptor;
	security_attributes.bInheritHandle = true;

	constexpr auto PIPE_NAME = "\\\\.\\pipe\\BattlEye";
	constexpr auto PAGE_SIZE = 0x1000;
	this->m_pipe = CreateNamedPipeA(
		PIPE_NAME,													// PIPE NAME
		PIPE_ACCESS_INBOUND | PIPE_ACCESS_DUPLEX,					// OPEN MODE
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,	// PIPE MODE
		PIPE_UNLIMITED_INSTANCES,									// MAX INSTANCES
		PAGE_SIZE, PAGE_SIZE,										// BUFFER IN/OUT SIZES
		50,															// DEFAULT TIME OUT
		&security_attributes);										// SECURITY DESCRIPTOR FOR NEW PIPE

	if (this->m_pipe == INVALID_HANDLE_VALUE)
	{
		loggr::log_error("Failed to setup BattlEye handle");
		return false;
	}

	return true;
}
