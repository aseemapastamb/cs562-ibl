#include "stdafx.h"

#include "InputManager.h"
#include "FrameRateManager.h"

#include "GraphicsManager.h"

#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Mesh.h"
#include "FBO.h"
#include "Texture.h"
//#include "Path.h"
//#include "Cloth.h"
//#include "PhysicsManager.h"

void FrameBufferSizeCallback(GLFWwindow* p_window, int width, int height);
void ScrollCallback(GLFWwindow* p_window, double x_ofset, double y_offset);

GraphicsManager::GraphicsManager() :
    window_width{ 1560 },
    window_height{ 1200 },
    model{ 1.0f },
    view{ 1.0f },
    projection{ 1.0f },
    norm_inverse{ 1.0f },
    up{ 0.0f, 1.0f, 0.0f },
    //floor_mesh{ nullptr },
    camera{ nullptr },
    background_color{ 0.12f, 0.1f, 0.1f },
    global_light_off{ false },
    global_light{},
    local_lights_off{ true },
    hard_edges{ false },
    g_buffer_shader{ nullptr },
    shadow_shader{ nullptr },
    light_shader{ nullptr },
    local_lights_shader{ nullptr },
    blur_hori_shader{ nullptr },
    blur_vert_shader{ nullptr },
    sat_shader{ nullptr },
    //sphere_mesh{ nullptr },
    g_buffer_fbo{ nullptr },
    quad_vao{ 0 },
    quad_vbo{ 0 },
    quad_ebo{ 0 },
    shadow_fbo{ nullptr },
    shadow_target{ 0.0f },
    conv_or_sat{ true },
    var_or_msm{ false },
    kernel_halfwidth{ 0 },
    block_id{ 0 },
    is_skydome{ true },
    skydome_tex{ nullptr },
    irradiance_tex{ nullptr },
    hammersley_block{ nullptr },
    hammersley_block_id{ 0 },
    exposure{ 0.5f }
{
    camera = new Camera(glm::vec3{ -30.0f, 30.0f, 35.0f }, up, -52.0f, -40.0f);
    p_window = nullptr;
}

GraphicsManager::~GraphicsManager() {

}

// setup up glfw and glad
bool GraphicsManager::InitWindow() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    p_window = glfwCreateWindow(window_width, window_height, "CS562", NULL, NULL);
    if (p_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        DestroyWindow();
        return false;
    }
    glfwMakeContextCurrent(p_window);
    glfwSetFramebufferSizeCallback(p_window, FrameBufferSizeCallback);
    glfwSetScrollCallback(p_window, ScrollCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    return true;
}

// shutdown imgui and glfw
void GraphicsManager::DestroyWindow() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (uint16_t i = 0; i < objects.size(); ++i) {
        delete objects[i]->model;
        delete objects[i];
    }

    delete hammersley_block;
    delete irradiance_tex;
    delete skydome_tex;
    delete shadow_fbo;
    delete g_buffer_fbo;
    delete sat_shader;
    delete blur_vert_shader;
    delete blur_hori_shader;
    delete local_lights_shader;
    delete light_shader;
    delete shadow_shader;
    delete g_buffer_shader;
    delete camera;

    glfwTerminate();
}

GLFWwindow* GraphicsManager::GetWindow() const {
    return p_window;
}

