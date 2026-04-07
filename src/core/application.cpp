#include <stdlib.h>
#include <math.h>


#define COLOR_RGB(r,g,b) ((u64)(((u8)(b)|((u16)((u8)(g))<<8))|(((u64)(u8)(r))<<16)))
#define NumSoldiers 30000
#include "application.h"

void RenderToBuffer(app_backbuffer* buffer, i32 offsetx, i32 offsety)
{
    u32* pixel = (u32*)buffer->memory;
    for(int Y = 0; Y < buffer->height ; ++Y)
    { 
        for(int X = 0; X < buffer->width ; ++X)
        {
           *pixel++ = COLOR_RGB(25,25,25);
        } 
    }
}

//  SECTION  Renderer ?
void DrawGrid(app_backbuffer* buffer);
void DrawLine(app_backbuffer* buffer, vec2 startPoint, vec2 endPoint, u32 color = 0xff22ff)
{

    // TODO: checks bounds
    if(endPoint.x < 0  | endPoint.y < 0 | startPoint.x < 0 | startPoint.y < 0)   
    { return; } // Discard
    if(startPoint.x >= buffer->width | startPoint.y >= buffer->height | endPoint.x >= buffer->width  | endPoint.y >= buffer->height ) 
    { return; } // Discard

    // Checks TODO: Remake these ...
    // if(startPoint.x<0) {width  = width +x; x=0;}
    // if(startPoint.y<0) {height = height+y; y=0;}
    // if( xBottomRight > buffer->width)  {width  = buffer->width  - x;}
    // if( yBottomRight > buffer->height) {height = buffer->height - y;}

    vec2 dir = endPoint - startPoint;
    f32 n = sqrt(dir.x*dir.x + dir.y*dir.y); // n is in pixel theorically
    dir = dir/n;

    u32* pixel;
    for (int i=0; i<(u32)n;i=i+1)
    {
        pixel = (u32*)buffer->memory + (u32)(((u32)startPoint.y*buffer->width + (u32)startPoint.x));
        *pixel = color;
        startPoint = startPoint + dir;
    }
}
void DrawVector(app_backbuffer* buffer, vec2 startPoint, vec2 endPoint, u32 color = 0xff22ff, f32 scale = 1.0f)
{
    vec2 dir = endPoint - startPoint;
    f32 norm = sqrt(dir.x*dir.x + dir.y*dir.y); // n is in pixel theorically
    vec2 dirn = dir/norm;
    vec2 n = {dirn.y,-dirn.x};
    vec2 newEndPoint = startPoint + scale*dir;
    // Drawing main vector line :
    DrawLine(buffer,startPoint, newEndPoint, color);
    // Drawing vector arrow :
    DrawLine(buffer,newEndPoint, newEndPoint - (5*dirn) - 3*n,color);
    DrawLine(buffer,newEndPoint, newEndPoint - (5*dirn) + 3*n,color);
}
void DrawRectangle(app_backbuffer* buffer, f32 x, f32 y, f32 width, f32 height, u32 color = 0xff22ff)
{
    f32 xBottomRight = x + width;
    f32 yBottomRight = y + height;
    if(xBottomRight < 0  | yBottomRight < 0)   { return; } // Discard
    if(x > buffer->width | y > buffer->height) { return; } // Discard

    // Checks TODO: Remake these ...
    if(x<0) {width  = width +x; x=0;}
    if(y<0) {height = height+y; y=0;}
    if( xBottomRight > buffer->width)  {width  = buffer->width  - x;}
    if( yBottomRight > buffer->height) {height = buffer->height - y;}
    
    DrawLine(buffer, {x,y},{xBottomRight,y}, color);
    DrawLine(buffer, {xBottomRight,y},{xBottomRight, yBottomRight}, color);
    DrawLine(buffer, {xBottomRight, yBottomRight},{x,yBottomRight}, color);
    DrawLine(buffer, {x,yBottomRight},{x,y}, color);
}
void DrawRectangleFilled(app_backbuffer* buffer, f32 x, f32 y, f32 width, f32 height, u32 color = 0x00ff00)
{
    x -= width/2;
    y -= height/2;
    f32 xBottomRight = x + width;
    f32 yBottomRight = y + height;
    if(xBottomRight < 0  | yBottomRight < 0)   { return; } // Discard
    if(x > buffer->width | y > buffer->height) { return; } // Discard

    // Checks TODO: Remake these ...
    if(x<0) {width  = width +x; x=0;}
    if(y<0) {height = height+y; y=0;}
    if( xBottomRight > buffer->width)  {width  = buffer->width  - x;}
    if( yBottomRight > buffer->height) {height = buffer->height - y;}

    u8* row = (u8*)buffer->memory + (u32)(((u32)y*buffer->width + (u32)x)*(buffer->bytesPerPixel));
    for(int Y = 0; Y < (u32)height ; ++Y)
    { 
        u32* pixel = (u32*) row;
        for(int X = 0; X < (u32)width ; ++X)
        {
           *pixel++ = color;
        } 
        row += buffer->stride;
    }
}

//  SECTION  Spatial grid


