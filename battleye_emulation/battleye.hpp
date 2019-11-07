#pragma once
#include <cstdint>

namespace battleye
{
	struct packet
	{
		std::uint8_t id;
	};

	struct heartbeat_packet : packet
	{
		std::uint32_t heartbeat_data;
	};

}