#include <windows.h> 
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <cstdlib> 
#include <ctime>   

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =============================================================
// FUNGSI BANTUAN UNTUK COMPILER LAMA
// =============================================================
template <typename T>
std::string to_str(T value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

float randFloat(float min, float max) {
    return min + (float)rand() / (float)RAND_MAX * (max - min);
}

// =============================================================
// 1. GLOBAL CONSTANTS & STATES
// =============================================================
enum GameState { STATE_MENU, STATE_STARTING, STATE_RACING, STATE_FINISHED };
GameState currentState = STATE_MENU;

const float DT = 0.016f; 
const float TRACK_WIDTH = 18.0f;
const float MASS = 798.0f;           
const float MAX_ENGINE_FORCE = 15500.0f; 
const float WHEEL_RAD = 0.36f;
const float DRAG_COEFF = 0.98f;      
const float ROLLING_RESIST = 20.0f;  
const float CAR_COLLISION_RADIUS = 2.0f; 

/* 0: Back (TPP) - Default
   1: First Person (Bumper)
   2: Left Side
   3: Right Side
   4: Frontal */
int cameraMode = 0; 

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

// =============================================================
// 1.5. SCENERY SYSTEM 
// =============================================================
enum SceneryType { 
    TYPE_TREE_PINE, TYPE_TREE_BIRCH, TYPE_TREE_APPLE, 
    TYPE_BUILDING, TYPE_BILLBOARD, TYPE_CROWD, TYPE_GRANDSTAND, TYPE_CLOUD,
    TYPE_TENT, TYPE_PLANE, TYPE_HELI 
};

struct SceneryObject {
    float x, z;
    float scaleX, scaleY;
    float rot;
    float r, g, b;
    SceneryType type;
    float colRadius; 
};

std::vector<SceneryObject> sceneryList;

// =============================================================
// 2. PHYSICS SYSTEM (DENGAN HITBOX COLLISION)
// =============================================================
class PhysicsSystem {
public:
    Vec3 pos, fwd;
    float angle, steerAngle, speedMS, rpm, wheelRot, startTimer;
    float throttleSmooth, currentFOV;
    int currentGear, startLights;
    float lightTimer;
    bool isOffTrack;

    int lapCount, maxLap;
    float lapTimer, totalTimer, bestLapTime;
    bool crossedStartLine;

    float gearMaxSpeed[8] = { 35.0f, 0.0f, 90.0f, 140.0f, 190.0f, 240.0f, 290.0f, 340.0f };
    float gearRatios[8]   = { -2.8f, 0.0f, 4.0f, 3.2f, 2.6f, 2.1f, 1.7f, 1.3f };

    PhysicsSystem() { maxLap = 3; reset(); }

    void reset() {
        pos = Vec3(0, WHEEL_RAD, 0); angle = 0; steerAngle = 0; speedMS = 0;
        rpm = 1000; wheelRot = 0; startTimer = 0; throttleSmooth = 0;
        startLights = 0; lightTimer = 0; currentGear = 1; isOffTrack = false;
        currentFOV = 62.0f; lapCount = 0; lapTimer = 0; totalTimer = 0; 
        bestLapTime = 0; crossedStartLine = true;
    }

    void alignToTrack(const std::vector<Vec3>& track) {
        if(track.size() < 2) return;
        Vec3 dir = (track[1] - track[0]).normalize();
        angle = atan2(dir.x, dir.z);
        pos = track[0] - dir * 15.0f + Vec3(0, WHEEL_RAD, 0);
    }

    void checkCollisions() {
        for (size_t i = 0; i < sceneryList.size(); i++) {
            SceneryObject& obj = sceneryList[i];
            if (obj.colRadius <= 0.0f) continue;

            float dx = pos.x - obj.x;
            float dz = pos.z - obj.z;
            float distSq = (dx * dx) + (dz * dz);
            
            float minDist = CAR_COLLISION_RADIUS + obj.colRadius;
            if (distSq < minDist * minDist) {
                float dist = std::sqrt(distSq);
                float overlap = minDist - dist;
                
                if (dist > 0.0001f) {
                    pos.x += (dx / dist) * overlap;
                    pos.z += (dz / dist) * overlap;
                }
                speedMS = -(speedMS * 0.4f); 
                rpm = 1000.0f; 
            }
        }
    }

    void update(float dt, bool keys[], const std::vector<Vec3>& track) {
        if (currentState == STATE_MENU || currentState == STATE_FINISHED) return;
        if (currentState == STATE_STARTING) {
            startTimer += dt; lightTimer += dt;
            if (lightTimer > 0.6f && startLights < 5) { startLights++; lightTimer = 0; }
            if (startTimer > 4.2f) currentState = STATE_RACING;
            return;
        }

        lapTimer += dt; totalTimer += dt;
        float distToStart = Vec3::dist(pos, track[0]);
        if (distToStart < 15.0f && !crossedStartLine) {
            crossedStartLine = true;
            if (lapCount > 0 && (lapTimer < bestLapTime || bestLapTime == 0)) bestLapTime = lapTimer;
            lapCount++; lapTimer = 0;
            if (lapCount > maxLap) currentState = STATE_FINISHED;
        }
        if (distToStart > 40.0f) crossedStartLine = false;

        float throttleInput = (keys['w'] && currentGear > 1) ? 1.0f : (keys['w'] && currentGear == 0 ? -1.0f : 0.0f);
        float brakeInput = keys['s'] ? 1.0f : 0.0f;
        throttleSmooth += (throttleInput - throttleSmooth) * 0.08f;
        
        float rpmNorm = (rpm - 1000) / 12500.0f;
        float torque = std::max(0.3f, 1.0f - (float)pow((rpmNorm - 0.6f), 2) * 2.5f);
        float powerDrop = (rpmNorm > 0.85f) ? (1.0f - (rpmNorm - 0.85f) * 2.5f) : 1.0f;
        float gearFactor = gearRatios[currentGear];
        float engineForce = throttleSmooth * MAX_ENGINE_FORCE * torque * powerDrop * std::abs(gearFactor) * (1.0f - (currentGear * 0.025f));

        float grip = 1.0f + (0.85f * speedMS * speedMS) * 0.00055f;
        float maxT = 11500.0f * grip;
        if (std::abs(engineForce) > maxT) engineForce = maxT * (engineForce > 0 ? 1 : -1);

        float airRes = DRAG_COEFF * speedMS * speedMS * (speedMS >= 0 ? 1 : -1);
        float rollRes = ROLLING_RESIST * speedMS + (std::abs(speedMS) > 0.1f ? (1.0f - std::abs(throttleInput))*40.0f : 0);
        float brakeF = brakeInput * 30000.0f * (speedMS >= 0 ? 1 : -1);

        speedMS += ((engineForce - airRes - rollRes - brakeF) / MASS) * dt;
        float maxSpd = gearMaxSpeed[currentGear] / 3.6f;
        if (std::abs(speedMS) > maxSpd && currentGear != 1) speedMS = std::min(std::abs(speedMS), maxSpd) * (speedMS > 0 ? 1 : -1);

        float steerIn = (keys['a'] ? 1.0f : (keys['d'] ? -1.0f : 0.0f));
        steerAngle += (steerIn * 0.5f * (1.1f / (1.0f + std::abs(speedMS) * 0.045f)) - steerAngle) * 7.5f * dt;
        angle += (speedMS / (5.5f + std::abs(speedMS) * 0.42f)) * steerAngle * dt;

        fwd = Vec3(sin(angle), 0, cos(angle));
        pos = pos + fwd * speedMS * dt;
        
        Vec3 closest; float minDist = 1e9;
        for (size_t i=0; i<track.size(); i++) { 
            float d = Vec3::dist(pos, track[i]); 
            if (d < minDist) { minDist = d; closest = track[i]; } 
        }
        if (Vec3::dist(pos, closest) > TRACK_WIDTH * 0.92f) {
            pos = closest + (pos - closest).normalize() * TRACK_WIDTH * 0.92f;
            speedMS *= 0.97f; isOffTrack = true;
        } else isOffTrack = false;

        // Cek tabrakan dengan pohon / gedung setelah dipaksa stay di batas aspal
        checkCollisions();

        rpm = 1000.0f + (std::abs(speedMS) * 265.0f / (std::abs(gearFactor) + 0.35f));
        if (rpm > 13500) rpm = 13500 - (rand()%150);
        wheelRot += (speedMS / WHEEL_RAD) * dt;
        currentFOV = 62.0f + (std::abs(speedMS) * 0.12f);
    }
};

PhysicsSystem car;
std::vector<Vec3> trackPoints;
Vec3 camPos, camLookAt;
bool keys[256] = { false };

// =============================================================
// 3. RENDER SYSTEM (MOBIL BARU + LINGKUNGAN)
// =============================================================
class RenderSystem {
public:
    static void drawBox(float sx, float sy, float sz) {
        glPushMatrix(); glScalef(sx, sy, sz); glutSolidCube(1.0f); glPopMatrix();
    }

    static void setRed() { glColor3f(0.9f, 0.0f, 0.0f); }
    static void setWhite() { glColor3f(1.0f, 1.0f, 1.0f); }
    static void setBlack() { glColor3f(0.05f, 0.05f, 0.05f); }
    static void setYellow() { glColor3f(1.0f, 0.8f, 0.0f); }

    static void drawWheel(float x, float y, float z, bool isFront) {
        glPushMatrix();
        glTranslatef(x, y, z);
        if (isFront) glRotatef(car.steerAngle * 45.0f, 0, 1, 0);
        glRotatef(car.wheelRot * 180.0f / M_PI, 1, 0, 0);

        setBlack();
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        float thick = isFront ? 0.48f : 0.45f; 
        float rad   = isFront ? 0.40f : 0.55f; 
        glutSolidTorus(thick/2.0f, rad, 15, 15);
        glPopMatrix();

        setYellow();
        glPushMatrix();
        glTranslatef(x > 0 ? 0.02f : -0.02f, 0, 0);
        glRotatef(90, 0, 1, 0);
        glutSolidTorus(0.01f, rad * 0.75f, 10, 10);
        glPopMatrix();

        glColor3f(0.6f, 0.6f, 0.6f);
        glPushMatrix();
        glScalef(0.1f, 1.0f, 1.0f);
        glutSolidSphere(rad * 0.5f, 10, 10);
        glPopMatrix();
        glPopMatrix();
    }

    static void drawSuspension(float x, float z) {
        setBlack();
        glLineWidth(2.0f);
        glBegin(GL_LINES);
            glVertex3f(0.0f, 0.2f, z); glVertex3f(x, 0.0f, z);
            glVertex3f(0.0f, 0.4f, z); glVertex3f(x, 0.0f, z);
        glEnd();
    }

    static void drawCar() {
        glPushMatrix();
        glTranslatef(car.pos.x, car.pos.y, car.pos.z);
        glRotatef(car.angle * 180.0f / M_PI, 0, 1, 0);

        setRed();
        glBegin(GL_QUADS);
            glVertex3f(-0.35f, 0.50f, 0.4f); glVertex3f(0.35f, 0.50f, 0.4f);
            glVertex3f(0.25f, 0.40f, 3.2f);  glVertex3f(-0.25f, 0.40f, 3.2f);
            glVertex3f(-0.35f, 0.10f, 0.4f); glVertex3f(0.35f, 0.10f, 0.4f);
            glVertex3f(0.25f, 0.10f, 3.2f);  glVertex3f(-0.25f, 0.10f, 3.2f);
            glVertex3f(0.35f, 0.50f, 0.4f);  glVertex3f(0.35f, 0.10f, 0.4f);
            glVertex3f(0.25f, 0.10f, 3.2f);  glVertex3f(0.25f, 0.40f, 3.2f);
            glVertex3f(-0.35f, 0.50f, 0.4f); glVertex3f(-0.35f, 0.10f, 0.4f);
            glVertex3f(-0.25f, 0.10f, 3.2f); glVertex3f(-0.25f, 0.40f, 3.2f);
        glEnd();

        setWhite();
        glPushMatrix(); glTranslatef(0.0f, 0.42f, 1.8f); drawBox(0.35f, 0.02f, 1.2f); glPopMatrix();

        setRed();
        glPushMatrix(); glTranslatef(0.0f, 0.35f, -0.4f); drawBox(0.75f, 0.55f, 1.6f); glPopMatrix();

        glBegin(GL_TRIANGLES);
            glVertex3f(-0.30f, 0.60f, -1.0f); glVertex3f(0.30f, 0.60f, -1.0f); glVertex3f(0.0f, 1.0f, -1.0f);
        glEnd();

        glBegin(GL_QUADS);
            setRed();
            glVertex3f(-0.37f, 0.60f, -1.2f); glVertex3f(0.37f, 0.60f, -1.2f);
            glVertex3f(0.05f, 0.50f, -2.2f);  glVertex3f(-0.05f, 0.50f, -2.2f);
        glEnd();

        glPushMatrix();
        glTranslatef(0.7f, 0.30f, -0.6f); drawBox(0.6f, 0.55f, 1.4f);
        glTranslatef(-1.4f, 0.0f, 0.0f); drawBox(0.6f, 0.55f, 1.4f);
        glPopMatrix();

        setRed();
        glPushMatrix(); glTranslatef(0.0f, 0.85f, -2.0f); drawBox(2.0f, 0.1f, 0.7f); 
        setWhite(); glTranslatef(0.0f, 0.06f, 0.0f); drawBox(2.0f, 0.02f, 0.7f); glPopMatrix();

        setRed();
        glPushMatrix(); glTranslatef(1.0f, 0.65f, -2.0f); drawBox(0.05f, 0.6f, 0.8f);
        glTranslatef(-2.0f, 0.0f, 0.0f); drawBox(0.05f, 0.6f, 0.8f); glPopMatrix();

        glColor3f(0.8f, 0.8f, 0.8f);
        glPushMatrix(); glTranslatef(0.0f, 0.12f, 3.2f); drawBox(2.8f, 0.05f, 0.7f); glPopMatrix();

        setWhite();
        glPushMatrix(); glTranslatef(0.0f, 0.62f, -0.35f); glutSolidSphere(0.2f, 12, 12); glPopMatrix();

        drawSuspension(1.0f, 2.5f); drawSuspension(-1.0f, 2.5f);
        drawSuspension(1.0f, -1.3f); drawSuspension(-1.0f, -1.3f);

        drawWheel(0.95f, -0.05f, 2.5f, true); drawWheel(-0.95f, -0.05f, 2.5f, true);
        drawWheel(1.05f, -0.1f, -1.3f, false); drawWheel(-1.05f, -0.1f, -1.3f, false);

        glPopMatrix();
    }

    static void drawPersonAndFlash(float x, float y, float z, float seedVal, float timeMs) {
        glPushMatrix();
        glTranslatef(x, y, z);
        
        int seed = (int)(std::abs(seedVal));
        float r = (seed % 3 == 0) ? 0.9f : ((seed % 3 == 1) ? 0.2f : 0.8f);
        float g = (seed % 4 == 0) ? 0.2f : ((seed % 4 == 1) ? 0.8f : 0.9f);
        float b = (seed % 5 == 0) ? 0.1f : ((seed % 5 == 1) ? 0.9f : 0.2f);
        if(r>0.7f && g>0.7f && b>0.7f) { r=1.0f; g=1.0f; b=1.0f; }

        glColor3f(0.2f, 0.2f, 0.3f);
        glPushMatrix(); glTranslatef(0, 0.4f, 0); glScalef(0.4f, 0.8f, 0.25f); glutSolidCube(1.0); glPopMatrix();

        glColor3f(r, g, b);
        glPushMatrix(); glTranslatef(0, 1.1f, 0); glScalef(0.5f, 0.6f, 0.3f); glutSolidCube(1.0); glPopMatrix();

        glColor3f(0.9f, 0.75f, 0.6f);
        glPushMatrix(); glTranslatef(0, 1.55f, 0); glutSolidSphere(0.2, 6, 6); glPopMatrix();

        float flashTime = timeMs * 0.003f + seed * 10.0f;
        if (std::sin(flashTime) > 0.99f) { 
            glDisable(GL_LIGHTING); 
            glColor3f(1.0f, 1.0f, 1.0f);
            glPushMatrix(); glTranslatef(0.2f, 1.3f, 0.3f); glutSolidSphere(0.4, 8, 8); glPopMatrix();
            glEnable(GL_LIGHTING);
        }

        glPopMatrix();
    }

    static void drawScenery() {
        float timeMs = glutGet(GLUT_ELAPSED_TIME);

        for(size_t i=0; i<sceneryList.size(); i++) {
            SceneryObject& obj = sceneryList[i];

            if (obj.type == TYPE_CLOUD) {
                glDisable(GL_LIGHTING);
                glColor3f(1.0f, 1.0f, 1.0f);
                float renderX = obj.x + timeMs * 0.002f;
                if (renderX > 4000.0f) renderX -= 8000.0f;

                glPushMatrix();
                glTranslatef(renderX, obj.scaleY, obj.z);
                glScalef(obj.scaleX, obj.scaleX * 0.4f, obj.scaleX * 0.8f);
                glutSolidSphere(1.0, 10, 10);
                glTranslatef(0.8f, -0.2f, 0.2f); glutSolidSphere(0.7, 8, 8);
                glTranslatef(-1.6f, 0.1f, -0.4f); glutSolidSphere(0.6, 8, 8);
                glPopMatrix();
                glEnable(GL_LIGHTING);
                continue;
            }

            if (obj.type == TYPE_PLANE || obj.type == TYPE_HELI) {
                glPushMatrix();
                glTranslatef(obj.x, obj.scaleY, obj.z);
                glRotatef(obj.rot, 0, 1, 0); 
                glScalef(obj.scaleX, obj.scaleX, obj.scaleX); 

                if (obj.type == TYPE_PLANE) {
                    glColor3f(0.9f, 0.9f, 0.9f); 
                    glPushMatrix(); glScalef(4.0f, 1.0f, 1.0f); glutSolidSphere(1.0, 10, 10); glPopMatrix();
                    glColor3f(0.8f, 0.1f, 0.1f); 
                    glPushMatrix(); glTranslatef(0.5f, 0, 0); glScalef(2.0f, 0.1f, 6.0f); glutSolidCube(1.0); glPopMatrix();
                    glPushMatrix(); glTranslatef(-3.0f, 0.5f, 0); glScalef(1.0f, 1.5f, 0.1f); glutSolidCube(1.0); glPopMatrix();
                    glPushMatrix(); glTranslatef(-3.0f, 0, 0); glScalef(1.0f, 0.1f, 2.5f); glutSolidCube(1.0); glPopMatrix();
                } else {
                    glColor3f(0.15f, 0.15f, 0.15f); 
                    glPushMatrix(); glScalef(2.0f, 1.5f, 1.5f); glutSolidSphere(1.0, 10, 10); glPopMatrix();
                    glPushMatrix(); glTranslatef(-2.5f, 0, 0); glScalef(3.0f, 0.4f, 0.4f); glutSolidCube(1.0); glPopMatrix();
                    glPushMatrix(); glTranslatef(-4.0f, 0.5f, 0.2f); glScalef(0.1f, 1.0f, 0.1f); glutSolidCube(1.0); glPopMatrix();
                    
                    glColor3f(0.8f, 0.8f, 0.8f);
                    glPushMatrix();
                    glTranslatef(0, 1.6f, 0);
                    glRotatef(timeMs * 1.5f, 0, 1, 0); 
                    glScalef(6.0f, 0.1f, 0.4f);
                    glutSolidCube(1.0);
                    glPopMatrix();
                    
                    glPushMatrix();
                    glTranslatef(-4.0f, 0.5f, 0.3f);
                    glRotatef(timeMs * 2.0f, 0, 0, 1); 
                    glScalef(1.5f, 0.1f, 0.1f);
                    glutSolidCube(1.0);
                    glPopMatrix();
                }
                glPopMatrix();
                continue;
            }

            // Culling Objek Bawah (diperbesar karena tracknya sangat besar)
            float dx = obj.x - camPos.x;
            float dz = obj.z - camPos.z;
            if((dx*dx + dz*dz) > 800000.0f) continue; 

            glPushMatrix();
            glTranslatef(obj.x, 0, obj.z);
            glRotatef(obj.rot, 0, 1, 0);

            if (obj.type == TYPE_TREE_PINE) {
                glColor3f(0.3f, 0.15f, 0.05f);
                glPushMatrix(); glTranslatef(0, obj.scaleY * 0.1f, 0); glScalef(1.0f, obj.scaleY * 0.2f, 1.0f); glutSolidCube(1.0); glPopMatrix();
                glColor3f(0.1f, 0.35f, 0.1f);
                glPushMatrix(); glTranslatef(0, obj.scaleY * 0.2f, 0); glRotatef(-90.0f, 1, 0, 0); 
                glutSolidCone(obj.scaleY * 0.25f, obj.scaleY * 0.5f, 8, 2);
                glTranslatef(0, 0, obj.scaleY * 0.25f); glutSolidCone(obj.scaleY * 0.2f, obj.scaleY * 0.4f, 8, 2); glPopMatrix();
            } else if (obj.type == TYPE_TREE_BIRCH) {
                glColor3f(0.9f, 0.9f, 0.9f);
                glPushMatrix(); glTranslatef(0, obj.scaleY * 0.25f, 0); glScalef(0.6f, obj.scaleY * 0.5f, 0.6f); glutSolidCube(1.0); glPopMatrix();
                glColor3f(0.6f, 0.8f, 0.2f);
                glPushMatrix(); glTranslatef(0, obj.scaleY * 0.7f, 0); glScalef(1.0f, 1.3f, 1.0f); glutSolidSphere(obj.scaleY * 0.35f, 7, 7); glPopMatrix();
            } else if (obj.type == TYPE_TREE_APPLE) {
                glColor3f(0.4f, 0.2f, 0.1f);
                glPushMatrix(); glTranslatef(0, obj.scaleY * 0.2f, 0); glScalef(1.0f, obj.scaleY * 0.4f, 1.0f); glutSolidCube(1.0); glPopMatrix();
                glColor3f(0.15f, 0.5f, 0.15f);
                glPushMatrix(); glTranslatef(0, obj.scaleY * 0.7f, 0); glutSolidSphere(obj.scaleY * 0.4f, 8, 8); 
                glColor3f(0.9f, 0.1f, 0.1f); float r = obj.scaleY * 0.38f; 
                glPushMatrix(); glTranslatef(r, 0, 0); glutSolidSphere(1.2, 5, 5); glPopMatrix();
                glPushMatrix(); glTranslatef(-r, r*0.5f, 0); glutSolidSphere(1.2, 5, 5); glPopMatrix();
                glPushMatrix(); glTranslatef(0, -r*0.3f, r); glutSolidSphere(1.2, 5, 5); glPopMatrix();
                glPushMatrix(); glTranslatef(0, r*0.4f, -r); glutSolidSphere(1.2, 5, 5); glPopMatrix();
                glPopMatrix(); 
            }
            else if (obj.type == TYPE_BUILDING) {
                glPushMatrix(); glTranslatef(0, obj.scaleY * 0.5f, 0); 
                glColor3f(obj.r, obj.g, obj.b);
                glPushMatrix(); glScalef(obj.scaleX, obj.scaleY, obj.scaleX); glutSolidCube(1.0); glPopMatrix();
                glDisable(GL_LIGHTING);
                int numFloors = (int)(obj.scaleY / 10.0f); 
                float w = obj.scaleX * 0.5f + 0.2f; 
                glBegin(GL_QUADS);
                for(int f = 1; f < numFloors; f++) {
                    float y = -obj.scaleY * 0.5f + (f * 10.0f); float h = 2.5f; 
                    if (((int)(obj.x + obj.z + f)) % 5 == 0) glColor3f(0.9f, 0.9f, 0.6f); else glColor3f(0.15f, 0.2f, 0.25f); 
                    glVertex3f(-w, y-h,  w); glVertex3f( w, y-h,  w); glVertex3f( w, y+h,  w); glVertex3f(-w, y+h,  w);
                    glVertex3f( w, y-h, -w); glVertex3f(-w, y-h, -w); glVertex3f(-w, y+h, -w); glVertex3f( w, y+h, -w);
                    glVertex3f( w, y-h,  w); glVertex3f( w, y-h, -w); glVertex3f( w, y+h, -w); glVertex3f( w, y+h,  w);
                    glVertex3f(-w, y-h, -w); glVertex3f(-w, y-h,  w); glVertex3f(-w, y+h,  w); glVertex3f(-w, y+h, -w);
                }
                glEnd(); glEnable(GL_LIGHTING); glPopMatrix();
            } else if (obj.type == TYPE_BILLBOARD) {
                glColor3f(0.2f, 0.2f, 0.2f); glPushMatrix(); glTranslatef(0, 5.0f, 0); glScalef(1.0f, 10.0f, 1.0f); glutSolidCube(1.0); glPopMatrix();
                glColor3f(obj.r, obj.g, obj.b); glPushMatrix(); glTranslatef(0, 12.0f, 0); glScalef(15.0f, 6.0f, 1.0f); glutSolidCube(1.0); glPopMatrix();
            } else if (obj.type == TYPE_TENT) {
                float tW = obj.scaleX * 0.5f, tH = obj.scaleY;        
                glColor3f(0.8f, 0.8f, 0.8f); 
                float corners[4][2] = {{tW, tW}, {-tW, tW}, {tW, -tW}, {-tW, -tW}};
                for(int k=0; k<4; k++) { glPushMatrix(); glTranslatef(corners[k][0], tH * 0.5f, corners[k][1]); glScalef(0.4f, tH, 0.4f); glutSolidCube(1.0); glPopMatrix(); }
                glColor3f(obj.r, obj.g, obj.b); 
                glPushMatrix(); glTranslatef(0, tH, 0); glRotatef(-90.0f, 1, 0, 0); glRotatef(45.0f, 0, 0, 1); glutSolidCone(obj.scaleX * 0.7f, obj.scaleY * 0.5f, 4, 1); glPopMatrix();
                int peopleInTent = 6;
                for(int p = 0; p < peopleInTent; p++) {
                    float px = std::fmod((obj.x * 13.0f + p * 17.0f), tW * 1.5f) - (tW * 0.75f);
                    float pz = std::fmod((obj.z * 11.0f + p * 23.0f), tW * 1.5f) - (tW * 0.75f);
                    drawPersonAndFlash(px, 0.0f, pz, obj.x + p, timeMs);
                }
            } else if (obj.type == TYPE_CROWD) {
                drawPersonAndFlash(0, 0, 0, obj.x + obj.z, timeMs);
            } else if (obj.type == TYPE_GRANDSTAND) {
                int tiers = 5; float tierDepth = 2.0f, tierHeight = 1.5f, standWidth = 26.0f;    
                glColor3f(0.4f, 0.4f, 0.4f);
                glPushMatrix(); glTranslatef(0, (tiers * tierHeight) * 0.5f, -(tiers * tierDepth)); glScalef(standWidth + 1.0f, tiers * tierHeight, 0.5f); glutSolidCube(1.0); glPopMatrix();
                glColor3f(0.7f, 0.7f, 0.7f);
                glPushMatrix(); glTranslatef(0, tiers * tierHeight + 1.5f, -(tiers * tierDepth) * 0.5f); glRotatef(12.0f, 1, 0, 0); glScalef(standWidth + 2.0f, 0.4f, tiers * tierDepth + 5.0f); glutSolidCube(1.0); glPopMatrix();

                for (int t = 0; t < tiers; t++) {
                    glColor3f(obj.r, obj.g, obj.b); 
                    glPushMatrix(); glTranslatef(0, t * tierHeight + 0.5f, -t * tierDepth); glScalef(standWidth, 1.0f, tierDepth); glutSolidCube(1.0); glPopMatrix();
                    int peopleCount = 14; float spacing = standWidth / peopleCount;
                    for(int p = 0; p < peopleCount; p++) {
                        float noiseX = std::fmod(obj.x + t * 3.0f + p * 7.0f, 0.8f) - 0.4f;
                        float noiseZ = std::fmod(obj.z + t * 5.0f + p * 2.0f, 0.8f) - 0.4f;
                        float px = -standWidth * 0.5f + (p + 0.5f) * spacing + noiseX;
                        float pz = -t * tierDepth + noiseZ;
                        drawPersonAndFlash(px, t * tierHeight + 1.0f, pz, obj.x + t + p, timeMs);
                    }
                }
            }
            glPopMatrix();
        }
    }

    static void drawTrack() {
        glDisable(GL_LIGHTING);
        // Grass (Background)
        glColor3f(0.12f, 0.3f, 0.12f); glBegin(GL_QUADS);
        glVertex3f(-6000, 0, -6000); glVertex3f(6000, 0, -6000);
        glVertex3f(6000, 0, 6000); glVertex3f(-6000, 0, 6000); glEnd();

        for(size_t i=0; i<trackPoints.size(); i++) {
            Vec3 p1 = trackPoints[i], p2 = trackPoints[(i+1)%trackPoints.size()];
            Vec3 n = Vec3(-(p2-p1).z, 0, (p2-p1).x).normalize();
            
 // Checkered Finish Line (Pola Papan Catur Hitam Putih)
            if (i == 0 || i == 1) { // Kita pakai 2 segmen agar garis finishnya cukup tebal/lebar
                int cols = 24; // Jumlah kotak dari kiri ke kanan lintasan
                int rows = 3;  // Jumlah kotak dari belakang ke depan
                Vec3 dir = p2 - p1;
                
                glBegin(GL_QUADS);
                for(int r = 0; r < rows; r++) {
                    float f0 = (float)r / rows;
                    float f1 = (float)(r + 1) / rows;
                    
                    Vec3 row_p1_start = p1 + dir * f0;
                    Vec3 row_p1_end   = p1 + dir * f1;

                    for(int c = 0; c < cols; c++) {
                        // Logika matematika untuk warna selang-seling (Papan Catur)
                        if ((r + c + i) % 2 == 0) {
                            glColor3f(1.0f, 1.0f, 1.0f); // Warna Putih Terang
                        } else {
                            glColor3f(0.05f, 0.05f, 0.05f); // Warna Hitam Pekat
                        }
                        
                        float w0 = -1.0f + (c * 2.0f / cols);
                        float w1 = -1.0f + ((c + 1) * 2.0f / cols);

                        // 4 Titik sudut untuk satu buah kotak (Quad)
                        Vec3 c1 = row_p1_start + n * (TRACK_WIDTH * w0);
                        Vec3 c2 = row_p1_start + n * (TRACK_WIDTH * w1);
                        Vec3 c3 = row_p1_end   + n * (TRACK_WIDTH * w1);
                        Vec3 c4 = row_p1_end   + n * (TRACK_WIDTH * w0);

                        glVertex3f(c1.x, 0.026f, c1.z);
                        glVertex3f(c2.x, 0.026f, c2.z);
                        glVertex3f(c3.x, 0.026f, c3.z);
                        glVertex3f(c4.x, 0.026f, c4.z);
                    }
                }
                glEnd();
            } else {
                glColor3f(0.15f, 0.15f, 0.18f); // Asphalt
                glBegin(GL_QUAD_STRIP);
                glVertex3f(p1.x-n.x*TRACK_WIDTH,0.02f,p1.z-n.z*TRACK_WIDTH); glVertex3f(p1.x+n.x*TRACK_WIDTH,0.02f,p1.z+n.z*TRACK_WIDTH);
                glVertex3f(p2.x-n.x*TRACK_WIDTH,0.02f,p2.z-n.z*TRACK_WIDTH); glVertex3f(p2.x+n.x*TRACK_WIDTH,0.02f,p2.z+n.z*TRACK_WIDTH); glEnd();
            }

            // Red & White Kerbs
            if ((i/4)%2==0) glColor3f(1,0,0); else glColor3f(1,1,1);
            glBegin(GL_QUAD_STRIP);
            glVertex3f(p1.x+n.x*TRACK_WIDTH, 0.03f, p1.z+n.z*TRACK_WIDTH); glVertex3f(p1.x+n.x*(TRACK_WIDTH+1.8f), 0.03f, p1.z+n.z*(TRACK_WIDTH+1.8f));
            glVertex3f(p2.x+n.x*TRACK_WIDTH, 0.03f, p2.z+n.z*TRACK_WIDTH); glVertex3f(p2.x+n.x*(TRACK_WIDTH+1.8f), 0.03f, p2.z+n.z*(TRACK_WIDTH+1.8f)); glEnd();
            glBegin(GL_QUAD_STRIP);
            glVertex3f(p1.x-n.x*(TRACK_WIDTH+1.8f), 0.03f, p1.z-n.z*(TRACK_WIDTH+1.8f)); glVertex3f(p1.x-n.x*TRACK_WIDTH, 0.03f, p1.z-n.z*TRACK_WIDTH);
            glVertex3f(p2.x-n.x*(TRACK_WIDTH+1.8f), 0.03f, p2.z-n.z*(TRACK_WIDTH+1.8f)); glVertex3f(p2.x-n.x*TRACK_WIDTH, 0.03f, p2.z-n.z*TRACK_WIDTH); glEnd();
        }
        glEnable(GL_LIGHTING);
    }
};

// =============================================================
// 4. UI SYSTEM (NEW HUD & MINIMAP)
// =============================================================
class UISystem {
public:
    static float totalTime;
    static std::string formatTime(float t) {
        int m = (int)t / 60, s = (int)t % 60, ms = (int)((t - std::floor(t)) * 100);
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << m << ":" << std::setw(2) << s << ":" << std::setw(2) << ms;
        return ss.str();
    }
    static void drawText(int x, int y, std::string s, void* font = GLUT_BITMAP_HELVETICA_18) {
        glRasterPos2i(x, y);
        for(size_t i=0; i<s.length(); i++) glutBitmapCharacter(font, s[i]);
    }
    static void render() {
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 1280, 0, 720);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST); glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (currentState == STATE_MENU) {
            glColor3f(1,1,1); drawText(450, 420, "F1 SIMULATOR ULTIMATE (MULTI-CAM & SCENERY)", GLUT_BITMAP_TIMES_ROMAN_24);
            drawText(560, 380, "PRESS ENTER TO START", GLUT_BITMAP_HELVETICA_12);
        } else if (currentState == STATE_RACING || currentState == STATE_STARTING) {
            // MINIMAP
            float cx = 130, cy = 130, r = 100, sc = 0.06f;
            glColor4f(0, 0, 0, 0.4f); glBegin(GL_TRIANGLE_FAN); for(int i=0; i<32; i++) glVertex2f(cx+std::cos(i*M_PI/16)*r, cy+std::sin(i*M_PI/16)*r); glEnd();
            glColor3f(0.5f, 0.5f, 0.5f); glBegin(GL_LINE_LOOP); for(size_t k=0; k<trackPoints.size(); k++) glVertex2f(cx+trackPoints[k].x*sc, cy+trackPoints[k].z*sc); glEnd();
            glColor3f(0.2, 1, 0.2); glBegin(GL_TRIANGLE_FAN); for(int i=0; i<16; i++) glVertex2f(cx+trackPoints[0].x*sc + std::cos(i*M_PI/8)*4, cy+trackPoints[0].z*sc + std::sin(i*M_PI/8)*4); glEnd();
            glPushMatrix(); glTranslatef(cx+car.pos.x*sc, cy+car.pos.z*sc, 0); glRotatef(-car.angle*180/M_PI,0,0,1);
            glColor3f(1,1,0); glBegin(GL_TRIANGLES); glVertex2f(0,7); glVertex2f(-4,-4); glVertex2f(4,-4); glEnd(); glPopMatrix();
            
            // TIMERS
            glColor4f(0, 0, 0, 0.5f); glRectf(20, 580, 260, 700);
            glColor3f(1,1,1); 
            int currentL = std::min(car.lapCount, car.maxLap);
            drawText(40, 675, "CURRENT LAP: " + to_str(currentL), GLUT_BITMAP_HELVETICA_18);
            drawText(40, 655, "TOTAL LAPS: " + to_str(car.maxLap), GLUT_BITMAP_HELVETICA_12);
            drawText(40, 638, "LAP PROGRESS: " + to_str(currentL) + "/" + to_str(car.maxLap), GLUT_BITMAP_HELVETICA_12);
            drawText(40, 610, "TIME: " + formatTime(car.lapTimer), GLUT_BITMAP_HELVETICA_18);
            drawText(40, 590, "BEST: " + formatTime(car.bestLapTime), GLUT_BITMAP_HELVETICA_12);

            // GEAR & SPEED
            int bX = 1020, bY = 60;
            glColor4f(0, 0, 0, 0.5f); glRectf(bX - 20, bY - 20, bX + 230, bY + 110);
            glColor3f(1,1,1); 
            drawText(bX, bY + 80, to_str((int)(std::abs(car.speedMS)*3.6f)) + " KM/H", GLUT_BITMAP_HELVETICA_18);
            std::string gearChar = (car.currentGear==0?"R":car.currentGear==1?"N":to_str(car.currentGear-1));
            drawText(bX, bY + 50, "GEAR: " + gearChar, GLUT_BITMAP_HELVETICA_12);
            float rpmNorm = (car.rpm - 1000) / 12500.0f;
            if (rpmNorm < 0.7f) glColor3f(0, 0.8f, 0); else if (rpmNorm < 0.9f) glColor3f(1, 0.8f, 0); 
            else { float blink = (int)(totalTime * 15) % 2 == 0 ? 1.0f : 0.4f; glColor3f(1 * blink, 0, 0); }
            glRectf(bX, bY + 20, bX + rpmNorm * 200, bY + 35);
            glColor3f(0.3, 0.3, 0.3); glBegin(GL_LINE_LOOP); glVertex2f(bX, bY+20); glVertex2f(bX+200, bY+20); glVertex2f(bX+200, bY+35); glVertex2f(bX, bY+35); glEnd();
        }

        if (currentState == STATE_STARTING) {
            for (int i=0; i<5; i++) {
                glColor3f(i<car.startLights?1:0.1f, 0, 0);
                glBegin(GL_TRIANGLE_FAN); for(int a=0; a<20; a++) glVertex2f(480+i*80+std::cos(a*M_PI/10)*25, 550+std::sin(a*M_PI/10)*25); glEnd();
            }
        }
        if (currentState == STATE_FINISHED) {
            glColor4f(0,0,0,0.8f); glRectf(0,0,1280,720);
            glColor3f(1,1,1); drawText(540, 450, "RACE FINISHED", GLUT_BITMAP_TIMES_ROMAN_24);
            drawText(540, 400, "BEST: " + formatTime(car.bestLapTime));
            drawText(540, 370, "TOTAL: " + formatTime(car.totalTimer));
drawText(540, 300, "PRESS 'R' TO RESTART", GLUT_BITMAP_HELVETICA_12);
        }
        glDisable(GL_BLEND); glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
        glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
    }
};
float UISystem::totalTime = 0;