//  SECTION  Main loop
void ApplicationUpdateAndRender(app_memory* appMemory, app_backbuffer* appBackbuffer, app_inputs* appInputs)
{
    // INFO  Initialization of memory
    ASSERT(sizeof(app_state) <= appMemory->permanentStorageSize);
    app_state* appState = (app_state*)appMemory->permanentStorage;
    if(!appMemory->isInitialized)
    {
        appState->offsets = {};
        appState->offsets.x = 20.0f;
        appState->offsets.y = 20.0f;
        appState->pos = {0,0};

        for (int i = 0; i < WIDTH/(1<<4) * HEIGHT/(1<<4); ++i)
            appState->spatialGrid.bucket[i] = -1;

        for(int i=0; i<NumSoldiers ; ++i)
        {
            appState->soldiers.position[i] =     {(f32)(10 + rand() % (1920-100+1)), (f32)(10 + rand() % (1080-100+1))};
            appState->soldiers.velocity[i] =     {(f32)(-4.0f + rand() % 8),(f32)(-4.0f + rand() % 8)};
            appState->soldiers.acceleration[i] = {(f32)(-0.5f  + rand() % 2),(f32)(-0.5f  + rand() % 2)};
            

            u32 bucketX = appState->soldiers.position[i].x / (1<<4);
            u32 bucketY = appState->soldiers.position[i].y / (1<<4);
            u32 idx = bucketY * WIDTH/(1<<4) + bucketX;

            appState->soldiers.next[i] = appState->spatialGrid.bucket[idx];
            appState->spatialGrid.bucket[idx] = i;
        }

        appMemory->isInitialized = TRUE;
    }

    // INFO  Gameplay Update and Render to buffer
    if (appInputs->D.isPressed) { appState->offsets.x += 10; }
    if (appInputs->Q.isPressed) { appState->offsets.x -= 10; }
    if (appInputs->Z.isPressed) { appState->offsets.y -= 10; }
    if (appInputs->S.isPressed) { appState->offsets.y += 10; }
    if(appInputs->mouseInputs.isPressed) {appState->pos = appInputs->mouseInputs.mousePosition;}


    // SECTION  UPDATE
    for (int i = 0; i < WIDTH/(1<<4) * HEIGHT/(1<<4); ++i)
        appState->spatialGrid.bucket[i] = -1;

    for(int i=0; i<NumSoldiers ; ++i)
    {
        if(appState->soldiers.position[i].x > 0 && appState->soldiers.position[i].y>0 &&
        appState->soldiers.position[i].x < WIDTH && appState->soldiers.position[i].y < HEIGHT) 
        {
            u32 bucketX = appState->soldiers.position[i].x / (1<<4);
            u32 bucketY = appState->soldiers.position[i].y / (1<<4);
            u32 idx = bucketY * WIDTH/(1<<4) + bucketX;
            appState->soldiers.next[i] = appState->spatialGrid.bucket[idx];
            appState->spatialGrid.bucket[idx] = i;
        }
    }

    for(int i=0; i<NumSoldiers ; ++i)
    {
        appState->soldiers.position[i] = appState->soldiers.position[i] + 0.2f * appState->soldiers.velocity[i];
        //appState->soldiers.velocity[i] = appState->soldiers.velocity[i] + 0.005f * appState->soldiers.acceleration[i];
    }

    // SECTION  RENDERING
    RenderToBuffer(appBackbuffer, (i32)appState->offsets.x, (i32)appState->offsets.y); // Clear to color TODO: update this
    DrawGrid(appBackbuffer); // Draw some grid

    DrawRectangleFilled(appBackbuffer,appState->pos.x,appState->pos.y,4,4);

    for(int i=0; i<NumSoldiers ; ++i)
    {
        DrawRectangleFilled(appBackbuffer,appState->soldiers.position[i].x,appState->soldiers.position[i].y,8.0f,8.0f);

        // DrawVector(appBackbuffer,{appState->soldiers.position[i].x,appState->soldiers.position[i].y},
        //                          {appState->soldiers.position[i].x + appState->soldiers.velocity[i].x,
        //                           appState->soldiers.position[i].y + appState->soldiers.velocity[i].y}, 0xff0000, 30.0f);
    }

    u32 bucketX = 20;
    u32 bucketY = 20;
    u32 idx = bucketY * WIDTH/(1<<4) + bucketX;
    DrawRectangle(appBackbuffer, bucketX*(1<<4),bucketY*(1<<4),1<<4,1<<4, 0xff0000); 
    i32  e = appState->spatialGrid.bucket[idx];
    while(e != -1)
    {
        // appState->soldiers.velocity[e].x = 0.0f;
        // appState->soldiers.velocity[e].y = 0.0f;
        
        DrawVector(appBackbuffer,{0.0f,0.0f},{appState->soldiers.position[e].x,appState->soldiers.position[e].y}, 0xff0000);

        e = appState->soldiers.next[e];
    }

    // Tests
    DrawLine(appBackbuffer,{50,50}, appState->pos);
    DrawRectangle(appBackbuffer, 75,199,40,40); 

}



// SECTION  Implementation of renderer (imcomplete)
 
void DrawGrid(app_backbuffer* buffer)
{
    
    static u32 gridCellDimension = 1 << 4;
    u32 gridColor = 0x222222;
    for (int Y=0; Y<buffer->height; Y= Y + gridCellDimension)
    {
        DrawLine(buffer,{0,(f32)Y}, {(f32)buffer->width-1,(f32)Y},gridColor);
    }
    for(int X =0; X<buffer->width; X = X + gridCellDimension)
    {
        DrawLine(buffer,{(f32)X,0}, {(f32)X,(f32)buffer->height-1},gridColor);
    }
}