// initialize all models and animations
void GraphicsManager::SetupScene() {
    stbi_set_flip_vertically_on_load(true);

    EnableDepthTest();

    // load shaders
    g_buffer_shader = new Shader(".\\shaders\\g_buffer.vert", ".\\shaders\\g_buffer.frag");
    g_buffer_shader->Use();
    g_buffer_shader->SetInt("skydome", 0);
    g_buffer_shader->Unuse();

    shadow_shader = new Shader(".\\shaders\\shadow.vert", ".\\shaders\\shadow.frag");

    light_shader = new Shader(".\\shaders\\deferred_lighting.vert", ".\\shaders\\deferred_lighting.frag");
    light_shader->Use();
    light_shader->SetInt("g_world_pos", 0);
    light_shader->SetInt("g_world_norm", 1);
    light_shader->SetInt("g_diffuse_colour", 2);
    light_shader->SetInt("g_specular_colour", 3);
    light_shader->SetInt("shadow_map", 4);
    light_shader->SetInt("skydome_map", 5);
    light_shader->SetInt("irradiance_map", 6);
    light_shader->Unuse();

    local_lights_shader = new Shader(".\\shaders\\deferred_local_lights.vert", ".\\shaders\\deferred_local_lights.frag");
    local_lights_shader->Use();
    local_lights_shader->SetInt("g_world_pos", 0);
    local_lights_shader->SetInt("g_world_norm", 1);
    local_lights_shader->SetInt("g_diffuse_colour", 2);
    local_lights_shader->SetInt("g_specular_colour", 3);
    local_lights_shader->Unuse();

    blur_hori_shader = new Shader(".\\shaders\\blur_hori.comp");
    blur_vert_shader = new Shader(".\\shaders\\blur_vert.comp");
    sat_shader = new Shader(".\\shaders\\sat.comp");

    // setup models
    SetupModels();

    // setup mesh
    SetupMesh();

    //// grid calculations
    //SetupGrid(10);

    // setup global light
    global_light.position = glm::vec3{ 30.0f, 20.0f, 30.0f };
    global_light.ambient = glm::vec3{ 0.1f, 0.1f, 0.1f };
    global_light.diffuse = glm::vec3{ 0.9f, 0.9f, 0.9f };
    global_light.specular = glm::vec3{ 0.1f, 0.1f, 0.1f };

    // setup local lights
    SetupLocalLights();

    // setup g buffer fbo
    g_buffer_fbo = new FBO(window_width, window_height, 4);

    // setup shadow fbo
    shadow_fbo = new FBO(1024, 1024, 1);
    glGenBuffers(1, &block_id);

    // skydome
    skydome_tex = new TextureHDR("..\\resources\\textures\\Alexs_Apt_2k.hdr");
    irradiance_tex = new TextureHDR("..\\resources\\textures\\Alexs_Apt_2k_Irr.hdr");
    SetupHammersleyBlock();

    // setup quad
    SetupQuad();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(p_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GraphicsManager::RenderScene() {
    // Camera movement
    ProcessCameraMovement();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // G-Buffer Pass
    //===========================================
    GBufferPass();

    // Shadow Pass
    //===========================================
    ShadowPass();

    // Blur Passes
    //===========================================
    if (conv_or_sat) {
        ConvolutionBlurPass();
    }
    else {
        SummedAreaTableBlurPass();
    }

    // Deferred Lighting Pass
    //===========================================
    DeferredLightPass();

    // Local Lights Pass
    //===========================================
    LocalLightPass();

    //// render grid
    //line_shader->Use();
    //line_shader->SetMat4("projection", projection);
    //line_shader->SetMat4("view", view);
    //model = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });
    //line_shader->SetMat4("model", model);
    //line_shader->SetVec3("aColor", glm::vec3{ 0.1f, 0.1f, 0.1f });
    //DrawGrid();
}

void GraphicsManager::RenderGUI() {
    // ImGui new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Light Settings"); {
        ImGui::Checkbox("Turn Off Global Light", &global_light_off);
        ImGui::SliderFloat3("Position", &global_light.position.x, -50.0f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::ColorEdit3("Ambient", &global_light.ambient.x);
        ImGui::ColorEdit3("Diffuse", &global_light.diffuse.x);
        ImGui::ColorEdit3("Specular", &global_light.specular.x);
        ImGui::SliderFloat("Exposure", &exposure, 0.1f, 10.0f);
        ImGui::NewLine();
        ImGui::NewLine();
        ImGui::Checkbox("Turn Off Local Lights", &local_lights_off);
        ImGui::Checkbox("Distinct Radius for Local Lights", &hard_edges);
        ImGui::SliderFloat3("Position", &local_lights.back().position.x, -20.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::ColorEdit3("Colour", &local_lights.back().colour.x);
        ImGui::SliderFloat("Radius", &local_lights.back().radius, 0.5f, 10.0f);
    }
    ImGui::End();
    ImGui::Begin("Floor Settings"); {
        ImGui::SliderFloat3("Position", &objects[0]->transform.position.x, -20.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::ColorEdit3("Diffuse", &objects[0]->material.diffuse.x);
        ImGui::ColorEdit3("Specular", &objects[0]->material.specular.x);
        ImGui::SliderFloat("Roughness", &objects[0]->material.roughness, 0.0f, 1.0f);
    }
    ImGui::End();
    ImGui::Begin("Backpack Settings"); {
        ImGui::SliderFloat3("Position", &objects[3]->transform.position.x, -20.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::ColorEdit3("Diffuse", &objects[3]->material.diffuse.x);
        ImGui::ColorEdit3("Specular", &objects[3]->material.specular.x);
        ImGui::SliderFloat("Roughness", &objects[3]->material.roughness, 0.0f, 1.0f);
    }
    ImGui::End();
    ImGui::Begin("Teapot 0 Settings"); {
        ImGui::SliderFloat3("Position", &objects[1]->transform.position.x, -20.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::ColorEdit3("Diffuse", &objects[1]->material.diffuse.x);
        ImGui::ColorEdit3("Specular", &objects[1]->material.specular.x);
        ImGui::SliderFloat("Roughness", &objects[1]->material.roughness, 0.0f, 1.0f);
    }
    ImGui::End();
    ImGui::Begin("Teapot 1 Settings"); {
        ImGui::SliderFloat3("Position", &objects[2]->transform.position.x, -20.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::ColorEdit3("Diffuse", &objects[2]->material.diffuse.x);
        ImGui::ColorEdit3("Specular", &objects[2]->material.specular.x);
        ImGui::SliderFloat("Roughness", &objects[2]->material.roughness, 0.0f, 1.0f);
    }
    ImGui::End();
    ImGui::Begin("Shadow Settings"); {
        ImGui::SliderInt("Kernel Halfwidth (Blur Intensity)", &kernel_halfwidth, 0, 50, "%d", ImGuiSliderFlags_AlwaysClamp);
        //ImGui::Checkbox("Filter Toggle", &conv_or_sat);
        //ImGui::Text("Checked = Convolutional Blur Filter");
        //ImGui::Text("Unchecked = Summed Area Table Blur Filter");
        ImGui::Checkbox("Shadow Intensity Toggle", &var_or_msm);
        ImGui::Text("Checked = Variance Method");
        ImGui::Text("Unchecked = Hamburger 4-MSM Method");
        ImGui::NewLine();
        ImGui::NewLine();
        ImGui::Text("Shadow Buffer:");
        ImGui::Image((ImTextureID)shadow_fbo->texture_id[0],
            ImVec2(static_cast<float>(shadow_fbo->GetWidth()) / 5.0f,
                static_cast<float>(shadow_fbo->GetHeight()) / 5.0f),
            ImVec2(0, 1), ImVec2(1, 0));
        ImGui::NewLine();
    }
    ImGui::End();
    //ImGui::Begin("G-Buffers"); {
    //    for (uint8_t i = 0; i < 4; ++i) {
    //        ImGui::Image((ImTextureID)g_buffer_fbo->texture_id[i],
    //            ImVec2(static_cast<float>(g_buffer_fbo->GetWidth()) / 5.0f,
    //                static_cast<float>(g_buffer_fbo->GetHeight()) / 5.0f),
    //            ImVec2(0, 1), ImVec2(1, 0));
    //        ImGui::NewLine();
    //    }
    //}
    //ImGui::End();
    ImGui::Begin("Skydome"); {
        ImGui::Checkbox("Skydome", &is_skydome);
    }
    ImGui::End();
    ImGui::Begin("Viewer"); {
        ImGui::Text("Left           =       A or NumPad 4");
        ImGui::Text("Right          =       D or NumPad 6");
        ImGui::Text("Forward        =       W or NumPad 8");
        ImGui::Text("Backward       =       S or NumPad 2");
        ImGui::NewLine();
        ImGui::Text("Look Up        =       Up Arrow");
        ImGui::Text("Look Down      =       Down Arrow");
        ImGui::Text("Look Left      =       Left Arrow or NumPad 0");
        ImGui::Text("Look Right     =       Right Arrow or NumPad Period (.)");
        ImGui::NewLine();
        ImGui::Text("Zoom In        =       Scroll Up or Right Shift or NumPad Plus (+)");
        ImGui::Text("Zoom Out       =       Scroll Down or Right Control or NumPad Minus (-)");
        ImGui::NewLine();
        ImGui::Text("Reset Camera   =       Tab or NumPad 5");
        ImGui::NewLine();
        ImGui::Text("Current FPS    =       %f", p_frame_rate_manager->FrameRate());
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsManager::SwapBuffers() {
    glfwPollEvents();
    glfwSwapBuffers(p_window);
}

// HELPERS

void GraphicsManager::SetupGrid(int slices) {
    for (int i = -slices; i < slices; ++i) {
        std::vector<glm::vec3> col;
        for (int j = -slices; j < slices; ++j) {
            col.push_back(glm::vec3{ i, 0.0f, j });
        }
        grid_cols.push_back(col);
    }
    for (int i = -slices; i < slices; ++i) {
        std::vector<glm::vec3> row;
        for (int j = -slices; j < slices; ++j) {
            row.push_back(glm::vec3{ j, 0.0f, i });
        }
        grid_rows.push_back(row);
    }
}

void GraphicsManager::DrawGrid() {
    uint32_t grid_vao, grid_vbo;
    glLineWidth(1.0f);

    glGenVertexArrays(1, &grid_vao);
    glBindVertexArray(grid_vao);

    glGenBuffers(1, &grid_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);

    for (int i = 0; i < grid_cols.size(); ++i) {
        for (int j = 0; j < grid_cols.size() - 1; ++j) {
            float p0[6]{
                grid_cols[i][j].x,
                grid_cols[i][j].y,
                grid_cols[i][j].z,
                grid_cols[i][j + 1].x,
                grid_cols[i][j + 1].y,
                grid_cols[i][j + 1].z
            };
            glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), &p0[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glDrawArrays(GL_LINES, 0, 2);
        }
    }

    glBindVertexArray(0);
    glDeleteBuffers(1, &grid_vbo);
    glDeleteVertexArrays(1, &grid_vao);

    glGenVertexArrays(1, &grid_vao);
    glBindVertexArray(grid_vao);

    glGenBuffers(1, &grid_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);

    for (int i = 0; i < grid_rows.size(); ++i) {
        for (int j = 0; j < grid_rows.size() - 1; ++j) {
            float p0[6]{
                grid_rows[i][j].x,
                grid_rows[i][j].y,
                grid_rows[i][j].z,
                grid_rows[i][j + 1].x,
                grid_rows[i][j + 1].y,
                grid_rows[i][j + 1].z
            };
            glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), &p0[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glDrawArrays(GL_LINES, 0, 2);
        }
    }

    glBindVertexArray(0);
    glDeleteBuffers(1, &grid_vbo);
    glDeleteVertexArrays(1, &grid_vao);
}

void GraphicsManager::SetupQuad() {
    float vertices[12] = {
        -1,  1, 0,
        -1, -1, 0,
         1, -1, 0,
         1,  1, 0
    };
    uint32_t indices[6] = {
        0, 1, 2,
        0, 2, 3
    };

    // create buffers/arrays
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glGenBuffers(1, &quad_ebo);

    // bind
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    // load data into vertex buffers
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    // unbind buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //unbind array
    glBindVertexArray(0);
}

void GraphicsManager::DrawFullScreenQuad() {
    glBindVertexArray(quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GraphicsManager::SetupMesh() {
    // sphere mesh
    std::vector<Vertex> sphere_vertices;
    std::vector<uint32_t> sphere_indices;
    std::vector<Texture*> sphere_textures;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            Vertex vertex{};

            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            // position
            vertex.position = glm::vec3(xPos, yPos, zPos);
            // normal
            vertex.normal = glm::vec3(xPos, yPos, zPos);
            // uv
            vertex.tex_coords = glm::vec2(xSegment, ySegment);

            sphere_vertices.push_back(vertex);
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                sphere_indices.push_back(y * (X_SEGMENTS + 1) + x);
                sphere_indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                sphere_indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                sphere_indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }

    sphere_mesh = new Mesh{ sphere_vertices, sphere_indices, sphere_textures };
}

void GraphicsManager::SetupModels() {
    // Add multiple non-modifiable objects
    //for (uint16_t i = 0; i < 5; ++i) {
    //    for (uint16_t j = 0; j < 5; ++j) {
    //        Object* object = new Object();

    //        // model
    //        object->model = new Model("..\\resources\\objects\\cow-nonormals.obj");

    //        // material
    //        object->material.diffuse = glm::vec3{ 0.5f, 0.5f, 0.3f };
    //        object->material.specular = glm::vec3{ 0.8f, 0.8f, 0.8f };
    //        object->material.roughness = 0.9f;

    //        // transform
    //        object->transform.position = glm::vec3{ -22.5f + (i * 7.5f), 2.0f, -22.5f + (j * 7.5f) };
    //        object->transform.scale = glm::vec3{ 0.6f, 0.6f, 0.6f };

    //        objects.push_back(object);
    //    }
    //}

    // Add 1 floor
    Object* floor = new Object();
    // model
    floor->model = new Model("..\\resources\\objects\\cube.obj");
    // material
    floor->material.diffuse = glm::vec3{ 0.9f, 0.9f, 0.9f };
    floor->material.specular = glm::vec3{ 0.8f, 0.8f, 0.8f };
    floor->material.roughness = 0.2f;
    // transform
    floor->transform.position = glm::vec3{ 0.0f, 0.0f, 0.0f };
    floor->transform.scale = glm::vec3{ 50.0f, 0.1f, 50.0f };
    objects.push_back(floor);

    // Teapot 0
    Object* teapot_0 = new Object();
    // model
    teapot_0->model = new Model("..\\resources\\objects\\teapot.fbx");
    // material
    teapot_0->material.diffuse = glm::vec3{ 0.5f, 0.5f, 0.3f };
    teapot_0->material.specular = glm::vec3{ 0.8f, 0.8f, 0.8f };
    teapot_0->material.roughness = 0.9f;
    // transform
    teapot_0->transform.position = glm::vec3{ -10.0f, 3.0f, 2.0f };
    teapot_0->transform.scale = glm::vec3{ 4.0f };
    objects.push_back(teapot_0);

    // Teapot 1
    Object* teapot_1 = new Object();
    // model
    teapot_1->model = new Model("..\\resources\\objects\\teapot.fbx");
    // material
    teapot_1->material.diffuse = glm::vec3{ 0.5f, 0.5f, 0.3f };
    teapot_1->material.specular = glm::vec3{ 0.8f, 0.8f, 0.8f };
    teapot_1->material.roughness = 0.9f;
    // transform
    teapot_1->transform.position = glm::vec3{ 10.0f, 3.0f, 2.0f };
    teapot_1->transform.scale = glm::vec3{ 4.0f };
    objects.push_back(teapot_1);

    // Backpack
    Object* backpack = new Object();
    // model
    backpack->model = new Model("..\\resources\\objects\\backpack.obj");
    // material
    backpack->material.diffuse = glm::vec3{ 1.0f, 0.5f, 0.31f };
    backpack->material.specular = glm::vec3{ 0.8f, 0.8f, 0.8f };
    backpack->material.roughness = 0.3f;
    // transform
    backpack->transform.position.y = 10.0f;
    objects.push_back(backpack);
}

void GraphicsManager::SetupLocalLights() {
    // Add multiple non-modifiable local lights
    for (int16_t i = -50; i <= 50; i += 5) {
        for (int16_t j = -50; j <= 50; j += 5) {
            LocalLight temp_local_light{};

            temp_local_light.position = glm::vec3{ i, 3.0f, j };
            temp_local_light.colour = glm::vec3{ 0.0f, 1.0f, 1.5f };
            temp_local_light.radius = 3.0f;

            local_lights.push_back(temp_local_light);
        }
    }

    // Add 1 modifiable local light
    LocalLight local_light{};

    local_light.position = glm::vec3{ 0.0f, 2.0f, 2.0f };
    local_light.colour = glm::vec3{ 2.0f, 1.5f, 1.5f };
    local_light.radius = 4.0f;

    local_lights.push_back(local_light);
}

void GraphicsManager::GBufferPass() {
    g_buffer_fbo->Bind();

    ClearBuffer(glm::vec4{ background_color, 1.0f });

    // activate lighting shader
    g_buffer_shader->Use();

    // view/projection transformations
    projection = glm::perspective(glm::radians(camera->zoom), (float)window_width / (float)window_height, 0.1f, 300.0f);
    g_buffer_shader->SetMat4("projection", projection);

    DrawSkydome(is_skydome);

    view = camera->GetViewMat();
    g_buffer_shader->SetMat4("view", view);

    // all objects
    for (uint16_t i = 0; i < objects.size(); ++i) {
        Object* object = objects[i];

        // object properties
        g_buffer_shader->SetVec3("material.diffuse", object->material.diffuse);
        g_buffer_shader->SetVec3("material.specular", object->material.specular);
        g_buffer_shader->SetFloat("material.roughness", object->material.roughness);

        // model matrix
        model = glm::translate(glm::mat4{ 1.0f }, object->transform.position)
            * glm::scale(glm::mat4{ 1.0f }, object->transform.scale);
        g_buffer_shader->SetMat4("model", model);

        // draw object
        object->model->Draw(*g_buffer_shader);
    }

    // unbind g-buffer fbo
    g_buffer_fbo->Unbind();

    g_buffer_shader->Unuse();
}

void GraphicsManager::ShadowPass() {
    shadow_fbo->Bind();

    EnableFrontFaceCulling();

    EnableDepthTest();

    ClearBuffer(glm::vec4{ 0.0f });

    // activate shadow shader
    shadow_shader->Use();

    // view/projection transformations
    projection = ShadowProj();
    view = ShadowView(global_light.position, shadow_target);
    shadow_shader->SetMat4("projection", projection);
    shadow_shader->SetMat4("view", view);

    // depth info
    float light_dist = glm::length(global_light.position);
    float min_depth = light_dist - 35.0f;
    float max_depth = light_dist + 55.0f;
    shadow_shader->SetFloat("min_depth", min_depth);
    shadow_shader->SetFloat("max_depth", max_depth);

    // all objects
    for (uint16_t i = 0; i < objects.size(); ++i) {
        Object* object = objects[i];

        // model matrix
        model = glm::translate(glm::mat4{ 1.0f }, object->transform.position)
            * glm::scale(glm::mat4{ 1.0f }, object->transform.scale);
        shadow_shader->SetMat4("model", model);

        // draw object
        object->model->Draw(*shadow_shader);
    }

    EnableBackFaceCulling();

    // unbind shadow fbo
    shadow_fbo->Unbind();

    shadow_shader->Unuse();

    // set viewport back to window size
    glViewport(0, 0, window_width, window_height);
}

void GraphicsManager::ConvolutionBlurPass() {
    uint32_t kernel_size = 2 * kernel_halfwidth + 1;
    // s controls the width of the bell curve to match the number of desired weights
    float s = static_cast<float>(kernel_halfwidth) / 2.0f;
    if (kernel_halfwidth == 0) {
        s = 1.0f;
    }

    float sum = 0.0f;
    std::vector<float> weights(kernel_size);
    for (int i = 0; i < kernel_size; ++i) {
        float exponent = -0.5f * ((i - kernel_halfwidth) / s) * ((i - kernel_halfwidth) / s);
        weights[i] = expf(exponent);
        sum += weights[i];
    }

    // normalize
    for (int i = 0; i < kernel_size; ++i) {
        weights[i] /= sum;
    }

    // horizontal pass
    blur_hori_shader->Use();

    // send 2 images to shader
    // input image                                                   0 = input image
    shadow_fbo->BindReadImageTexture(blur_hori_shader->id, "src", 0, 0);
    // output image                                                   1 = output image
    shadow_fbo->BindWriteImageTexture(blur_hori_shader->id, "dst", 0, 1);

    // send block of weights to shader as an uniform block
    // start at 0, increment for other blocks
    uint32_t bindpoint = 0;

    int loc = glGetUniformBlockIndex(blur_hori_shader->id, "blur_kernel");
    glUniformBlockBinding(blur_hori_shader->id, loc, bindpoint);

    glBindBuffer(GL_UNIFORM_BUFFER, block_id);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, block_id);
    glBufferData(GL_UNIFORM_BUFFER, weights.size() * sizeof(float), weights.data(), GL_STATIC_DRAW);

    // send kernel halfwidth to shader
    blur_hori_shader->SetInt("w", kernel_halfwidth);

    // tiles WxH images with groups sized 128x1
    glDispatchCompute(shadow_fbo->GetWidth() / 128, shadow_fbo->GetHeight(), 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    blur_hori_shader->Unuse();

    // vertical pass
    blur_vert_shader->Use();

    // send 2 images to shader
    // input image                                                   0 = input image
    shadow_fbo->BindReadImageTexture(blur_vert_shader->id, "src", 0, 0);
    // output image                                                   1 = output image
    shadow_fbo->BindWriteImageTexture(blur_vert_shader->id, "dst", 0, 1);

    // send block of weights to shader as an uniform block
    // start at 0, increment for other blocks
    bindpoint = 0;

    loc = glGetUniformBlockIndex(blur_vert_shader->id, "blur_kernel");
    glUniformBlockBinding(blur_vert_shader->id, loc, bindpoint);

    glBindBuffer(GL_UNIFORM_BUFFER, block_id);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, block_id);
    glBufferData(GL_UNIFORM_BUFFER, weights.size() * sizeof(float), weights.data(), GL_STATIC_DRAW);

    // send kernel halfwidth to shader
    blur_vert_shader->SetInt("w", kernel_halfwidth);

    // tiles WxH images with groups sized 1x128
    glDispatchCompute(shadow_fbo->GetWidth(), shadow_fbo->GetHeight() / 128, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    blur_vert_shader->Unuse();
}

void GraphicsManager::SummedAreaTableBlurPass() {
    // horizontal pass
    sat_shader->Use();
    
    // send 2 images to shader
    // input image                                             0 = input image
    shadow_fbo->BindReadImageTexture(sat_shader->id, "src", 0, 0);
    // output image                                             1 = output image
    shadow_fbo->BindWriteImageTexture(sat_shader->id, "dst", 0, 1);

    uint32_t delta = 1;
    uint32_t img = 0;
    while (delta < shadow_fbo->GetHeight()) {
        // set uniform ivec2 delta for horizontal pass
        sat_shader->SetIVec2("delta", glm::ivec2(delta, 0));

        // dispatch shader
        glDispatchCompute(16, 16, 1);

        // increment delta
        delta *= 2;

        // ping-pong (exchange) src and dst
        // input image
        shadow_fbo->BindReadImageTexture(sat_shader->id, "src", 0, (++img) % 2);
        // output image
        shadow_fbo->BindWriteImageTexture(sat_shader->id, "dst", 0, (img + 1) % 2);
    }

    // vertical pass
    
    //// send 2 images to shader
    //// input image                                             0 = input image
    //shadow_fbo->BindReadImageTexture(sat_shader->id, "src", 0, 0);
    //// output image                                             1 = output image
    //shadow_fbo->BindWriteImageTexture(sat_shader->id, "dst", 0, 1);

    delta = 1;
    img = 0;
    while (delta < shadow_fbo->GetWidth()) {
        // set uniform ivec2 delta for vertical pass
        sat_shader->SetIVec2("delta", glm::ivec2(0, delta));

        // dispatch shader
        glDispatchCompute(16, 16, 1);

        // increment delta
        delta *= 2;

        // ping-pong (exchange) src and dst
        // input image
        shadow_fbo->BindReadImageTexture(sat_shader->id, "src", 0, (++img) % 2);
        // output image
        shadow_fbo->BindWriteImageTexture(sat_shader->id, "dst", 0, (img + 1) % 2);
    }

    sat_shader->Unuse();
}

void GraphicsManager::DeferredLightPass() {
    ClearBuffer(glm::vec4{ background_color, 1.0f });

    light_shader->Use();

    // g-buffers
    g_buffer_fbo->BindTexture(light_shader->id, "g_world_pos", 0, 0);
    g_buffer_fbo->BindTexture(light_shader->id, "g_world_norm", 1, 1);
    g_buffer_fbo->BindTexture(light_shader->id, "g_diffuse_colour", 2, 2);
    g_buffer_fbo->BindTexture(light_shader->id, "g_specular_colour", 3, 3);
    light_shader->SetUInt("buffer_width", g_buffer_fbo->GetWidth());
    light_shader->SetUInt("buffer_height", g_buffer_fbo->GetHeight());

    // shadow map
    shadow_fbo->BindTexture(light_shader->id, "shadow_map", 0, 4);
    light_shader->SetMat4("shadow_mat", ShadowMatrix(global_light.position));
    light_shader->SetBool("var_or_msm", var_or_msm);

    // depth info
    float light_dist = glm::length(global_light.position);
    float min_depth = light_dist - 35.0f;
    float max_depth = light_dist + 55.0f;
    light_shader->SetFloat("min_depth", min_depth);
    light_shader->SetFloat("max_depth", max_depth);

    // IBL
    light_shader->SetBool("is_ibl", is_skydome);
    skydome_tex->Bind(light_shader->id, 5, "skydome_map");
    irradiance_tex->Bind(light_shader->id, 6, "irradiance_map");

    uint32_t bindpoint = 1;
    int loc = glGetUniformBlockIndex(light_shader->id, "HammersleyBlock");
    glUniformBlockBinding(light_shader->id, loc, bindpoint);

    glBindBuffer(GL_UNIFORM_BUFFER, hammersley_block_id);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, hammersley_block_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) + (sizeof(float) * hammersley_block->hammersley.size()),
                    &hammersley_block, GL_STATIC_DRAW);
    // to explicitly pass data
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &hammersley_block->n);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float), sizeof(float) * hammersley_block->hammersley.size(),
        hammersley_block->hammersley.data());

    light_shader->SetFloat("exposure", exposure); 

    // light properties
    light_shader->SetVec3("global_light.position", global_light.position);
    light_shader->SetVec3("global_light.ambient", global_light.ambient);
    light_shader->SetVec3("global_light.diffuse", global_light.diffuse);
    light_shader->SetVec3("global_light.specular", global_light.specular);

    light_shader->SetVec3("view_pos", camera->position);
    light_shader->SetBool("global_light_off", global_light_off);

    // draw quad on screen
    DrawFullScreenQuad();

    // unbind texture units
    g_buffer_fbo->UnbindTexture(0);
    g_buffer_fbo->UnbindTexture(1);
    g_buffer_fbo->UnbindTexture(2);
    g_buffer_fbo->UnbindTexture(3);
    shadow_fbo->UnbindTexture(4);

    skydome_tex->Unbind();
    irradiance_tex->Unbind();

    light_shader->Unuse();
}

void GraphicsManager::LocalLightPass() {
    DisableDepthTest();

    EnableAdditiveBlend();

    EnableFrontFaceCulling();

    local_lights_shader->Use();

    // g-buffers
    g_buffer_fbo->BindTexture(local_lights_shader->id, "g_world_pos", 0, 0);
    g_buffer_fbo->BindTexture(local_lights_shader->id, "g_world_norm", 1, 1);
    g_buffer_fbo->BindTexture(local_lights_shader->id, "g_diffuse_colour", 2, 2);
    g_buffer_fbo->BindTexture(local_lights_shader->id, "g_specular_colour", 3, 3);
    local_lights_shader->SetUInt("buffer_width", g_buffer_fbo->GetWidth());
    local_lights_shader->SetUInt("buffer_height", g_buffer_fbo->GetHeight());

    projection = glm::perspective(glm::radians(camera->zoom), (float)window_width / (float)window_height, 0.1f, 300.0f);
    local_lights_shader->SetMat4("projection", projection);
    view = camera->GetViewMat();
    local_lights_shader->SetMat4("view", view);

    local_lights_shader->SetVec3("view_pos", camera->position);
    local_lights_shader->SetBool("hard_edges", hard_edges);
    local_lights_shader->SetBool("local_lights_off", local_lights_off);

    // for every local light, these values differ
    for (LocalLight& local_light : local_lights) {
        // light properties
        local_lights_shader->SetVec3("local_light.position", local_light.position);
        local_lights_shader->SetVec3("local_light.colour", local_light.colour);
        local_lights_shader->SetFloat("local_light.radius", local_light.radius);

        model = glm::translate(glm::mat4{ 1.0f }, local_light.position)
            * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ local_light.radius });
        local_lights_shader->SetMat4("model", model);

        sphere_mesh->Draw(*local_lights_shader, DrawMode::TRIANGLE_STRIP);
    }

    local_lights_shader->Unuse();

    EnableBackFaceCulling();

    DisableBlend();

    EnableDepthTest();
}

void GraphicsManager::DrawSkydome(bool is_skydome) {
    if (is_skydome) {
        EnableFrontFaceCulling();

        // cast to mat3 and back to mat4 to discard translation info
        glm::mat4 skydome_view = static_cast<glm::mat4>(static_cast<glm::mat3>(camera->GetViewMat()));
        g_buffer_shader->SetMat4("view", skydome_view);

        // at origin, scale 150
        model = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 150.0f });
        g_buffer_shader->SetMat4("model", model);

        // for skydome
        g_buffer_shader->SetBool("is_skydome", true);
        skydome_tex->Bind(g_buffer_shader->id, 0, "skydome");

        sphere_mesh->Draw(*g_buffer_shader, DrawMode::TRIANGLE_STRIP);

        skydome_tex->Unbind();

        EnableBackFaceCulling();
    }

    // reset mode after drawing skydome
    g_buffer_shader->SetBool("is_skydome", false);
}

void GraphicsManager::SetupHammersleyBlock() {
    // n = 20
    hammersley_block = new HammersleyBlock(20);

    int kk;
    int pos = 0;

    for (int k = 0; k < hammersley_block->n; ++k) {
        kk = k;
        float u = 0.0f;

        for (float p = 0.5f; kk; p *= 0.5, kk >>= 1) {
            if (kk & 1) {
                u += p;
            }
        }

        float v = (k + 0.5f) / hammersley_block->n;

        hammersley_block->hammersley[pos++] = u;
        hammersley_block->hammersley[pos++] = v;
    }

    glGenBuffers(1, &hammersley_block_id);
}

void GraphicsManager::EnableFrontFaceCulling() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
}

