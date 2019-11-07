#include "process.hpp"
#include <algorithm>
#include <tlhelp32.h>
#include <Psapi.h>
#include <array>

std::uint32_t native::process::id_from_name(std::string_view process_name) noexcept
{
	DWORD process_list[516], bytes_needed;
	if (K32EnumProcesses(process_list, sizeof(process_list), &bytes_needed))
	{
		for (size_t index = 0; index < bytes_needed / sizeof(std::uint32_t); index++)
		{
			auto proc = process(process_list[index], PROCESS_ALL_ACCESS);

			if (proc && process_name == proc.get_name())
				return process_list[index];
		}
	}

	return 0x00;
}

std::uint32_t native::process::get_id() const noexcept
{
	return GetProcessId(this->handle().unsafe_handle());
}
std::string native::process::get_name() const noexcept
{
	char buffer[MAX_PATH];
	GetModuleBaseNameA(this->handle().unsafe_handle(), nullptr, buffer, MAX_PATH);

	return std::string(buffer);
}

const safe_handle& native::process::handle() const noexcept
{
	return this->m_handle;
}
