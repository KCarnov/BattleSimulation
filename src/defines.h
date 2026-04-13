#pragma once

#ifndef UNICODE
#define UNICODE
#endif 

#define WIDTH 1500
#define HEIGHT 800


#define KILOBYTES(Value) ((Value)*1024)
#define MEGABYTES(Value) (KILOBYTES(Value)*1024)
#define GIGABYTES(Value) (MEGABYTES(Value)*1024)

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;

typedef char  i8;
typedef short i16;
typedef int   i32;
typedef long  i64;

typedef float  f32;
typedef double f64;

typedef int  b32;
typedef char b8;

#define TRUE  1
#define FALSE 0

#ifdef DEBUG
#define ASSERT(Expression) if(!(Expression)) {*(int*)0 = 0;}  
#else
#define ASSERT(Expression)
#endif
// INFO  Assertion : Writes something to the null pointer, which is forbidden and will crash/stop the compiler at that spot



struct vec2
{
    f32 x,y;
};

vec2 operator+(vec2 a, vec2 b) { return {a.x + b.x, a.y + b.y}; }
vec2 operator-(vec2 a, vec2 b) { return {a.x - b.x, a.y - b.y}; }
vec2 operator/(vec2 a, f32 n) { return {a.x / n, a.y / n}; }
vec2 operator*(f32 n ,vec2 a) { return {a.x * n, a.y * n}; }
vec2 operator+=(vec2 a,vec2 b) { return {a.x + b.x, a.y + b.y};}
vec2 operator+=(vec2 a,f32 b) { return {a.x + b, a.y + b};}