#include <stdlib.h>
#include <math.h>

#define COLOR_RGB(r,g,b) ((u64)(((u8)(b)|((u16)((u8)(g))<<8))|(((u64)(u8)(r))<<16)))
#define NumSoldiers 60
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
void BuildDensityGrid(vec2* field)
{

}

//  SECTION  Math
f32 Norm(vec2 v)
{
    f32 norm = sqrt(v.x*v.x + v.y*v.y);
    return norm;
}
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
f32 max(f32 a, f32 b)
{
    f32 result;
    result = (fabsf(a) < fabsf(b)) ? b : a;
    return result;
}
f32 sign(f32 x)
{
    return ((x > 0) - (x < 0));
}

//  SECTION  DEBUG Immediate UI 
b32 DEBUG_Button(app_state* state, app_inputs* appInputs, app_backbuffer* buffer, f32 x, f32 y, f32 width, f32 height, u32 color = 0x00ff00)
{
    b32 result = false;
    
    b32 hovered = (state->pos.x > x && state->pos.x < x + width) && (state->pos.y > y && state->pos.y < y + height);
    if(appInputs->mouseInputs.isPressed && hovered)
    {
        result = true;
        DrawRectangle(buffer,x,y,width,height,0xff0000);
    }
    else
    {
        DrawRectangle(buffer,x,y,width,height,color);
    }
    
    return result;
}

//  SECTION  Boids

