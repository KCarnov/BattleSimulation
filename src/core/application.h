#pragma once
#include "../defines.h"



/// NOTE: Services the application provides to the platform

struct app_backbuffer
{
    void* memory;
    u16 width;
    u16 height;
    u32 stride;
    u32 bytesPerPixel;
};

struct mouse_state
{
    b32 isPressed = 0;
    union{
        struct {
            f32 mouseX;
            f32 mouseY;
        };
        vec2 mousePosition;
    };
};

struct key_state
{
    b32 isPressed = 0;
};

struct app_inputs
{
    key_state Z;
    key_state S;
    key_state Q;
    key_state D;  
    key_state F; 
    mouse_state mouseInputs;

};

struct app_memory 
{
    u64 permanentStorageSize;
    void* permanentStorage;

    u64 transientStorageSize;
    void* transientStorage;

    b32 isInitialized = FALSE;

    // NOTE: 
    // Hold the memory allocation that will be used throughout the all application life
    // There is technically no need to allocate anything anymore via malloc, we use this 
    // pre-allocated memory !
};

void ApplicationUpdateAndRender(app_memory* appMemory, app_backbuffer* appBackbuffer, app_inputs* appInputs);

////

struct spatial_grid
{
    i32 bucket[WIDTH/(1<<4) * HEIGHT/(1<<4)];
};

struct soldiers
{
    vec2 position[NumSoldiers];
    vec2 velocity[NumSoldiers];
    vec2 acceleration[NumSoldiers];
    i32 next[NumSoldiers];
};

struct app_state
{
    vec2 offsets;
    vec2 pos;
    soldiers soldiers;
    spatial_grid spatialGrid;
}; 

/// NOTE: Services the platform provides the application