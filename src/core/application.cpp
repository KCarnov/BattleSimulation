#include <stdlib.h>
#include <math.h>


#define COLOR_RGB(r,g,b) ((u64)(((u8)(b)|((u16)((u8)(g))<<8))|(((u64)(u8)(r))<<16)))
#define NumSoldiers 10000
#include "application.h"

void RenderToBuffer(app_backbuffer* buffer)
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

void BuildDensityField(f32* grid, vec2 position, u32 gridCountX, u32 gridCountY, u32 gridCellSize)
{
    //NOTE: position is in grid coordinate frame
    f32 X = position.x / (f32)gridCellSize;
    f32 Y = position.y / (f32)gridCellSize;

    i32 cellX = (i32)X;
    i32 cellY = (i32)Y;
    if(cellX<0){cellX = 0;}
    if(cellY<0){cellY = 0;}
    if(cellX>=gridCountX){cellX = gridCountX-1;}
    if(cellY>=gridCountY){cellY = gridCountY-1;}

    f32 fracX = X - cellX;
    f32 fracY = Y - cellY;

    float w00 = (1 - fracX)*(1 - fracY);
    float w10 = fracX*(1 - fracY);
    float w01 = (1 - fracX)*fracY;
    float w11 = fracX*fracY;

    grid[cellX + cellY*(gridCountX+1)] += w00;
    grid[(cellX+1) + cellY*(gridCountX+1)] += w10;
    grid[cellX + (cellY+1)*(gridCountX+1)] += w01;
    grid[(cellX+1) + (cellY+1)*(gridCountX+1)] += w11;
    //u32 grid_index = cellX + cellY * gridCountX;
    //return grid_index;
}

vec2 BilinearSample(vec2* field, i32 width, i32 height,
                     f32 x, f32 y, f32 cell_size)
{
    // 1) Convert world → grid space
    float gx = x / cell_size;
    float gy = y / cell_size;

    int i = (int)gx;
    int j = (int)gy;

    float fx = gx - i;
    float fy = gy - j;

    // 2) Clamp to grid (avoid out-of-bounds)
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    if (i >= width - 1) i = width - 2;
    if (j >= height - 1) j = height - 2;

    // 3) Compute indices
    int idx00 = j * width + i;
    int idx10 = j * width + (i + 1);
    int idx01 = (j + 1) * width + i;
    int idx11 = (j + 1) * width + (i + 1);

    // 4) Fetch values
    vec2 v00 = field[idx00];
    vec2 v10 = field[idx10];
    vec2 v01 = field[idx01];
    vec2 v11 = field[idx11];

    // 5) Compute weights
    float w00 = (1 - fx) * (1 - fy);
    float w10 = fx * (1 - fy);
    float w01 = (1 - fx) * fy;
    float w11 = fx * fy;

    // 6) Interpolate
    vec2 result;
    result.x = w00*v00.x + w10*v10.x + w01*v01.x + w11*v11.x;
    result.y = w00*v00.y + w10*v10.y + w01*v01.y + w11*v11.y;

    return result;
}

//  SECTION  Math