//  SECTION  Main loop
void ApplicationUpdateAndRender(app_memory* appMemory, app_backbuffer* appBackbuffer, app_inputs* appInputs)
{

    // Grid info
    constexpr u32 gridCellSize = (1<<4);
    constexpr u32 gridCountX = (u32)600/gridCellSize;
    constexpr u32 gridCountY = (u32)600/gridCellSize;
    constexpr u32 gridCellCount  = gridCountX * gridCountY;
    f32 padding              = 100.0f;
    vec2 gridCornerTopLeft      = {padding,padding/2};
    vec2 gridCornerBottomRight  = {gridCornerTopLeft.x + (f32)(gridCountX*gridCellSize),gridCornerTopLeft.y + (f32)(gridCountY*gridCellSize)};
    f32 gridWidth  = gridCornerBottomRight.x - gridCornerTopLeft.x;
    f32 gridHeight = gridCornerBottomRight.y - gridCornerTopLeft.y;
    vec2 gridDimension = {gridWidth, gridHeight};

    
    f32  scalarField[gridCellCount];
    vec2 gradientField[gridCellCount];

    // INFO  Initialization of memory
    ASSERT(sizeof(app_state) <= appMemory->permanentStorageSize);
    app_state* appState = (app_state*)appMemory->permanentStorage;
    if(!appMemory->isInitialized)
    {

        appState->selectedSoldierIndex = 0;
        appState->pos = vec2({260.0f,164.0f});


        for(i32 i=0; i<NumSoldiers ; ++i)
        {
            appState->soldiers.position[i]     = Random_vec2({0.1,0.1}, gridDimension - vec2{0.1,0.1}); //{150,220};//
            appState->soldiers.velocity[i]     = Random_vec2(-50.0f,50.0f);
            appState->soldiers.acceleration[i] = Random_vec2(0.0f,0.0f);;
            appState->soldiers.heading[i]      = Random_f32(0.0f,3.0f);
        }

        // appState->soldiers.position[0]    = vec2({gridCellSize*2 + gridCellSize/2, gridCellSize*1 + gridCellSize/2});
        // appState->soldiers.velocity[0]     = {0,-0.5f};
        // appState->soldiers.acceleration[0]     = {0,0};

        // appState->soldiers.position[1]    = vec2({gridCellSize*2 + gridCellSize/2, gridCellSize*3+ gridCellSize/2});
        // appState->soldiers.velocity[1]     = {0,1.0f};
        // appState->soldiers.acceleration[1]     = {0,0};

        appState->showScalarField   = false;
        appState->showGradientField = false;

        
        appMemory->isInitialized = TRUE;

    }

    
    // INFO  Gameplay Update and Render to buffer
    u32 selectedSoldierIndex = appState->selectedSoldierIndex;
    if (appInputs->D.isPressed) 
    { 
        appState->selectedSoldierIndex -= 1; 
        if(appState->selectedSoldierIndex < 0) {appState->selectedSoldierIndex = NumSoldiers-1;}
    }
    if (appInputs->Q.isPressed) { appState->selectedSoldierIndex = (appState->selectedSoldierIndex+1)%NumSoldiers; }

    if(appInputs->mouseInputs.isPressed) {appState->pos = appInputs->mouseInputs.mousePosition - gridCornerTopLeft;}

    // For debug
    vec2 DEBUGavoidanceF = {};
    vec2 DEBUGalignmentF = {};
    vec2 DEBUGcohesionF  = {};
    // init field
    for (i32 grid_index = 0; grid_index < gridCellCount; ++grid_index)
    {
        scalarField[grid_index] = 1.0f;
        gradientField[grid_index] = {1.0f,1.0f};
    }

    vec2 targetMousePosition = appState->pos;
    // SECTION  UPDATE
    f32 coherenceRadius = 16.0f;
    f32 avoidanceRadius = 8.0f;
    
    f32 formationOffset = 14.0f;
    f32 formationWidth  = (f32)((30 + 1)*(formationOffset));
    f32 formationHeight = (f32)((6 + 1)*(formationOffset));
    u32 leaderIndex = 0;

    //DEBUG: 
    i32 bucketX = 0;
    i32 bucketY = 0;

    // Loop soldiers once
    for(i32 index_unit = 0 ; index_unit < NumSoldiers; ++index_unit)
    {
        vec2 avoidanceForce = {0,0};
        vec2 cohesionForce  = {0,0};
        vec2 alignmentForce = {0,0};
        vec2 steeringForce  = {0,0};

        vec2 averagePosition = {0,0};
        vec2 averageVelocity = {0,0};

        vec2 position = appState->soldiers.position[index_unit];
        vec2 velocity = appState->soldiers.velocity[index_unit];
        vec2 targetPosition = {0,0};

        // Goal
        if(index_unit == leaderIndex) 
        {targetPosition = targetMousePosition;}
        else
        {targetPosition = appState->soldiers.position[leaderIndex] + vec2{formationOffset, 0};}
        
        u32 numberOfNeighbour = 0.0f;

        f32 closestNeighbourToTheLeft = 1000.0f;
        f32 closestNeighbourToTheTop  = 1000.0f;

        f32 speed = Norm(appState->soldiers.velocity[index_unit]); 

        for(i32 index_neighbour = 0 ; index_neighbour < NumSoldiers; ++index_neighbour)
        {
            if(index_neighbour == index_unit) { continue; }

            vec2 relative_pos = position - appState->soldiers.position[index_neighbour];
            f32 dist = sqrt(relative_pos.x*relative_pos.x + relative_pos.y*relative_pos.y);
            f32 dist_squared = relative_pos.x*relative_pos.x + relative_pos.y*relative_pos.y;

            if(dist < (coherenceRadius))
            {
                numberOfNeighbour++;

                // Avoidance
                if(dist < avoidanceRadius)
                {
                    //relative_pos = Normalize(relative_pos);
                    avoidanceForce = avoidanceForce + (speed*(100.0f/dist)*Normalize(relative_pos));
                }

                // Cohesion
                averagePosition = averagePosition + appState->soldiers.position[index_neighbour];

                // Alignment
                averageVelocity = averageVelocity + appState->soldiers.velocity[index_neighbour];

                // Formation ?
                b32 horizontalQuadrant = fabs(relative_pos.y) < fabs(relative_pos.x) && relative_pos.x>0;
                b32 verticalQuadrant   = fabs(relative_pos.y) > fabs(relative_pos.x) && relative_pos.y>0;
                if(numberOfNeighbour>0 && index_unit != leaderIndex)
                {
                    if(horizontalQuadrant)
                    {
                        if((fabs(relative_pos.x) < closestNeighbourToTheLeft))
                        {
                            closestNeighbourToTheLeft = relative_pos.x;
                            targetPosition.x = appState->soldiers.position[index_neighbour].x + sign(relative_pos.x)*formationOffset;

                        }
                    }  
                    if(verticalQuadrant)
                    {
                        if((fabs(relative_pos.y) < closestNeighbourToTheTop))
                        {
                            closestNeighbourToTheTop = relative_pos.y;
                            targetPosition.y = appState->soldiers.position[index_neighbour].y + sign(relative_pos.y)*formationOffset;
                        }     
                    }
                }

            } 
        } // End of neighbour loop

        // Check formation bounds ? Not good when moving (if copy of leader velocity is disabled)

        if(index_unit != leaderIndex)
        {
            if(targetPosition.x > appState->soldiers.position[leaderIndex].x + formationWidth)
            {
                targetPosition.x = appState->soldiers.position[leaderIndex].x + formationOffset;
            }
            if(targetPosition.x < appState->soldiers.position[leaderIndex].x)
            {
                targetPosition.x = appState->soldiers.position[leaderIndex].x + formationOffset; // offset de 10 vers Y maybe
            }

            if(targetPosition.y > appState->soldiers.position[leaderIndex].y + formationHeight)
            {
                targetPosition.y = appState->soldiers.position[leaderIndex].y; // offset de 10 vers Y maybe
            }

            if(targetPosition.y < appState->soldiers.position[leaderIndex].y)
            {
                targetPosition.y = appState->soldiers.position[leaderIndex].y + formationOffset; // offset de 10 vers Y maybe
            }
        }
        
        if(numberOfNeighbour>0)
        {
            // Cohesion
            averagePosition = averagePosition / numberOfNeighbour;
            vec2 cohesionVector = Normalize(averagePosition - position);
            cohesionForce = 0.0f * cohesionVector;

            // Alignment
            averageVelocity = averageVelocity / numberOfNeighbour;
            vec2 alignmentVector = Normalize(averageVelocity - velocity);
            alignmentForce = 0.0f * alignmentVector;
        }
        
        // Steering (v_des - v_unit)
        vec2 desiredDirection;
        desiredDirection = targetPosition - position;

        vec2 desiredVelocity;
        desiredVelocity = 1.0f*desiredDirection;
        if (index_unit != leaderIndex)
        {desiredDirection = desiredVelocity + 0.0f*appState->soldiers.velocity[leaderIndex];}

        //desiredVelocity = leaderPosition - appState->soldiers.position[index_unit];
        if(Norm(desiredDirection) >= 1.0f) // the radius of a unit ? 8.0/2.0 = 4.0
        {
            steeringForce = 4.0f*desiredDirection - velocity;
        }
        else
        {
            steeringForce = desiredDirection - velocity;
        }
        //steeringForce = 50.0f*Normalize(desiredVelocity) - appState->soldiers.velocity[index_unit];

        // Avoidance
        avoidanceForce =  {avoidanceForce.x, avoidanceForce.y};

        //DEBUG:
        if(index_unit==selectedSoldierIndex)
        {
            DEBUGalignmentF = alignmentForce;
            DEBUGcohesionF  = cohesionForce;
            DEBUGavoidanceF = avoidanceForce;

            bucketX = (i32)(position.x/(f32)gridCellSize);
            bucketY = (i32)(position.y/(f32)gridCellSize);
        }

        // vec2 newForce = 5.0f*(avoidanceForce + cohesionForce + alignmentForce ) + 1.0f*steeringForce;
        // if (Norm(newForce) > 50.0f)
        // {
        //     newForce = 50.0f * Normalize(newForce);
        // }
        appState->soldiers.acceleration[index_unit] = (1.0f*avoidanceForce + 1.0f*cohesionForce + 1.0f*alignmentForce + 1.0f * 1/(0.1f+Norm(avoidanceForce)) * steeringForce);
        
    }
    // Update soldiers dynamics
    // do not update solider 0
    for(int i=0; i<NumSoldiers ; ++i)
    {
        // Boids "bounce" over edges from padding
        f32 margin = 0.05f;
        if(appState->soldiers.position[i].x <= (margin) || appState->soldiers.position[i].x >= (gridWidth-margin))
        {
            appState->soldiers.velocity[i] = {-appState->soldiers.velocity[i].x,appState->soldiers.velocity[i].y};
        }
        if(appState->soldiers.position[i].y <= (margin) || appState->soldiers.position[i].y >= (gridHeight-margin))
        {
            appState->soldiers.velocity[i] = {appState->soldiers.velocity[i].x,-appState->soldiers.velocity[i].y};
        }

        appState->soldiers.position[i] = appState->soldiers.position[i] + 0.01666f * appState->soldiers.velocity[i];
        appState->soldiers.velocity[i] = appState->soldiers.velocity[i] + 0.01666f * appState->soldiers.acceleration[i];

        if (Norm(appState->soldiers.velocity[i]) > 30.0f)
        {
            appState->soldiers.velocity[i] = 30.0f * Normalize(appState->soldiers.velocity[i]);
        }
        // if (Norm(appState->soldiers.velocity[i]) < 30.0f)
        // {
        //     appState->soldiers.velocity[i] = 30.0f * Normalize(appState->soldiers.velocity[i]);
        // }

    }

    



    // SECTION  RENDERING
    RenderToBuffer(appBackbuffer); // Clear to color TODO: update this
    DrawGrid(appBackbuffer, gridCornerTopLeft, {gridWidth,gridHeight}, gridCellSize); // Draw some grid

    // subsection  DEBUG: GUI
    if (DEBUG_Button(appState, appInputs, appBackbuffer, 50, 100, 50, 20))
        appState->showScalarField   = !appState->showScalarField;
    // if (DEBUG_Button(appState, appInputs, appBackbuffer, 50, 130, 50, 20))
    //     appState->showGradientField = !appState->showGradientField;

    DrawRectangleFilled(appBackbuffer,gridCornerTopLeft.x + appState->pos.x,gridCornerTopLeft.y + appState->pos.y,4,4);

    // subsection  Grid and boids stuff
    // DEBUG: Draw fields
    for (i32 grid_index = 0; grid_index < gridCellCount; ++grid_index)
    {
        vec2 centerOfCell = gridCornerTopLeft + vec2{(f32)(grid_index % (i32)gridCountX)*gridCellSize,(f32)(grid_index / (i32)gridCountX)*gridCellSize};
        vec2 dir = gradientField[grid_index];
        DrawVector(appBackbuffer, centerOfCell + vec2{gridCellSize/2,gridCellSize/2}, dir, 0xff00ff, 5.0f);
    }


    // DEBUG: Formation area
    DrawRectangle(appBackbuffer, gridCornerTopLeft.x + appState->soldiers.position[leaderIndex].x + formationWidth/2, 
                                 gridCornerTopLeft.y + appState->soldiers.position[leaderIndex].y + formationHeight/2, 
                                 formationWidth, 
                                 formationHeight, 
                                 0xff0000); 


    // DEBUG: spatial partitioning bucket
    DrawRectangle(appBackbuffer, gridCornerTopLeft.x + bucketX*(gridCellSize) + gridCellSize/2, 
                                 gridCornerTopLeft.y + bucketY*(gridCellSize) + gridCellSize/2, 
                                 gridCellSize, gridCellSize, 0xFF00FF);   
    
    // DEBUG:
    for(i32 i=0; i<NumSoldiers ; ++i)
    {
        vec2 position =  gridCornerTopLeft + appState->soldiers.position[i];

        #if 1 
        f32 size = 8.0f;
        DrawRectangleFilled(appBackbuffer,position.x,position.y,size,size);
        #elif 0
        vec2 unitVelocity = Normalize(appState->soldiers.velocity[i]);
        vec2 A = position + 4.5f*vec2({+unitVelocity.y, -unitVelocity.x}) - 3.0f*unitVelocity; 
        vec2 B = position + 4.5f*vec2({-unitVelocity.y, +unitVelocity.x}) - 3.0f*unitVelocity; 
        vec2 C = position + 6.0f*unitVelocity;
        #endif


        vec2 goalPosition = gridCornerTopLeft + targetMousePosition;
        DrawRectangle(appBackbuffer, goalPosition.x, goalPosition.y, 4.0f, 4.0f, 0xff0000); 

        if(i==selectedSoldierIndex)
        {
            //DrawTriangle(appBackbuffer,A,B,C,0.0f,0xff0000);
            #if 1
            DrawVector(appBackbuffer, position, appState->soldiers.velocity[i], 0xffff00, 0.5f); // Draw velocity vector
            DrawVector(appBackbuffer, position, appState->soldiers.acceleration[i], 0xff00ff, 3.0f); // Draw acceleration vector (at velocity tip)
            // + 0.5f*appState->soldiers.velocity[i]
            DrawVector(appBackbuffer, position, DEBUGalignmentF, 0xff0000, 20.0f);
            DrawVector(appBackbuffer, position, DEBUGavoidanceF, 0x00ff00, 20.0f);
            DrawVector(appBackbuffer, position, DEBUGcohesionF , 0x0000ff, 20.0f);

            DrawRectangle(appBackbuffer, position.x, position.y, 2*avoidanceRadius, 2*avoidanceRadius, 0xff5500); 
            DrawRectangle(appBackbuffer, position.x, position.y, 2*coherenceRadius, 2*coherenceRadius, 0x00ff00); 

            #endif
        }
    }




    // Tests
    DrawLine(appBackbuffer,{0,0}, gridCornerTopLeft + appState->pos, 0xDDAADD);
    DrawRectangle(appBackbuffer, gridCornerTopLeft.x + gridWidth/2, gridCornerTopLeft.y + gridHeight/2, gridWidth, gridHeight, 0xAAAAAA); // grid border

}



// SECTION  Implementation of renderer (imcomplete)
void DrawGrid(app_backbuffer* buffer, vec2 topLeft, vec2 dim, u32 gridCellDimension)
{
    u32 gridColor = 0x666666;
    for (int Y = topLeft.y; Y < topLeft.y + dim.y; Y = Y + gridCellDimension)
    {
        DrawLine(buffer,{topLeft.x,(f32)Y}, {topLeft.x + dim.x, (f32)Y},gridColor);
    }
    for(int X = topLeft.x;  X < topLeft.x + dim.x; X = X + gridCellDimension)
    {
        DrawLine(buffer,{(f32)X,topLeft.y}, {(f32)X, topLeft.y + dim.y}, gridColor);
    }
}