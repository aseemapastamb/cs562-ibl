#include "stdafx.h"

#include "Camera.h"

Camera::Camera(glm::vec3 _position, glm::vec3 _up, float _yaw, float _pitch):
	first_mouse{ true },
	position{ _position },
	world_up{ _up },
	yaw{ _yaw },
	pitch{ _pitch },
	front{ glm::vec3{0.0f, 0.0f, -1.0f} },
	up{ _up },
	right{ glm::vec3{1.0f, 0.0f, 0.0f} },
	move_speed{ SPEED },
	mouse_sensitivity{ SENSITIVITY },
	zoom{ ZOOM } {
	default_position = position;
	default_yaw = yaw;
	default_pitch = pitch;
	default_zoom = zoom;
	UpdateCameraVectors();
}

Camera::Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float _yaw, float _pitch):
	position{ glm::vec3{pos_x, pos_y, pos_z} },
	world_up{ glm::vec3{up_x, up_y, up_z} },
	yaw{ _yaw },
	pitch{ _pitch },
	front{ glm::vec3{0.0f, 0.0f, -1.0f} },
	up{ glm::vec3{up_x, up_y, up_z} },
	right{ glm::vec3{1.0f, 0.0f, 0.0f} },
	move_speed{ SPEED },
	mouse_sensitivity{ SENSITIVITY },
	zoom{ ZOOM } {
	default_position = position;
	default_yaw = yaw;
	default_pitch = pitch;
	default_zoom = zoom;
	UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMat() {
	return glm::lookAt(position, position + front, up);;
}

void Camera::ProcessKeyboard(CameraMovement direction, float dt) {
	float velocity = move_speed * dt;
	
	switch (direction) {
	case CameraMovement::FORWARD:
		position += front * velocity;
		break;
	case CameraMovement::BACKWARD:
		position -= front * velocity;
		break;
	case CameraMovement::LEFT:
		position -= right * velocity;
		break;
	case CameraMovement::RIGHT:
		position += right * velocity;
		break;
	case CameraMovement::PITCHUP:
		pitch += 10.0f * velocity;
		break;
	case CameraMovement::PITCHDOWN:
		pitch -= 10.0f * velocity;
		break;
	case CameraMovement::YAWLEFT:
		yaw -= 10.0f * velocity;
		break;
	case CameraMovement::YAWRIGHT:
		yaw += 10.0f * velocity;
		break;
	case CameraMovement::ZOOMIN:
		zoom -= 10.0f * velocity;
		break;
	case CameraMovement::ZOOMOUT:
		zoom += 10.0f * velocity;
		break;
	case CameraMovement::RESET:
		position = default_position;
		yaw = default_yaw;
		pitch = default_pitch;
		zoom = default_zoom;
		break;
	default:
		break;
	}

	UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float y_offset) {
	zoom -= y_offset;

	UpdateCameraVectors();
}

void Camera::ProcessMouseMovement(glm::vec2 offset) {
	if (first_mouse) {
		offset = glm::vec2{ 0.0f };
		first_mouse = false;
	}
	offset *= mouse_sensitivity;

	yaw += offset.x;
	pitch -= offset.y;

	UpdateCameraVectors();
}

// HELPERS
void Camera::UpdateCameraVectors() {
	// clamp zoom
	if (zoom < 1.0f) {
		zoom = 1.0f;
	}
	if (zoom > 45.0f) {
		zoom = 45.0f;
	}

	// clamp pitch so screen does not flip
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}
	if (pitch > 89.0f) {
		pitch = 89.0f;
	}

	glm::vec3 new_front;
	new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	new_front.y = sin(glm::radians(pitch));
	new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(new_front);

	right = glm::normalize(glm::cross(front, world_up));
	up = glm::normalize(glm::cross(right, front));
}