void GraphicsManager::EnableBackFaceCulling() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void GraphicsManager::DisableFaceCulling() {
    glDisable(GL_CULL_FACE);
}

void GraphicsManager::EnableDepthTest() {
    glEnable(GL_DEPTH_TEST);
}

void GraphicsManager::DisableDepthTest() {
    glDisable(GL_DEPTH_TEST);
}

void GraphicsManager::EnableMultiSampling() {
    glEnable(GL_MULTISAMPLE);
}

void GraphicsManager::DisableMultiSampling() {
    glDisable(GL_MULTISAMPLE);
}

void GraphicsManager::EnableAdditiveBlend() {
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);
}

void GraphicsManager::DisableBlend() {
    glDisable(GL_BLEND);
}

void GraphicsManager::ClearBuffer(glm::vec4 clear_colour, FBO* buffer) {
    if (buffer != nullptr) {
        buffer->Bind();
    }

    glClearColor(clear_colour.x, clear_colour.y, clear_colour.z, clear_colour.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (buffer != nullptr) {
        buffer->Unbind();
    }
}

// glm has a lookAt(...) function
// this implementation is to understand it
glm::mat4 GraphicsManager::LookAt(glm::vec3 const& light_pos, glm::vec3 const& look_target) const {
    // these form an orthonormal frame, and can form the rows and cols of a rotation matrix
    glm::vec3 V = glm::normalize(look_target - light_pos);
    glm::vec3 A = glm::normalize(glm::cross(V, up));
    glm::vec3 B = glm::cross(A, V);

    // translate to origin
    glm::mat4 matrix = glm::translate(glm::mat4{ 1.0f }, -light_pos);

    // rotation matrix
    glm::mat4 R{
        A.x, B.x, -V.x, 0.0f,
        A.y, B.y, -V.y, 0.0f,
        A.z, B.z, -V.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }; // glm matrices are column major

    // final lookAt matrix
    matrix = R * matrix;

    return matrix;
}

glm::mat4 GraphicsManager::ShadowProj() const {
    return glm::perspectiveFov(glm::radians(45.0f), (float)shadow_fbo->GetWidth(), (float)shadow_fbo->GetHeight(), 0.1f, 100.0f);
}

glm::mat4 GraphicsManager::ShadowView(glm::vec3 const& light_pos, glm::vec3 const& look_target) const {
    return LookAt(light_pos, look_target);
}

glm::mat4 GraphicsManager::ShadowMatrix(glm::vec3 const& light_pos) const {
    return glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.5f })
        * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 0.5f })
        * ShadowProj() 
        * ShadowView(light_pos, shadow_target);
}

