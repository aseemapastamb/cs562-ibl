#pragma once

// to control the application
class SimManager {
private:
	bool is_running;

public:
	SimManager();
	~SimManager();

	void Quit();
	bool Status();

	// check if window is closed
	void Update();
};

extern SimManager* p_sim_manager;