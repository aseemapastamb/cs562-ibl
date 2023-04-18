#include "stdafx.h"

#include <chrono>

#include "FrameRateManager.h"

using namespace std::chrono;

FrameRateManager::FrameRateManager(uint32_t _max_frame_rate):
	start_ticks{ 0 },
	end_ticks{ 0 },
	max_frame_rate{ _max_frame_rate },
	max_ticks_per_frame{ 1000000 / max_frame_rate },
	delta_time{ 0 },
	frame_counter{ 0 },
	frame_rate_timer{ 0 },
	frame_rate{ 0 } {
	assert(max_frame_rate > 0 && "Maximum frame rate cannot be 0 or less");
}

void FrameRateManager::FrameStart() {
	auto now = time_point_cast<microseconds>(steady_clock::now());
	start_ticks = now.time_since_epoch().count();
}

void FrameRateManager::FrameEnd() {
	auto now = time_point_cast<microseconds>(steady_clock::now());
	end_ticks = now.time_since_epoch().count();

	int64_t elapsed_time = end_ticks - start_ticks;
	// wait if operations are over to maintain constant framerate
	while (elapsed_time < max_ticks_per_frame) {
		now = time_point_cast<microseconds>(steady_clock::now());
		end_ticks = now.time_since_epoch().count();
		elapsed_time = end_ticks - start_ticks;
	}
	delta_time = static_cast<float>(elapsed_time) / 1000000.0f;

	// calculating frame rate over last 120 frames
	++frame_counter;
	frame_rate_timer += elapsed_time;
	if (frame_counter >= 120) {
		frame_rate = 120000000.0f / static_cast<float>(frame_rate_timer);
		frame_counter = 0;
		frame_rate_timer = 0;
	}
}