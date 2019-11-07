#pragma once
#include <windows.h>
#include <iostream>
#include <chrono>
#include <fmt\core.h>

namespace loggr
{
	inline void allocate()
	{
		if (!AllocConsole())
		{
			// ???? SHOULD NEVER HAPPEN
			return;
		}

		std::freopen("CONOUT$", "w", stdout);
	}
	inline void free()
	{
		FreeConsole();
	}

	// CONSOLE LOGGING FUNCTIONS
	template <class...T>
	inline void log_raw(T... arguments)
	{
		fmt::print(arguments...);
	}

	inline void log(std::string_view message)
	{
		fmt::print("[+] {}\n", message);
	}
	inline void log_error(std::string_view message)
	{
		fmt::print("[!] {}\n", message);
	}

	template <bool hex = false, class T>
	inline void log(std::string_view variable_name, const T& value)
	{
		constexpr auto format_string = hex ? 
			"[=] {:<15} {:X}\n" :
			"[=] {:<15} {}\n";

		fmt::print(format_string, variable_name, value);
	}

	template <std::size_t indentation>
	inline void log_error_indented(std::string_view message)
	{
		fmt::print("[!] {:<{}} {}\n", ' ', indentation, message);
	}

	template <std::size_t indentation>
	inline void log_indented(std::string_view message)
	{
		fmt::print("[+] {:<{}} {}\n", ' ', indentation, message);
	}

	template <std::size_t indentation, bool hex = false, class T>
	inline void log_indented(std::string_view variable_name, const T& value)
	{
		constexpr auto format_string = hex ?
			"[=] {:<{}} {:.<15} {:02X}\n" :
			"[=] {:<{}} {:.<15} {}\n";

		fmt::print(format_string, ' ', indentation, variable_name, value);
	}


	// CONSOLE MODIFICATION FUNCTIONS
	inline COORD get_position()
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
		{
			loggr::log_error("Failed to get cursor position");
			return { 0, 0 };
		}

		return info.dwCursorPosition;
	}
	inline void set_position(const COORD cursor)
	{
		if (!SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor))
		{
			loggr::log_error("Failed to set cursor position");

		}
	}
	inline void clear_line()
	{
		// GET CURSOR POSITION
		auto position = loggr::get_position();


		position.X = 0;

		// CLEAR LINE
		DWORD count = 0;
		const auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
		auto result = FillConsoleOutputCharacter(handle, ' ', 150, position, &count);

		// RESET POSITION
		set_position(position);
	}

};