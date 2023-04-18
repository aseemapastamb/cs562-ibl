#pragma once

//////////////////////////////////////////////////////////////
// External Libraries
//////////////////////////////////////////////////////////////

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "stb_image.h"

// C Std
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

// Memory
#include <memory>

// Math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>
#include <assimp/vector3.h>
#include <assimp/quaternion.h>