// File: Common.h
#ifndef COMMON_H
#define COMMON_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

template <typename T>
std::string to_str(T value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

inline float randFloat(float min, float max) {
    return min + (float)rand() / (float)RAND_MAX * (max - min);
}

enum GameState { STATE_MENU, STATE_STARTING, STATE_RACING, STATE_FINISHED };

extern GameState currentState;
extern int cameraMode;
extern bool keys[256];

const float DT = 0.016f; 
const float TRACK_WIDTH = 18.0f;
const float MASS = 798.0f;           
const float MAX_ENGINE_FORCE = 15500.0f; 
const float WHEEL_RAD = 0.36f;
const float DRAG_COEFF = 0.98f;      
const float ROLLING_RESIST = 20.0f;  
const float CAR_COLLISION_RADIUS = 2.0f; 

struct Vec3 {
    float x, y, z;
    Vec3(float _x=0, float _y=0, float _z=0) : x(_x), y(_y), z(_z) {}
    Vec3 operator+(const Vec3& v) const { return Vec3(x+v.x, y+v.y, z+v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x-v.x, y-v.y, z-v.z); }
    Vec3 operator*(float s) const { return Vec3(x*s, y*s, z*s); }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalize() const { float l = length(); return (l > 0.0001f) ? *this * (1.0f/l) : Vec3(0,0,0); }
    static float dist(Vec3 a, Vec3 b) { return (a-b).length(); }
};

extern std::vector<Vec3> trackPoints;
extern Vec3 camPos, camLookAt;

#endif
