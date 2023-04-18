#include "stdafx.h"

#include "GraphicsManager.h"
#include "InputManager.h"

#include "SimManager.h"

SimManager::SimManager() :
	is_running(true) {

}

SimManager::~SimManager() {

}

void SimManager::Quit() {
	is_running = false;
}

bool SimManager::Status() {
	return is_running;
}

// check if window is closed
void SimManager::Update() {
	if (p_input_manager->IsKeyTriggered(GLFW_KEY_ESCAPE) || glfwWindowShouldClose(p_graphics_manager->GetWindow())) {
		p_sim_manager->Quit();
	}
}