#pragma once

// Forward Declaration
class Shader;

class Path {
private:
	// path
	static std::vector<glm::vec3> initial_vertices;
	static std::vector<glm::vec3> control_points;
	static std::vector<glm::vec3> path_vertices;
	// map <u, s, i>
	static std::map<float, std::pair<float, uint32_t>> arc_length;
	static size_t num_segments, num_entries_per_seg;

	// speed control
	static float t1, t2, t_total, vc;
	// time along path
	static float t_curr;

	static bool path_completed;

	// model data
	static glm::vec3 pos, pitch, yaw, roll;

private:
	static glm::vec3 Bezier(float u, std::vector<glm::vec3> const& control_pts);
	static std::pair<float, uint32_t> ArcLength(float u);
	static std::pair<float, uint32_t> InverseArcLength(float s);

	static float DistanceTime(float t);
	static float VelocityTime(float t);

public:
	// path
	static void SetupDefaultPath();
	static void GeneratePath(glm::vec3 model_pos, glm::vec3 target_pos);
	static void SetupPath(std::vector<glm::vec3> points);
	static void DrawPath(Shader* shader);

	// speed control
	static void SetupSpeed(float _t_total, float _t1, float _t2);

	// return pos, pitch (u), yaw (v), roll (w)
	static std::tuple<glm::vec3, glm::vec3, glm::vec3, glm::vec3> UpdateModelAlongPath();

	static bool PathCompleted();
};