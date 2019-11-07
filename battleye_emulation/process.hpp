#pragma once
#include "safe_handle.hpp"

#include <windows.h>
#include <unordered_map>
#include <string>
#include <locale>
#include <codecvt>

#pragma warning(disable:4996) // DEPRECATED LIBRARY :(
using wstring_converter_t = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>;

namespace native
{
	class process
	{
	public:
		process() noexcept { }

		explicit process(HANDLE handle) noexcept : m_handle(handle) {}

		explicit process(std::uint32_t id, std::uint32_t desired_access) noexcept :
			m_handle(safe_handle(OpenProcess(desired_access, false, id))) { }

		explicit process(std::string_view process_name, std::uint32_t desired_access) noexcept
		{
			const auto process_id = native::process::id_from_name(process_name);

			if (!process_id)
				return;

			this->m_handle = safe_handle(OpenProcess(desired_access, false, process_id));
		}

		explicit operator bool() const noexcept
		{
			return this->handle().unsafe_handle() != nullptr;
		}

		// STATICS
		static std::uint32_t id_from_name(std::string_view process_name) noexcept;

		// INFORMATION
		std::uint32_t get_id() const noexcept;
		std::string get_name() const noexcept;

		const safe_handle& handle() const noexcept;

	private:
		safe_handle m_handle;
	};
}