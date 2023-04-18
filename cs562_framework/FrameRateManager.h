#pragma once

// calculate and control framerate
class FrameRateManager
{
public:
	FrameRateManager() = default;
	FrameRateManager(uint32_t _max_frame_rate);
	FrameRateManager(FrameRateManager const&) = delete;
	FrameRateManager& operator=(FrameRateManager const&) = delete;
	FrameRateManager(FrameRateManager&&) = delete;
	FrameRateManager& operator=(FrameRateManager&&) = delete;
	~FrameRateManager() = default;

	void FrameStart();
	void FrameEnd();
	inline
	float DeltaTime() { return delta_time; }
	inline
	float FrameRate() { return frame_rate; }

private:
	int64_t start_ticks;
	int64_t end_ticks;
	uint32_t max_frame_rate;
	int64_t max_ticks_per_frame;
	float delta_time;
	uint32_t frame_counter;
	int64_t frame_rate_timer;
	float frame_rate;
};

extern FrameRateManager* p_frame_rate_manager;