// =============================================================
// 5. CAMERA & DISPLAY LOGIC
// =============================================================
void display() {
    glClearColor(0.4f, 0.75f, 1.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(car.currentFOV, (float)1280/720, 0.1f, 8000.0f);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    
    float eyeX, eyeY, eyeZ, centerX, centerY, centerZ;
    static float orbit = 0; orbit += 0.004f;

    if (currentState == STATE_MENU) {
        eyeX = car.pos.x + std::sin(orbit)*45; eyeY = 22; eyeZ = car.pos.z + std::cos(orbit)*45;
        centerX = car.pos.x; centerY = car.pos.y; centerZ = car.pos.z;
    } else if (currentState == STATE_STARTING) {
        float t = std::min(1.0f, car.startTimer / 4.2f);
        Vec3 rPos = car.pos - car.fwd * 11.0f + Vec3(0, 3.8f, 0);
        Vec3 tLook = car.pos + car.fwd * 12.0f;
        Vec3 mPos = car.pos + Vec3(std::sin(orbit)*45, 22, std::cos(orbit)*45);
        camPos = mPos * (1.0f - t) + rPos * t;
        camLookAt = car.pos * (1.0f - t) + tLook * t;
        eyeX = camPos.x; eyeY = camPos.y; eyeZ = camPos.z;
        centerX = camLookAt.x; centerY = camLookAt.y; centerZ = camLookAt.z;
    } else {
        // MULTI-CAMERA SYSTEM
        switch (cameraMode) {
            case 1: // BUMPER / FIRST PERSON
                eyeX = car.pos.x + car.fwd.x * 3.2f; eyeY = 0.9f; eyeZ = car.pos.z + car.fwd.z * 3.2f;
                centerX = eyeX + car.fwd.x * 10.0f; centerY = 0.8f; centerZ = eyeZ + car.fwd.z * 10.0f;
                break;
            case 2: // LEFT SIDE VIEW
                eyeX = car.pos.x - std::sin(car.angle + M_PI/2)*7.0f; eyeY = 1.8f; eyeZ = car.pos.z - std::cos(car.angle + M_PI/2)*7.0f;
                centerX = car.pos.x + car.fwd.x * 3.0f; centerY = 0.5f; centerZ = car.pos.z + car.fwd.z * 3.0f;
                break;
            case 3: // RIGHT SIDE VIEW
                eyeX = car.pos.x + std::sin(car.angle + M_PI/2)*7.0f; eyeY = 1.8f; eyeZ = car.pos.z + std::cos(car.angle + M_PI/2)*7.0f;
                centerX = car.pos.x + car.fwd.x * 3.0f; centerY = 0.5f; centerZ = car.pos.z + car.fwd.z * 3.0f;
                break;
            case 4: // FRONT FACING (TV CAM)
                eyeX = car.pos.x + car.fwd.x * 12.0f; eyeY = 2.0f; eyeZ = car.pos.z + car.fwd.z * 12.0f;
                centerX = car.pos.x; centerY = 0.6f; centerZ = car.pos.z;
                break;
            default: // THIRD PERSON (TPP) - DENGAN SMOOTHING
                Vec3 targetCamPos = car.pos - car.fwd * 11.0f + Vec3(0, 3.8f, 0);
                Vec3 targetLookAt = car.pos + car.fwd * 12.0f;
                camPos = camPos + (targetCamPos - camPos) * 0.12f;
                camLookAt = camLookAt + (targetLookAt - camLookAt) * 0.12f;
                eyeX = camPos.x; eyeY = camPos.y; eyeZ = camPos.z;
                centerX = camLookAt.x; centerY = camLookAt.y; centerZ = camLookAt.z;
                break;
        }
    }
    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, 0, 1, 0);

    // Pencahayaan mengikuti mobil
    GLfloat lp[] = { car.pos.x + 100, 400, car.pos.z + 100, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, lp);

    RenderSystem::drawTrack();
    RenderSystem::drawScenery();
    RenderSystem::drawCar();
    UISystem::render();

    glutSwapBuffers();
}

