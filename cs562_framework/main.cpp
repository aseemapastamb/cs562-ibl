#include "stdafx.h"

#include "GraphicsManager.h"
#include "SimManager.h"
#include "InputManager.h"
#include "FrameRateManager.h"
//#include "PhysicsManager.h"

// various managers to control and organize
GraphicsManager* p_graphics_manager = nullptr;
SimManager* p_sim_manager = nullptr;
InputManager* p_input_manager = nullptr;
FrameRateManager* p_frame_rate_manager = nullptr;
//PhysicsManager* p_physics_manager = nullptr;

// allocate all the managers
void CreateManagers() {
    p_graphics_manager = new GraphicsManager();
    p_sim_manager = new SimManager();
    p_input_manager = new InputManager();
    p_frame_rate_manager = new FrameRateManager(60);
    //p_physics_manager = new PhysicsManager();
}

// deallocate the managers
void DestroyManagers() {
    //delete p_physics_manager;
    //p_physics_manager = nullptr;

    delete p_frame_rate_manager;
    p_frame_rate_manager = nullptr;

    delete p_input_manager;
    p_input_manager = nullptr;

    delete p_sim_manager;
    p_sim_manager = nullptr;

    delete p_graphics_manager;
    p_graphics_manager = nullptr;
}

// called once at the start
bool EngineInit() {
    CreateManagers();

    bool status = p_graphics_manager->InitWindow();
    return status;
}

// called once before program exits
void EngineCleanup() {
    p_graphics_manager->DestroyWindow();

    DestroyManagers();
}

int main() {
    bool status = EngineInit();
    if (!status) {
        return -1;
    }

    //p_physics_manager->SetupCloth();
    p_graphics_manager->SetupScene();

    // Render loop
    // -----------
    while (p_sim_manager->Status()) {
        p_frame_rate_manager->FrameStart();
        
        p_input_manager->Update();

        p_sim_manager->Update();

        //p_physics_manager->UpdateCloth(p_frame_rate_manager->DeltaTime());

        p_graphics_manager->RenderScene();
        p_graphics_manager->RenderGUI();
        p_graphics_manager->SwapBuffers();

        p_frame_rate_manager->FrameEnd();
    }

    EngineCleanup();
    return 0;
}

