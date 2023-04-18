#pragma once

// fwd decl
class Camera;
class Shader;
class Model;
class Mesh;
class FBO;
class TextureHDR;
class Texture;

// global light
struct DirectionalLight {
	glm::vec3 position;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

// local light
struct LocalLight {
	glm::vec3 position;
	glm::vec3 colour;
	float radius;
};

// object material
struct Material {
	glm::vec3 diffuse;
	glm::vec3 specular;
	float roughness;
};

struct Transform {
	glm::vec3 position;
	glm::vec3 scale{ 1.0f };
};

struct Object {
	Model* model;
	Material material;
	Transform transform;

	Object():
		model{ nullptr },
		material{},
		transform{}
	{}
};

struct HammersleyBlock {
	uint32_t n;
	std::vector<float> hammersley;

	HammersleyBlock(uint32_t _n) :
		n{ _n }
	{
		hammersley.resize(2 * n);
	}
};

class GraphicsManager {
private:
	// window
	GLFWwindow* p_window;

	// viewing
	glm::mat4 model, view, projection, norm_inverse;
	glm::vec3 up;

	// grid
	std::vector<std::vector<glm::vec3>> grid_cols;
	std::vector<std::vector<glm::vec3>> grid_rows;

	// objects
	std::vector<Object*> objects;
	//Mesh* floor_mesh;

	// lights
	glm::vec3 background_color;
	bool global_light_off;
	DirectionalLight global_light;
	bool local_lights_off;
	bool hard_edges;
	std::vector<LocalLight> local_lights;
	Mesh* sphere_mesh;

	// shaders
	Shader* g_buffer_shader;
	Shader* shadow_shader;
	Shader* light_shader;
	Shader* local_lights_shader;
	Shader* blur_hori_shader;
	Shader* blur_vert_shader;
	Shader* sat_shader;

	// G-buffer FBO
	FBO* g_buffer_fbo;
	uint32_t quad_vao, quad_vbo, quad_ebo;

	// Shadow
	FBO* shadow_fbo;
	glm::vec3 shadow_target;

	// Blur Filter
	bool conv_or_sat;	// true = convolutional blur, false = summed area table blur
	bool var_or_msm;	// true = variance, false = hamburger 4-msm
	int32_t kernel_halfwidth;
	uint32_t block_id;

	// Skydome
	bool is_skydome;
	TextureHDR* skydome_tex;
	TextureHDR* irradiance_tex;
	HammersleyBlock* hammersley_block;
	uint32_t hammersley_block_id;
	float exposure;

private:
	// helper to capture camera control inputs
	void ProcessCameraMovement();

	// for deferred shading
	void SetupQuad();
	void DrawFullScreenQuad();

	// models
	void SetupModels();

	// local lights
	void SetupLocalLights();

	// grid
	void SetupGrid(int slices);
	void DrawGrid();

	// mesh
	void SetupMesh();

	// graphics passes
	void GBufferPass();
	void ShadowPass();
	void ConvolutionBlurPass();
	void SummedAreaTableBlurPass();
	void DeferredLightPass();
	void LocalLightPass();
	void DrawSkydome(bool is_skydome);
	void SetupHammersleyBlock();

	// graphics helpers
	void EnableFrontFaceCulling();
	void EnableBackFaceCulling();
	void DisableFaceCulling();
	void EnableDepthTest();
	void DisableDepthTest();
	void EnableMultiSampling();
	void DisableMultiSampling();
	void EnableAdditiveBlend();
	void DisableBlend();
	void ClearBuffer(glm::vec4 clear_colour, FBO* buffer = nullptr);
	glm::mat4 LookAt(glm::vec3 const& light_pos, glm::vec3 const& look_target) const;
	glm::mat4 ShadowProj() const;
	glm::mat4 ShadowView(glm::vec3 const& light_pos, glm::vec3 const& look_target) const;
	glm::mat4 ShadowMatrix(glm::vec3 const& light_pos) const;

public:
	int window_width;
	int window_height;
	Camera* camera;

public:
	GraphicsManager();
	~GraphicsManager();

	// setup up glfw and glad
	bool InitWindow();
	// shutdown imgui and glfw
	void DestroyWindow();

	GLFWwindow* GetWindow() const;

	// initialize all models and animations
	void SetupScene();

	// called every frame
	void RenderScene();
	void RenderGUI();
	void SwapBuffers();

	void FrameBufferSizeCallbackImpl(int width, int height);
};

extern GraphicsManager* p_graphics_manager;