void timer(int v) { 
    car.update(DT, keys, trackPoints); 
    UISystem::totalTime += DT;
    glutPostRedisplay(); 
    glutTimerFunc(16, timer, 0); 
}

// =============================================================
// 6. INPUT HANDLER
// =============================================================
void keyboard(unsigned char k, int x, int y) {
    keys[std::tolower(k)] = true;
    
    // ENTER untuk mulai game
    if(k == 13 && currentState == STATE_MENU) currentState = STATE_STARTING;
    
    // Q dan E untuk Ganti Gigi
    if(std::tolower(k) == 'e' && car.currentGear < 7) car.currentGear++;
    if(std::tolower(k) == 'q' && car.currentGear > 0) car.currentGear--;
    
    // --- TAMBAHKAN TOMBOL KAMERA DI SINI ---
    if(std::tolower(k) == 'f') cameraMode = 1; // Tombol F: Kamera Bumper (First Person)
    if(std::tolower(k) == 'v') cameraMode = 0; // Tombol V: Kembali ke Default (Third Person)
    // ---------------------------------------

    // R untuk Restart jika sudah finish
    if(currentState == STATE_FINISHED && std::tolower(k) == 'r') {
        car.reset(); 
        car.alignToTrack(trackPoints); 
        currentState = STATE_MENU;
    }
}
void keyboardUp(unsigned char k, int x, int y) { keys[std::tolower(k)] = false; }

