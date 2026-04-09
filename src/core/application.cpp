#include <stdlib.h>
#include <math.h>


#define COLOR_RGB(r,g,b) ((u64)(((u8)(b)|((u16)((u8)(g))<<8))|(((u64)(u8)(r))<<16)))
#define NumSoldiers 5
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
void DrawGrid(app_backbuffer* buffer, vec2 topLeft, vec2 bottomRight, u32 gridCellDimension);
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
void DrawVector(app_backbuffer* buffer, vec2 startPoint, vec2 direction, u32 color = 0xff22ff, f32 scale = 1.0f)
{
    vec2 endPoint = startPoint + direction;
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
void DrawTriangle(app_backbuffer* buffer, vec2 A, vec2 B, vec2 C, f32 heading = 0.0f, u32 color = 0x00ff00)
{
    // NOTE: width is 'height' of triangle, while height is the base, 
    // cause i want the triangle to be along X axis by default

    DrawLine(buffer, A, C, color);
    DrawLine(buffer, B, C, color);
    DrawLine(buffer, A, B, color);

}

//  SECTION  scalar & vector grid
inline
vec2 GetCellPosition(u32 grid_index, u32 gridCountX, u32 gridCountY, u32 gridCellSize)
{
    return {(f32)((u32)grid_index % gridCountX) * gridCellSize,(f32)((u32)grid_index / gridCountY) * gridCellSize};
}
inline
u32 GetGridIndex(vec2 position, u32 gridCountX, u32 gridCountY, u32 gridCellSize)
{
    u32 cellX = position.x / gridCellSize;
    u32 cellY = position.y / gridCellSize;
    u32 grid_index = cellX + cellY * gridCountX;
    return grid_index;
}

//  SECTION  Math

vec2 Normalize(vec2 v)
{
    f32 norm = sqrt(v.x*v.x + v.y*v.y);
    return {v.x/norm, v.y/norm};
}
f32 Random_f32(f32 lb, f32 hb)
{
    // NOTE: rand() is from stdlib.h and return a random value between 0 and RAND_MAX
    f32 result = lb + ((f32)rand() / (f32)(RAND_MAX)) * (hb - lb);
    return result;
}
vec2 Random_vec2(vec2 lb, vec2 hb)
{ 
    return {Random_f32(lb.x, hb.x),Random_f32(lb.y,hb.y)};
}
vec2 Random_vec2(f32 lb, f32 hb)
{ 
    return {Random_f32(lb, hb),Random_f32(lb,hb)};
}
//  SECTION  Main loop
void ApplicationUpdateAndRender(app_memory* appMemory, app_backbuffer* appBackbuffer, app_inputs* appInputs)
{

    // Grid info
    constexpr u32 gridCountX = 10;
    constexpr u32 gridCountY = 10;
    constexpr u32 gridCount  = gridCountX * gridCountY;
    u32 gridCellSize       = (1<<4);
    f32 padding            = 100.0f;
    vec2 gridCornerTopLeft      = {padding,padding};
    vec2 gridCornerBottomRight  = {padding + (f32)(gridCountX*gridCellSize),padding + (f32)(gridCountY*gridCellSize)};
    f32 gridWidth  = gridCornerBottomRight.x - gridCornerTopLeft.x;
    f32 gridHeight = gridCornerBottomRight.y - gridCornerTopLeft.y;

    f32 scalarField[gridCount];
    vec2 gradientField[gridCount];
    // INFO  Initialization of memory
    ASSERT(sizeof(app_state) <= appMemory->permanentStorageSize);
    app_state* appState = (app_state*)appMemory->permanentStorage;
    if(!appMemory->isInitialized)
    {
        appState->offsets = {};
        appState->offsets.x = 20.0f;
        appState->offsets.y = 20.0f;
        appState->pos = {0,0};

        for (i32 grid_index = 0; grid_index < gridCount; ++grid_index)
            scalarField[grid_index] = 0.0f;

        for(i32 i=0; i<NumSoldiers ; ++i)
        {
            appState->soldiers.position[i]     = Random_vec2(gridCornerTopLeft, gridCornerBottomRight);
            appState->soldiers.velocity[i]     = Random_vec2(-5.0f,5.0f);
            appState->soldiers.acceleration[i] = Random_vec2(-5.0f,5.0f);;
            appState->soldiers.heading[i]      = Random_f32(0.0f,3.0f);
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

    // clearing scalar field
    for (i32 grid_index = 0; grid_index < gridCount; ++grid_index)
        scalarField[grid_index] = 0.0f;

    for(int i=0; i<NumSoldiers ; ++i)
    {
        if(appState->soldiers.position[i].x > 0 && appState->soldiers.position[i].y>0 &&
        appState->soldiers.position[i].x < WIDTH && appState->soldiers.position[i].y < HEIGHT) 
        {
            // Convert to grid Reference frame
            vec2 position_Rg = appState->soldiers.position[i] - gridCornerTopLeft;
            u32 cellX = position_Rg.x / gridCellSize;
            u32 cellY = position_Rg.y / gridCellSize;
            u32 grid_index = cellX + cellY * gridCountX;
            scalarField[grid_index] += 2;
        }
    }
    
    


    for(int i=0; i<NumSoldiers ; ++i)
    {
        // Boids "bounce" over edges from padding
        
        if(appState->soldiers.position[i].x < gridCornerTopLeft.x || appState->soldiers.position[i].x > gridCornerBottomRight.x)
        {
            appState->soldiers.velocity[i] = {-appState->soldiers.velocity[i].x,appState->soldiers.velocity[i].y};
        }
        if(appState->soldiers.position[i].y < gridCornerTopLeft.y || appState->soldiers.position[i].y > gridCornerBottomRight.y)
        {
            appState->soldiers.velocity[i] = {appState->soldiers.velocity[i].x,-appState->soldiers.velocity[i].y};
        }

        if(appState->soldiers.position[i].x < 0 || appState->soldiers.position[i].y < 0)
            continue;
        // compoute gradient ...
        vec2 position_Rg = appState->soldiers.position[i] - gridCornerTopLeft;
        u32 grid_index = GetGridIndex(position_Rg, gridCountX, gridCountY, gridCellSize);
        u32 cellX = position_Rg.x / gridCellSize;
        u32 cellY = position_Rg.y / gridCellSize;
        i32 idx_left   = (cellX == 0) ? grid_index : grid_index-1;
        i32 idx_right  = (cellX == gridCountX) ? grid_index : grid_index+1;
        i32 idx_top = (cellY == 0) ? grid_index : grid_index-gridCountX;
        i32 idx_bottom  = (cellY == gridCountY) ? grid_index : grid_index+gridCountX;

        f32 density_x = scalarField[idx_right] - scalarField[idx_left];
        f32 density_y = scalarField[idx_bottom] - scalarField[idx_top];
        vec2 gradientAvoidance = {-density_x/2.0f,-density_y/2.0f};

        appState->soldiers.acceleration[i] = gradientAvoidance;
        appState->soldiers.velocity[i] = appState->soldiers.velocity[i] + 0.5f * appState->soldiers.acceleration[i];
        //appState->soldiers.position[i] = appState->soldiers.position[i] + 0.01f * appState->soldiers.velocity[i];
    }

    // SECTION  RENDERING
    RenderToBuffer(appBackbuffer, (i32)appState->offsets.x, (i32)appState->offsets.y); // Clear to color TODO: update this
    DrawGrid(appBackbuffer, gridCornerTopLeft, {gridWidth,gridHeight}, gridCellSize); // Draw some grid

    DrawRectangleFilled(appBackbuffer,appState->pos.x,appState->pos.y,4,4);

    for(int i=0; i<NumSoldiers ; ++i)
    {
        vec2 position = appState->soldiers.position[i];
        //DrawRectangleFilled(appBackbuffer,appState->soldiers.position[i].x,appState->soldiers.position[i].y,8.0f,8.0f);
        vec2 unitVelocity = Normalize(appState->soldiers.velocity[i]);
        vec2 A = position + 3.0f*vec2({+unitVelocity.y, -unitVelocity.x}); 
        vec2 B = position + 3.0f*vec2({-unitVelocity.y, +unitVelocity.x}); 
        vec2 C = position + 9.0f*unitVelocity;
        DrawTriangle(appBackbuffer,A,B,C);
        
        //DrawVector(appBackbuffer, position, appState->soldiers.velocity[i], 0xff0000, 2.0f); // Draw velocity vector
        //DrawVector(appBackbuffer, position + 2.0f*appState->soldiers.velocity[i], appState->soldiers.acceleration[i], 0xff00ff, 10.0f); // Draw acceleration vector (at velocity tip)
    }

    // render a rectangle for density in each cell
    for(i32 grid_index = 0; grid_index < gridCount;++grid_index)
    {
        vec2 cellPosition = GetCellPosition(grid_index, gridCountX, gridCountY, gridCellSize);
        vec2 centerOffset = {(f32)gridCellSize/2.0f,(f32)gridCellSize/2.0f};
        cellPosition = gridCornerTopLeft + cellPosition + centerOffset; // Convert to R_I
        f32 radius = scalarField[grid_index];
        DrawRectangleFilled(appBackbuffer,cellPosition.x,cellPosition.y,radius,radius, 0x0000ff);
    }

    // render gradient field
    for(i32 grid_index = 0; grid_index < gridCount;++grid_index)
    {
        vec2 cellPosition = GetCellPosition(grid_index, gridCountX, gridCountY, gridCellSize);
        vec2 centerOffset = {(f32)gridCellSize/2.0f,(f32)gridCellSize/2.0f};
        cellPosition = gridCornerTopLeft + cellPosition + centerOffset; // Convert to R_I


        u32 cellX = grid_index % gridCellSize;
        u32 cellY = grid_index / gridCellSize;
        i32 idx_left   = (cellX == 0) ? grid_index : grid_index-1;
        i32 idx_right  = (cellX == gridCountX) ? grid_index : grid_index+1;
        i32 idx_top = (cellY == 0) ? grid_index : grid_index-gridCountX;
        i32 idx_bottom  = (cellY == gridCountY) ? grid_index : grid_index+gridCountX;

        f32 density_x = scalarField[idx_right] - scalarField[idx_left];
        f32 density_y = scalarField[idx_bottom] - scalarField[idx_top];
        vec2 gradientAvoidance = {-density_x/2.0f,-density_y/2.0f};

        DrawVector(appBackbuffer, cellPosition, gradientAvoidance, 0xff0000, 5.0f); // Draw gradient
    }
    // Tests
    DrawLine(appBackbuffer,{50,50}, appState->pos);
    DrawRectangle(appBackbuffer, gridCornerTopLeft.x, gridCornerTopLeft.y, gridWidth, gridHeight); 

}



// SECTION  Implementation of renderer (imcomplete)
void DrawGrid(app_backbuffer* buffer, vec2 topLeft, vec2 dim, u32 gridCellDimension)
{
    u32 gridColor = 0x222222;
    for (int Y = topLeft.y; Y < topLeft.y + dim.y; Y= Y + gridCellDimension)
    {
        DrawLine(buffer,{topLeft.x,(f32)Y}, {topLeft.x + dim.x, (f32)Y},gridColor);
    }
    for(int X = topLeft.x;  X < topLeft.x + dim.x; X = X + gridCellDimension)
    {
        DrawLine(buffer,{(f32)X,topLeft.y}, {(f32)X, topLeft.y + dim.y}, gridColor);
    }
}