vec2 Normalize(vec2 v)
{
    f32 eps = 0.001f;
    f32 norm = sqrt(v.x*v.x + v.y*v.y);
    if(norm < eps){ return {1,0}; }
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
    constexpr u32 gridCellSize = (1<<3);
    constexpr u32 gridCountX = (u32)800/gridCellSize;
    constexpr u32 gridCountY = (u32)800/gridCellSize;
    constexpr u32 gridCellCount  = gridCountX * gridCountY;
    constexpr u32 gridPointCount  = (gridCountX+1) * (gridCountY+1);
    f32 padding            = 100.0f;
    vec2 gridCornerTopLeft      = {padding,padding};
    vec2 gridCornerBottomRight  = {padding + (f32)(gridCountX*gridCellSize),padding + (f32)(gridCountY*gridCellSize)};
    f32 gridWidth  = gridCornerBottomRight.x - gridCornerTopLeft.x;
    f32 gridHeight = gridCornerBottomRight.y - gridCornerTopLeft.y;

    f32  scalarField[gridPointCount];
    vec2 gradientField[gridPointCount];


    // INFO  Initialization of memory
    ASSERT(sizeof(app_state) <= appMemory->permanentStorageSize);
    app_state* appState = (app_state*)appMemory->permanentStorage;
    if(!appMemory->isInitialized)
    {

        appState->pos = {0,0};

        for (i32 grid_index = 0; grid_index < gridPointCount; ++grid_index)
        {
            scalarField[grid_index] = 0.0f;
            gradientField[grid_index] = {0.0f,0.0f};
        }
        for(i32 i=0; i<NumSoldiers ; ++i)
        {
            appState->soldiers.position[i]     = Random_vec2(gridCornerTopLeft, gridCornerBottomRight); //{150,220};//
            appState->soldiers.velocity[i]     = Random_vec2(-5.0f,5.0f);
            appState->soldiers.acceleration[i] = Random_vec2(-5.0f,5.0f);;
            appState->soldiers.heading[i]      = Random_f32(0.0f,3.0f);
        }
        // appState->soldiers.position[0]    = gridCornerTopLeft + vec2({40,60});
        // appState->soldiers.velocity[0]     = {0,0};
        // appState->soldiers.acceleration[0]     = {0,0};
        appMemory->isInitialized = TRUE;
    }



    // INFO  Gameplay Update and Render to buffer
    if (appInputs->D.isPressed) { appState->soldiers.position[0].x += 1; }
    if (appInputs->Q.isPressed) { appState->soldiers.position[0].x -= 1; }
    if (appInputs->Z.isPressed) { appState->soldiers.position[0].y -= 1; }
    if (appInputs->S.isPressed) { appState->soldiers.position[0].y += 1; }
    if(appInputs->mouseInputs.isPressed) {appState->pos = appInputs->mouseInputs.mousePosition;}


    // SECTION  UPDATE

    // clearing scalar field & building it
    for (i32 grid_index = 0; grid_index < gridPointCount; ++grid_index)
        scalarField[grid_index] = 0.0f;

    for(int i=0; i<NumSoldiers ; ++i)
    {
        if(appState->soldiers.position[i].x > 0 && appState->soldiers.position[i].y>0 &&
        appState->soldiers.position[i].x < WIDTH && appState->soldiers.position[i].y < HEIGHT) 
        {
            // Convert to grid Reference frame
            vec2 position_Rg = appState->soldiers.position[i] - gridCornerTopLeft;
            BuildDensityField(scalarField, position_Rg, gridCountX, gridCountY, gridCellSize);
        }
    }
    
    // Update gradient field 
        // Update inner grid (with borders EXCLUDED)
        for (i32 grid_indexY = 1; grid_indexY < gridCountY; ++grid_indexY)
        {
            for (i32 grid_indexX = 1; grid_indexX < gridCountX; ++grid_indexX)
            {           
                f32 gradient_x = scalarField[(grid_indexX+1) + grid_indexY*(gridCountX+1)] - scalarField[(grid_indexX-1) + grid_indexY*(gridCountX+1)] ;
                f32 gradient_y = scalarField[(grid_indexX) + (grid_indexY+1)*(gridCountX+1)] - scalarField[(grid_indexX) + (grid_indexY-1)*(gridCountX+1)] ;
                vec2 gradient = {gradient_x, gradient_y};

                //vec2 gradientAvoidance = {-density_x /2.0f,-density_y/2.0f};
                //ASSERT(grid_index < gridCellCount && grid_index >=0)
                gradientField[grid_indexX + grid_indexY*(gridCountX+1)] = gradient;
            }
        }
        // Update borders (with corner EXCLUDED)
        for (i32 grid_indexY = 1; grid_indexY < gridCountY; ++grid_indexY)
        {   
                i32 grid_indexX = 0;
                f32 gradient_x = scalarField[(grid_indexX+1) + grid_indexY*(gridCountX+1)] - scalarField[(grid_indexX) + grid_indexY*(gridCountX+1)] ;
                f32 gradient_y = scalarField[(grid_indexX) + (grid_indexY+1)*(gridCountX+1)] - scalarField[(grid_indexX) + (grid_indexY-1)*(gridCountX+1)] ;
                vec2 gradient = {gradient_x, gradient_y};
                gradientField[grid_indexX + grid_indexY*(gridCountX+1)] = gradient;
        }
        for (i32 grid_indexY = 1; grid_indexY < gridCountY; ++grid_indexY)
        {   
                i32 grid_indexX = gridCountX;
                f32 gradient_x = scalarField[(grid_indexX) + grid_indexY*(gridCountX+1)] - scalarField[(grid_indexX-1) + grid_indexY*(gridCountX+1)] ;
                f32 gradient_y = scalarField[(grid_indexX) + (grid_indexY+1)*(gridCountX+1)] - scalarField[(grid_indexX) + (grid_indexY-1)*(gridCountX+1)] ;
                vec2 gradient = {gradient_x, gradient_y};
                gradientField[grid_indexX + grid_indexY*(gridCountX+1)] = gradient;
        }
        for (i32 grid_indexX = 1; grid_indexX < gridCountX; ++grid_indexX)
        {   
                i32 grid_indexY = 0;
                f32 gradient_x = scalarField[(grid_indexX+1) + grid_indexY*(gridCountX+1)] - scalarField[(grid_indexX-1) + grid_indexY*(gridCountX+1)] ;
                f32 gradient_y = scalarField[(grid_indexX) + (grid_indexY+1)*(gridCountX+1)] - scalarField[(grid_indexX) + (grid_indexY)*(gridCountX+1)] ;
                vec2 gradient = {gradient_x, gradient_y};
                gradientField[grid_indexX + grid_indexY*(gridCountX+1)] = gradient;
        }
        for (i32 grid_indexX = 1; grid_indexX < gridCountX; ++grid_indexX)
        {   
                i32 grid_indexY = gridCountY;
                f32 gradient_x = scalarField[(grid_indexX+1) + grid_indexY*(gridCountX+1)] - scalarField[(grid_indexX-1) + grid_indexY*(gridCountX+1)] ;
                f32 gradient_y = scalarField[(grid_indexX) + (grid_indexY)*(gridCountX+1)] - scalarField[(grid_indexX) + (grid_indexY-1)*(gridCountX+1)] ;
                vec2 gradient = {gradient_x, gradient_y};
                gradientField[grid_indexX + grid_indexY*(gridCountX+1)] = gradient;
        }
        gradientField[0]          = {scalarField[1]-scalarField[0], scalarField[gridCountX+1]-scalarField[0]};
        gradientField[gridCountX] = {scalarField[1]-scalarField[0], scalarField[gridCountX+1]-scalarField[0]};
        gradientField[(gridPointCount-1) - gridCountX] = {scalarField[1]-scalarField[0], scalarField[gridCountX+1]-scalarField[0]};
        gradientField[gridPointCount-1] = {scalarField[1]-scalarField[0], scalarField[gridCountX+1]-scalarField[0]};

    // Update soldiers dynamics
    // do not update solider 0
    for(int i=0; i<NumSoldiers ; ++i)
    {
        // Boids "bounce" over edges from padding
        f32 margin = 0.05f;
        if(appState->soldiers.position[i].x <= (gridCornerTopLeft.x+margin) || appState->soldiers.position[i].x >= (gridCornerBottomRight.x-margin))
        {
            appState->soldiers.velocity[i] = {-appState->soldiers.velocity[i].x,appState->soldiers.velocity[i].y};
        }
        if(appState->soldiers.position[i].y <= (gridCornerTopLeft.y+margin) || appState->soldiers.position[i].y >= (gridCornerBottomRight.y-margin))
        {
            appState->soldiers.velocity[i] = {appState->soldiers.velocity[i].x,-appState->soldiers.velocity[i].y};
        }

        // if(appState->soldiers.position[i].x < 0 || appState->soldiers.position[i].y < 0)
        //     continue;

        vec2 position_Rg = appState->soldiers.position[i] - gridCornerTopLeft;
        // ASSERT(grid_index < gridCellCount)
        appState->soldiers.acceleration[i] = -10.0f*BilinearSample(gradientField, gridCountX+1, gridCountY+1, position_Rg.x, position_Rg.y, (f32)gridCellSize);
        vec2 newVelocity = appState->soldiers.velocity[i] + 0.1f * appState->soldiers.acceleration[i];
        f32 norm = sqrt(newVelocity.x*newVelocity.x + newVelocity.y*newVelocity.y);
        if(norm>5.0f && norm > 0.0f)
        {
            newVelocity = (4.0/norm) * newVelocity;
        }
        appState->soldiers.velocity[i] = newVelocity;
        appState->soldiers.position[i] = appState->soldiers.position[i] + 0.1f * appState->soldiers.velocity[i];
        
    }

    // SECTION  RENDERING

    RenderToBuffer(appBackbuffer); // Clear to color TODO: update this
    DrawGrid(appBackbuffer, gridCornerTopLeft, {gridWidth,gridHeight}, gridCellSize); // Draw some grid

    DrawRectangleFilled(appBackbuffer,appState->pos.x,appState->pos.y,4,4);

    for(int i=0; i<NumSoldiers ; ++i)
    {
        vec2 position = appState->soldiers.position[i];
        //DrawRectangleFilled(appBackbuffer,appState->soldiers.position[i].x,appState->soldiers.position[i].y,8.0f,8.0f);
        vec2 unitVelocity = Normalize(appState->soldiers.velocity[i]);
        vec2 A = position + 2.0f*vec2({+unitVelocity.y, -unitVelocity.x}); 
        vec2 B = position + 2.0f*vec2({-unitVelocity.y, +unitVelocity.x}); 
        vec2 C = position + 6.0f*unitVelocity;

        if(i==10)
            DrawTriangle(appBackbuffer,A,B,C,0.0f,0xff0000);
        else
            DrawTriangle(appBackbuffer,A,B,C);
        #if 0
        DrawVector(appBackbuffer, position, appState->soldiers.velocity[i], 0xffff00, 2.0f); // Draw velocity vector
        DrawVector(appBackbuffer, position + 2.0f*appState->soldiers.velocity[i], appState->soldiers.acceleration[i], 0xff00ff, 10.0f); // Draw acceleration vector (at velocity tip)
        #endif
    }

#if 0
    // render a rectangle for density in each cell
    for(i32 grid_index = 0; grid_index < gridPointCount;++grid_index)
    {
        vec2 position = {(f32)(grid_index % ((i32)gridCountX+1))*(f32)gridCellSize,(f32)(grid_index / ((i32)gridCountY+1))*(f32)gridCellSize};
        position = gridCornerTopLeft + position; // Convert to R_I
        f32 radius = scalarField[grid_index]*10.0f;
        DrawRectangleFilled(appBackbuffer,position.x,position.y,radius,radius, 0xff0055);
    }
#endif
#if 0
    // render gradient field
    for(i32 grid_index = 0; grid_index < gridPointCount;++grid_index)
    {
        vec2 position = {(f32)(grid_index % ((i32)gridCountX+1))*(f32)gridCellSize,(f32)(grid_index / ((i32)gridCountY+1))*(f32)gridCellSize};
        position = gridCornerTopLeft + position; // Convert to R_I
        DrawVector(appBackbuffer, position, -2.0f*gradientField[grid_index], 0xffff00, 10.0f); // Draw gradient
    }
#endif 

    // Tests
    DrawLine(appBackbuffer,{50,50}, appState->pos);
    DrawRectangle(appBackbuffer, gridCornerTopLeft.x, gridCornerTopLeft.y, gridWidth, gridHeight); 

}



// SECTION  Implementation of renderer (imcomplete)
void DrawGrid(app_backbuffer* buffer, vec2 topLeft, vec2 dim, u32 gridCellDimension)
{
    u32 gridColor = 0x666666;
    for (int Y = topLeft.y; Y < topLeft.y + dim.y; Y= Y + gridCellDimension)
    {
        DrawLine(buffer,{topLeft.x,(f32)Y}, {topLeft.x + dim.x, (f32)Y},gridColor);
    }
    for(int X = topLeft.x;  X < topLeft.x + dim.x; X = X + gridCellDimension)
    {
        DrawLine(buffer,{(f32)X,topLeft.y}, {(f32)X, topLeft.y + dim.y}, gridColor);
    }
}