void specialKeyDown(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    cameraMode = 4; break;
        case GLUT_KEY_DOWN:  cameraMode = 0; break;
        case GLUT_KEY_LEFT:  cameraMode = 2; break;
        case GLUT_KEY_RIGHT: cameraMode = 3; break;
    }
}

// =============================================================
// 7. INITIALIZATION (WORLD GENERATOR)
// =============================================================
void initGame() {
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); glEnable(GL_COLOR_MATERIAL);
    std::srand(std::time(0));

    // 1. Generate Jalur Sirkuit (Bentuk Gelombang)
    for(float i=0; i<2*M_PI; i+=0.005f) {
        float r = 700.0f + std::cos(i*3)*200.0f; 
        trackPoints.push_back(Vec3(std::cos(i)*r, 0, std::sin(i)*r));
    }
    car.alignToTrack(trackPoints);

    // 2. Generate Objek Udara (Awan & Aircraft Statis)
    for(int i=0; i<120; i++) {
        SceneryObject obj;
        if (i < 70) { // Awan
            obj.type = TYPE_CLOUD;
            obj.x = randFloat(-5000, 5000); obj.z = randFloat(-5000, 5000);
            obj.scaleY = randFloat(200, 400); obj.scaleX = randFloat(40, 100);
            obj.colRadius = 0;
        } else { // Pesawat & Heli
            obj.type = (rand()%2==0) ? TYPE_PLANE : TYPE_HELI;
            obj.x = randFloat(-3000, 3000); obj.z = randFloat(-3000, 3000);
            obj.scaleY = randFloat(250, 500); obj.rot = randFloat(0, 360);
            obj.scaleX = randFloat(1.0, 2.0); obj.colRadius = 0;
        }
        sceneryList.push_back(obj);
    }

    // 3. Generate Lingkungan (Pohon & Gedung Luar)
    for(int i=0; i<1000; i++) {
        float theta = randFloat(0, 2*M_PI);
        float rTrack = 700.0f + std::cos(theta*3)*200.0f;
        float rPos = randFloat(100, 1500);
        
        // Cek agar tidak bertabrakan dengan jalan
        if (std::abs(rPos - rTrack) < 60.0f) continue;

        SceneryObject obj;
        obj.x = std::cos(theta) * rPos; obj.z = std::sin(theta) * rPos;
        obj.rot = randFloat(0, 360);
        int roll = rand() % 100;
        if (roll < 60) {
            int t = rand()%3; obj.type = (t==0)?TYPE_TREE_PINE:(t==1)?TYPE_TREE_BIRCH:TYPE_TREE_APPLE;
            obj.scaleY = randFloat(15, 40); obj.colRadius = 2.0f;
        } else if (roll < 90) {
            obj.type = TYPE_BUILDING; obj.scaleX = randFloat(30, 60); obj.scaleY = randFloat(60, 300);
            obj.r = randFloat(0.3, 0.6); obj.g = obj.r; obj.b = obj.r + 0.1f;
            obj.colRadius = obj.scaleX * 0.6f;
        } else {
            obj.type = TYPE_BILLBOARD; obj.r = 1; obj.g = 0.2; obj.b = 0.2; obj.colRadius = 2.0f;
        }
        sceneryList.push_back(obj);
    }

    // 4. Generate Trackside (Grandstand, Tenda, Penonton)
    for(size_t i=0; i<trackPoints.size(); i+=10) {
        Vec3 p1 = trackPoints[i], p2 = trackPoints[(i+1)%trackPoints.size()];
        Vec3 n = Vec3(-(p2-p1).z, 0, (p2-p1).x).normalize();
        float side = (i % 20 == 0) ? 1.0f : -1.0f;

        SceneryObject obj;
        if (i % 80 == 0) { // Grandstand tiap interval tertentu
            obj.type = TYPE_GRANDSTAND; obj.x = p1.x + n.x*40*side; obj.z = p1.z + n.z*40*side;
            obj.rot = std::atan2(-n.x*side, -n.z*side) * 180/M_PI;
            obj.r = randFloat(0.2, 0.8); obj.g = 0.2; obj.b = 0.8; obj.colRadius = 15.0f;
        } else if (i % 80 == 40) { // Tenda di antara Grandstand
            obj.type = TYPE_TENT; obj.x = p1.x + n.x*30*side; obj.z = p1.z + n.z*30*side;
            obj.rot = std::atan2(-n.x*side, -n.z*side) * 180/M_PI;
            obj.scaleX = 7,5; obj.scaleY = 4.5; obj.r = 0.9; obj.g = 0.1; obj.b = 0.1; obj.colRadius = 8.0f;
        } else { // Sisanya penonton berdiri
            obj.type = TYPE_CROWD; obj.x = p1.x + n.x*22*side; obj.z = p1.z + n.z*22*side;
            obj.colRadius = 0; obj.scaleY = randFloat(1.5, 2.0);
        }
        sceneryList.push_back(obj);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("F1 SIMULATOR ULTIMATE EDITION V21.0");
    
    initGame();
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeyDown);
    glutTimerFunc(16, timer, 0);
    
    glutMainLoop();
    return 0;
}
