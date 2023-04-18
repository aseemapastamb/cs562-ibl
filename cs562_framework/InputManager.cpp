#include "stdafx.h"

#include "GraphicsManager.h"

#include "InputManager.h"

InputManager::InputManager():
	mouse_current_pos{ 0 },
	mouse_previous_pos{ 0 } {
	// keyboard
	for (uint16_t i = 0; i < 512; ++i) {
		keyboard_previous_state[i] = GLFW_RELEASE;
		keyboard_current_state[i] = GLFW_RELEASE;
	}

	// mouse
	for (uint16_t i = 0; i < 8; ++i) {
		mouse_previous_state[i] = GLFW_RELEASE;
		mouse_current_state[i] = GLFW_RELEASE;
	}
}

InputManager::~InputManager() {

}

void InputManager::Update() {
	// keyboard
	for (uint16_t i = 0; i < 512; ++i) {
		keyboard_previous_state[i] = keyboard_current_state[i];
		keyboard_current_state[i] = glfwGetKey(p_graphics_manager->GetWindow(), i);
	}

	// mouse
	// buttons
	for (uint16_t i = 0; i < 8; ++i) {
		mouse_previous_state[i] = mouse_current_state[i];
		mouse_current_state[i] = glfwGetMouseButton(p_graphics_manager->GetWindow(), i);
	}
	// position
	mouse_previous_pos = mouse_current_pos;
	glfwGetCursorPos(p_graphics_manager->GetWindow(), &mouse_current_pos.x, &mouse_current_pos.y);
}

bool InputManager::IsKeyPressed(uint16_t key_down_value) const {
	if (key_down_value > 512) {
		return false;
	}
	else {
		return keyboard_current_state[key_down_value];
	}
}

bool InputManager::IsKeyReleased(uint16_t key_down_value) const {
	if (key_down_value > 512) {
		return false;
	}
	else {
		return (!keyboard_current_state[key_down_value]
			&& keyboard_previous_state[key_down_value]);
	}
}

bool InputManager::IsKeyTriggered(uint16_t key_down_value) const {
	if (key_down_value > 512) {
		return false;
	}
	else {
		return (keyboard_current_state[key_down_value]
			&& !keyboard_previous_state[key_down_value]);
	}
}

bool InputManager::IsMouseKeyPressed(uint16_t key_down_value) const {
	if (key_down_value > 8) {
		return false;
	}
	else {
		return mouse_current_state[key_down_value];
	}
}

bool InputManager::IsMouseKeyReleased(uint16_t key_down_value) const {
	if (key_down_value > 8) {
		return false;
	}
	else {
		return (!mouse_current_state[key_down_value]
			&& mouse_previous_state[key_down_value]);
	}
}

bool InputManager::IsMouseKeyTriggered(uint16_t key_down_value) const{
	if (key_down_value > 8) {
		return false;
	}
	else {
		return (mouse_current_state[key_down_value]
			&& !mouse_previous_state[key_down_value]);
	}
}

glm::vec2 InputManager::GetMouseDelta() const {
	return mouse_current_pos - mouse_previous_pos;
}