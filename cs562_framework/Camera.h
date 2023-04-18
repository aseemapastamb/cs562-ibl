#pragma once

enum class CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	PITCHUP,
	PITCHDOWN,
	YAWLEFT,
	YAWRIGHT,
	ZOOMIN,
	ZOOMOUT,
	RESET
};

const float YAW{ -90.0f };
const float PITCH = 0.0f;
const float SPEED = 10.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
private:
	glm::vec3 default_position;
	float default_yaw;
	float default_pitch;
	float default_zoom;
	bool first_mouse;

	void UpdateCameraVectors();

public:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	float yaw;
	float pitch;

	float move_speed;
	float mouse_sensitivity;
	float zoom;

	Camera(glm::vec3 _position = glm::vec3{ 0.0f, 0.0f, 3.0f }, glm::vec3 _up = glm::vec3{ 0.0f, 1.0f, 0.0f }, float _yaw = YAW, float _pitch = PITCH);
	Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float _yaw, float _pitch);

	glm::mat4 GetViewMat();
	void ProcessKeyboard(CameraMovement movement, float dt);
	void ProcessMouseScroll(float y_offset);
	void ProcessMouseMovement(glm::vec2 offset);
};