void GraphicsManager::FrameBufferSizeCallbackImpl(int width, int height) {
    glViewport(0, 0, width, height);

    window_width = width;
    window_height = height;

    g_buffer_fbo->SetWidth(window_width);
    g_buffer_fbo->SetHeight(window_height);
}

// helper to capture camera control inputs
void GraphicsManager::ProcessCameraMovement() {
    float dt = p_frame_rate_manager->DeltaTime();

    if (p_input_manager->IsKeyPressed(GLFW_KEY_KP_8) || p_input_manager->IsKeyPressed(GLFW_KEY_W)) {
        camera->ProcessKeyboard(CameraMovement::FORWARD, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_KP_2) || p_input_manager->IsKeyPressed(GLFW_KEY_S)) {
        camera->ProcessKeyboard(CameraMovement::BACKWARD, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_KP_4) || p_input_manager->IsKeyPressed(GLFW_KEY_A)) {
        camera->ProcessKeyboard(CameraMovement::LEFT, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_KP_6) || p_input_manager->IsKeyPressed(GLFW_KEY_D)) {
        camera->ProcessKeyboard(CameraMovement::RIGHT, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_UP)) {
        camera->ProcessKeyboard(CameraMovement::PITCHUP, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_DOWN)) {
        camera->ProcessKeyboard(CameraMovement::PITCHDOWN, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_KP_0) || p_input_manager->IsKeyPressed(GLFW_KEY_LEFT)) {
        camera->ProcessKeyboard(CameraMovement::YAWLEFT, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_KP_DECIMAL) || p_input_manager->IsKeyPressed(GLFW_KEY_RIGHT)) {
        camera->ProcessKeyboard(CameraMovement::YAWRIGHT, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_KP_SUBTRACT) || p_input_manager->IsKeyPressed(GLFW_KEY_RIGHT_CONTROL)) {
        camera->ProcessKeyboard(CameraMovement::ZOOMOUT, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_KP_ADD) || p_input_manager->IsKeyPressed(GLFW_KEY_RIGHT_SHIFT)) {
        camera->ProcessKeyboard(CameraMovement::ZOOMIN, dt);
    }
    if (p_input_manager->IsKeyPressed(GLFW_KEY_KP_5) || p_input_manager->IsKeyPressed(GLFW_KEY_TAB)) {
        camera->ProcessKeyboard(CameraMovement::RESET, dt);
    }

    //camera->ProcessMouseMovement(p_input_manager->GetMouseDelta());
}

// CALLBACKS

void FrameBufferSizeCallback(GLFWwindow* p_window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.

    p_graphics_manager->FrameBufferSizeCallbackImpl(width, height);
}

void ScrollCallback(GLFWwindow* p_window, double x_ofset, double y_offset) {
    p_graphics_manager->camera->ProcessMouseScroll(static_cast<float>(y_offset));
}