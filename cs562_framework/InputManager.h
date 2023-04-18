#pragma once

// update all input data
class InputManager {
private:
	// keyboard
	int keyboard_current_state[512];
	int keyboard_previous_state[512];

	// mouse
	int mouse_current_state[8];
	int mouse_previous_state[8];
	glm::dvec2 mouse_current_pos;
	glm::dvec2 mouse_previous_pos;

public:
	InputManager();
	~InputManager();

	void Update();

	bool IsKeyPressed(uint16_t key_down_value) const;
	bool IsKeyReleased(uint16_t key_down_value) const;
	bool IsKeyTriggered(uint16_t key_down_value) const;

	bool IsMouseKeyPressed(uint16_t key_down_value) const;
	bool IsMouseKeyReleased(uint16_t key_down_value) const;
	bool IsMouseKeyTriggered(uint16_t key_down_value) const;

	glm::vec2 GetMouseDelta() const;
};

extern InputManager* p_input_manager;