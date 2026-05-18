#include <windows.h> 
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cctype> 
#include <sstream> 
#include <cstdlib> 
#include <ctime>   

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
enum GameState { STATE_MENU, STATE_STARTING, STATE_RACING };
GameState currentState = STATE_MENU;

const float DT = 0.016f; 
const float TRACK_WIDTH = 18.0f;
const float MASS = 798.0f;           
const float MAX_ENGINE_FORCE = 18500.0f; 
const float WHEEL_RAD = 0.36f;
const float DRAG_COEFF = 0.85f;      
const float ROLLING_RESIST = 14.0f;  
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
// 2. PHYSICS & COLLISION SYSTEM 
// =============================================================
class PhysicsSystem {
public:
    Vec3 pos, vel, fwd;
    float angle, steerAngle, speedMS, rpm, wheelRot, startTimer;
    int currentGear; 
    bool isOffTrack;
    float gearRatios[10];

    PhysicsSystem() {
        float ratios[] = { -2.8f, 0.0f, 3.6f, 2.9f, 2.3f, 1.9f, 1.6f, 1.3f, 1.1f, 0.9f };
        for(int i=0; i<10; i++) gearRatios[i] = ratios[i];
        reset();
    }

    void reset() {
        pos = Vec3(0, WHEEL_RAD, 0);
        angle = 0; steerAngle = 0; speedMS = 0;
        rpm = 1000; wheelRot = 0; startTimer = 0;
        currentGear = 1; 
        isOffTrack = false;
    }

    void alignToTrack(const std::vector<Vec3>& track) {
        if(track.size() < 2) return;
        Vec3 direction = (track[1] - track[0]).normalize();
        angle = atan2(direction.x, direction.z);
        pos = track[0] + Vec3(0, WHEEL_RAD, 0);
        fwd = direction;
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
        if (currentState == STATE_MENU) return;
        if (currentState == STATE_STARTING) {
            startTimer += dt;
            if (startTimer > 3.0f) currentState = STATE_RACING;
            return;
        }

        fwd = Vec3(sin(angle), 0, cos(angle));
        float throttle = keys['w'] ? 1.0f : 0.0f;
        float brake = keys['s'] ? 1.0f : 0.0f;

        float gearFactor = gearRatios[currentGear];
        float engineForce = throttle * MAX_ENGINE_FORCE * gearFactor;
        
        if (throttle && std::abs(speedMS) < 5.0f && currentGear > 1) engineForce += 4500.0f; 

        float downforce = 0.5f * speedMS * speedMS; 
        float gripLimit = 1.0f + (downforce * 0.001f); 

        float airResistance = DRAG_COEFF * speedMS * speedMS * (speedMS >= 0 ? 1 : -1);
        float rollingResistance = ROLLING_RESIST * speedMS;
        float brakingForce = brake * 25000.0f * (speedMS >= 0 ? 1 : -1);

        float totalForce = engineForce - airResistance - rollingResistance - brakingForce;
        
        if (isOffTrack) { totalForce -= speedMS * 120.0f; totalForce *= 0.4f; }

        float acceleration = totalForce / MASS;
        speedMS += acceleration * dt;
        
        float maxSpeedGear = std::abs(gearFactor) * 48.0f; 
        if (std::abs(speedMS) > maxSpeedGear && currentGear != 1) speedMS = maxSpeedGear * (speedMS > 0 ? 1 : -1);

        pos = pos + fwd * (speedMS * dt);
        pos.y = WHEEL_RAD; 

        checkCollisions();

        float steerInput = (keys['a'] ? 1.0f : (keys['d'] ? -1.0f : 0.0f));
        float dynamicSensitivity = 1.0f / (1.0f + std::abs(speedMS) * 0.07f); 
        steerAngle += (steerInput * 0.45f * dynamicSensitivity - steerAngle) * 8.0f * dt;
        angle += steerAngle * speedMS * 0.045f * dt * gripLimit;

        rpm = 1000.0f + (std::abs(speedMS) * 230.0f / (std::abs(gearFactor) + 0.1f));
        if (rpm > 13000) rpm = 13000;
        wheelRot += (speedMS / WHEEL_RAD) * dt;

        checkBoundary(track);
    }

    void checkBoundary(const std::vector<Vec3>& track) {
        float minDist = 1e10;
        for(size_t i=0; i<track.size(); i++) {
            float d = Vec3::dist(pos, track[i]);
            if(d < minDist) minDist = d;
        }
        isOffTrack = (minDist > TRACK_WIDTH);
    }
};

PhysicsSystem car;
std::vector<Vec3> trackPoints;
Vec3 camPos, camLookAt;
bool keys[256] = { false }; 

// =============================================================
// 3. RENDER SYSTEM 
// =============================================================
class RenderSystem {
public:
    static void drawWheel(float x, float z, bool isFront) {
        glPushMatrix();
        glTranslatef(x, 0, z);
        if (isFront) glRotatef(car.steerAngle * 40.0f, 0, 1, 0);
        glRotatef(car.wheelRot * 180.0f/M_PI, 1, 0, 0);
        
        glColor3f(0.15f, 0.15f, 0.15f);
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        glTranslatef(0, 0, -x*0.4f);
        GLUquadric* q = gluNewQuadric();
        gluCylinder(q, 0.06, 0.06, std::abs(x)*0.4f, 8, 1);
        gluDeleteQuadric(q); 
        glPopMatrix();

        glColor3f(0.05f, 0.05f, 0.05f);
        glutSolidTorus(0.23, WHEEL_RAD, 15, 30);
        
        glColor3f(0.25f, 0.25f, 0.25f);
        glPushMatrix();
        glScalef(1.0f, 1.0f, 0.15f);
        glutSolidSphere(0.26, 12, 12);
        glPopMatrix();
        glPopMatrix();
    }

    static void drawF1Car() {
        glPushMatrix();
        glTranslatef(car.pos.x, car.pos.y, car.pos.z);
        glRotatef(car.angle * 180.0f/M_PI, 0, 1, 0);

        glColor3f(0.85f, 0.0f, 0.0f); 
        glPushMatrix(); glScalef(1.1f, 0.45f, 4.8f); glutSolidCube(1.0); glPopMatrix();

        glPushMatrix(); glTranslatef(0, -0.05f, 2.5f); glScalef(0.6f, 0.25f, 2.1f); glutSolidCube(1.0); glPopMatrix();

        float sides[] = {-1.0f, 1.0f};
        for(int i=0; i<2; i++) {
            glPushMatrix(); glTranslatef(0.82f * sides[i], -0.1f, 0.3f); glScalef(0.55f, 0.4f, 2.2f); glutSolidSphere(0.6, 12, 12); glPopMatrix();
        }

        glColor3f(0.1f, 0.1f, 0.1f);
        glPushMatrix(); glTranslatef(0, -0.15f, 3.5f); glScalef(3.4f, 0.06f, 0.8f); glutSolidCube(1.0); glPopMatrix();

        glPushMatrix(); glTranslatef(0, 0.8f, -2.2f); glScalef(2.2f, 0.1f, 0.9f); glutSolidCube(1.0);
        float wingSupports[] = {-0.6f, 0.6f};
        for(int i=0; i<2; i++) {
            glPushMatrix(); glTranslatef(wingSupports[i], -0.4f, 0); glScalef(0.1f, 0.8f, 0.1f); glutSolidCube(1.0); glPopMatrix();
        }
        glPopMatrix();

        drawWheel( 1.35f,  1.9f, true);
        drawWheel(-1.35f,  1.9f, true);
        drawWheel( 1.35f, -1.8f, false);
        drawWheel(-1.35f, -1.8f, false);

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

            // ================== PESAWAT & HELI (STATIS DI UDARA) ==================
            if (obj.type == TYPE_PLANE || obj.type == TYPE_HELI) {
                glPushMatrix();
                // Menggunakan titik asli (obj.x, obj.z) agar dia diam melayang di tempat
                glTranslatef(obj.x, obj.scaleY, obj.z);
                glRotatef(obj.rot, 0, 1, 0); 
                glScalef(obj.scaleX, obj.scaleX, obj.scaleX); // Varian ukuran

                if (obj.type == TYPE_PLANE) {
                    // Pesawat Terbang Diam
                    glColor3f(0.9f, 0.9f, 0.9f); 
                    glPushMatrix(); glScalef(4.0f, 1.0f, 1.0f); glutSolidSphere(1.0, 10, 10); glPopMatrix();
                    glColor3f(0.8f, 0.1f, 0.1f); 
                    glPushMatrix(); glTranslatef(0.5f, 0, 0); glScalef(2.0f, 0.1f, 6.0f); glutSolidCube(1.0); glPopMatrix();
                    glPushMatrix(); glTranslatef(-3.0f, 0.5f, 0); glScalef(1.0f, 1.5f, 0.1f); glutSolidCube(1.0); glPopMatrix();
                    glPushMatrix(); glTranslatef(-3.0f, 0, 0); glScalef(1.0f, 0.1f, 2.5f); glutSolidCube(1.0); glPopMatrix();
                } else {
                    // Helikopter Diam (tapi baling-baling tetap muter agar realistik)
                    glColor3f(0.15f, 0.15f, 0.15f); 
                    glPushMatrix(); glScalef(2.0f, 1.5f, 1.5f); glutSolidSphere(1.0, 10, 10); glPopMatrix();
                    glPushMatrix(); glTranslatef(-2.5f, 0, 0); glScalef(3.0f, 0.4f, 0.4f); glutSolidCube(1.0); glPopMatrix();
                    glPushMatrix(); glTranslatef(-4.0f, 0.5f, 0.2f); glScalef(0.1f, 1.0f, 0.1f); glutSolidCube(1.0); glPopMatrix();
                    
                    glColor3f(0.8f, 0.8f, 0.8f);
                    // Baling-baling Utama (Berputar)
                    glPushMatrix();
                    glTranslatef(0, 1.6f, 0);
                    glRotatef(timeMs * 1.5f, 0, 1, 0); 
                    glScalef(6.0f, 0.1f, 0.4f);
                    glutSolidCube(1.0);
                    glPopMatrix();
                    
                    // Baling-baling Ekor (Berputar)
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

            float dx = obj.x - camPos.x;
            float dz = obj.z - camPos.z;
            if((dx*dx + dz*dz) > 300000.0f) continue; 

            glPushMatrix();
            glTranslatef(obj.x, 0, obj.z);
            glRotatef(obj.rot, 0, 1, 0);

            if (obj.type == TYPE_TREE_PINE) {
                glColor3f(0.3f, 0.15f, 0.05f);
                glPushMatrix(); glTranslatef(0, obj.scaleY * 0.1f, 0); glScalef(1.0f, obj.scaleY * 0.2f, 1.0f); glutSolidCube(1.0); glPopMatrix();
                glColor3f(0.1f, 0.35f, 0.1f);
                glPushMatrix();
                glTranslatef(0, obj.scaleY * 0.2f, 0);
                glRotatef(-90.0f, 1, 0, 0); 
                glutSolidCone(obj.scaleY * 0.25f, obj.scaleY * 0.5f, 8, 2);
                glTranslatef(0, 0, obj.scaleY * 0.25f);
                glutSolidCone(obj.scaleY * 0.2f, obj.scaleY * 0.4f, 8, 2);
                glPopMatrix();

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
                
                glColor3f(0.9f, 0.1f, 0.1f);
                float r = obj.scaleY * 0.38f; 
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
                    float y = -obj.scaleY * 0.5f + (f * 10.0f);
                    float h = 2.5f; 
                    int randomSeed = (int)(obj.x + obj.z + f); 
                    if (randomSeed % 5 == 0) glColor3f(0.9f, 0.9f, 0.6f); 
                    else glColor3f(0.15f, 0.2f, 0.25f); 
                    glVertex3f(-w, y-h,  w); glVertex3f( w, y-h,  w); glVertex3f( w, y+h,  w); glVertex3f(-w, y+h,  w);
                    glVertex3f( w, y-h, -w); glVertex3f(-w, y-h, -w); glVertex3f(-w, y+h, -w); glVertex3f( w, y+h, -w);
                    glVertex3f( w, y-h,  w); glVertex3f( w, y-h, -w); glVertex3f( w, y+h, -w); glVertex3f( w, y+h,  w);
                    glVertex3f(-w, y-h, -w); glVertex3f(-w, y-h,  w); glVertex3f(-w, y+h,  w); glVertex3f(-w, y+h, -w);
                }
                glEnd();
                glEnable(GL_LIGHTING); glPopMatrix();

            } else if (obj.type == TYPE_BILLBOARD) {
                glColor3f(0.2f, 0.2f, 0.2f);
                glPushMatrix(); glTranslatef(0, 5.0f, 0); glScalef(1.0f, 10.0f, 1.0f); glutSolidCube(1.0); glPopMatrix();
                glColor3f(obj.r, obj.g, obj.b);
                glPushMatrix(); glTranslatef(0, 12.0f, 0); glScalef(15.0f, 6.0f, 1.0f); glutSolidCube(1.0); glPopMatrix();

            } else if (obj.type == TYPE_TENT) {
                float tW = obj.scaleX * 0.5f; 
                float tH = obj.scaleY;        
                
                glColor3f(0.8f, 0.8f, 0.8f); 
                float corners[4][2] = {{tW, tW}, {-tW, tW}, {tW, -tW}, {-tW, -tW}};
                for(int k=0; k<4; k++) {
                    glPushMatrix(); 
                    glTranslatef(corners[k][0], tH * 0.5f, corners[k][1]); 
                    glScalef(0.4f, tH, 0.4f); 
                    glutSolidCube(1.0); 
                    glPopMatrix();
                }

                glColor3f(obj.r, obj.g, obj.b); 
                glPushMatrix();
                glTranslatef(0, tH, 0);
                glRotatef(-90.0f, 1, 0, 0); 
                glRotatef(45.0f, 0, 0, 1);  
                glutSolidCone(obj.scaleX * 0.7f, obj.scaleY * 0.5f, 4, 1); 
                glPopMatrix();
                
                int peopleInTent = 6;
                for(int p = 0; p < peopleInTent; p++) {
                    float px = std::fmod((obj.x * 13.0f + p * 17.0f), tW * 1.5f) - (tW * 0.75f);
                    float pz = std::fmod((obj.z * 11.0f + p * 23.0f), tW * 1.5f) - (tW * 0.75f);
                    drawPersonAndFlash(px, 0.0f, pz, obj.x + p, timeMs);
                }

            } else if (obj.type == TYPE_CROWD) {
                drawPersonAndFlash(0, 0, 0, obj.x + obj.z, timeMs);
            
            } else if (obj.type == TYPE_GRANDSTAND) {
                int tiers = 5;               
                float tierDepth = 2.0f;      
                float tierHeight = 1.5f;     
                float standWidth = 26.0f;    

                glColor3f(0.4f, 0.4f, 0.4f);
                glPushMatrix(); glTranslatef(0, (tiers * tierHeight) * 0.5f, -(tiers * tierDepth)); glScalef(standWidth + 1.0f, tiers * tierHeight, 0.5f); glutSolidCube(1.0); glPopMatrix();

                glColor3f(0.7f, 0.7f, 0.7f);
                glPushMatrix(); glTranslatef(0, tiers * tierHeight + 1.5f, -(tiers * tierDepth) * 0.5f); glRotatef(12.0f, 1, 0, 0); glScalef(standWidth + 2.0f, 0.4f, tiers * tierDepth + 5.0f); glutSolidCube(1.0); glPopMatrix();

                for (int t = 0; t < tiers; t++) {
                    glColor3f(obj.r, obj.g, obj.b); 
                    glPushMatrix(); glTranslatef(0, t * tierHeight + 0.5f, -t * tierDepth); glScalef(standWidth, 1.0f, tierDepth); glutSolidCube(1.0); glPopMatrix();

                    int peopleCount = 14; 
                    float spacing = standWidth / peopleCount;
                    for(int p = 0; p < peopleCount; p++) {
                        float noiseX = std::fmod(obj.x + t * 3.0f + p * 7.0f, 0.8f) - 0.4f;
                        float noiseZ = std::fmod(obj.z + t * 5.0f + p * 2.0f, 0.8f) - 0.4f;
                        float px = -standWidth * 0.5f + (p + 0.5f) * spacing + noiseX;
                        float pz = -t * tierDepth + noiseZ;
                        float py = t * tierHeight + 1.0f; 

                        drawPersonAndFlash(px, py, pz, obj.x + t + p, timeMs);
                    }
                }
            }
            glPopMatrix();
        }
    }

    static void drawTrack() {
        glDisable(GL_LIGHTING);
        
        glColor3f(0.12f, 0.32f, 0.12f);
        glBegin(GL_QUADS);
        glVertex3f(-4000, 0, -4000); glVertex3f(4000, 0, -4000);
        glVertex3f(4000, 0, 4000); glVertex3f(-4000, 0, 4000);
        glEnd();

        glBegin(GL_QUAD_STRIP);
        for(size_t i=0; i<=trackPoints.size(); i++) {
            int idx = i % trackPoints.size();
            int next_idx = (i + 1) % trackPoints.size();
            Vec3 p1 = trackPoints[idx];
            Vec3 p2 = trackPoints[next_idx];
            Vec3 n = Vec3(-(p2-p1).z, 0, (p2-p1).x).normalize();
            
            if (idx % 4 < 2) glColor3f(0.8f, 0.1f, 0.1f);
            else glColor3f(0.9f, 0.9f, 0.9f);
            
            glVertex3f(p1.x - n.x*(TRACK_WIDTH+1.5f), 0.015f, p1.z - n.z*(TRACK_WIDTH+1.5f));
            glVertex3f(p1.x + n.x*(TRACK_WIDTH+1.5f), 0.015f, p1.z + n.z*(TRACK_WIDTH+1.5f));
        }
        glEnd();

        glColor3f(0.18f, 0.18f, 0.2f);
        glBegin(GL_QUAD_STRIP);
        for(size_t i=0; i<=trackPoints.size(); i++) {
            int idx = i % trackPoints.size();
            int next_idx = (i + 1) % trackPoints.size();
            Vec3 p1 = trackPoints[idx];
            Vec3 p2 = trackPoints[next_idx];
            Vec3 n = Vec3(-(p2-p1).z, 0, (p2-p1).x).normalize();
            
            glVertex3f(p1.x - n.x*TRACK_WIDTH, 0.02f, p1.z - n.z*TRACK_WIDTH);
            glVertex3f(p1.x + n.x*TRACK_WIDTH, 0.02f, p1.z + n.z*TRACK_WIDTH);
        }
        glEnd();
        
        glEnable(GL_LIGHTING);
    }
};

// =============================================================
// 4. CAMERA & UI SYSTEM 
// =============================================================
class CameraSystem {
public:
    static void update() {
        int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);
        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        gluPerspective(65.0f, (float)w/h, 0.1f, 5000.0f);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();

        static float orbit = 0; orbit += 0.006f;
        Vec3 targetPos, targetLook;

        if (currentState == STATE_MENU) {
            targetPos = car.pos + Vec3(sin(orbit)*28, 14, cos(orbit)*28);
            targetLook = car.pos;
            camPos = targetPos; camLookAt = targetLook;
        } 
        else if (currentState == STATE_STARTING) {
            float t = car.startTimer / 3.0f; 
            Vec3 menuPos = car.pos + Vec3(sin(orbit)*28, 14, cos(orbit)*28);
            Vec3 racePos = car.pos - car.fwd * 15.0f + Vec3(0, 5.0f, 0);
            camPos = menuPos * (1.0f-t) + racePos * t;
            camLookAt = car.pos;
        } 
        else {
            targetPos = car.pos - car.fwd * 15.0f + Vec3(0, 5.0f, 0);
            targetLook = car.pos + car.fwd * 10.0f;
            camPos = camPos + (targetPos - camPos) * 0.1f;
            camLookAt = camLookAt + (targetLook - camLookAt) * 0.1f;
        }

        gluLookAt(camPos.x, camPos.y, camPos.z, camLookAt.x, camLookAt.y, camLookAt.z, 0, 1, 0);
    }
};

class UISystem {
public:
    static void drawText(int x, int y, std::string s, void* font) {
        glRasterPos2i(x, y);
        for(size_t i=0; i<s.length(); i++) glutBitmapCharacter(font, s[i]);
    }

    static void render() {
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 1280, 0, 720);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);

        if (currentState == STATE_MENU) {
            glColor3f(1, 1, 1);
            drawText(500, 420, "F1 SIMULATOR v21.0 (HOVERING AIRSPACE)", GLUT_BITMAP_TIMES_ROMAN_24);
            drawText(540, 380, "PRESS [ENTER] TO START", GLUT_BITMAP_HELVETICA_18);
            drawText(50, 50, "W/S: Throttle/Brake  A/D: Steer  Q/E: Gear", GLUT_BITMAP_HELVETICA_12);
        } 
        else if (currentState == STATE_STARTING) {
            int countdown = 3 - (int)car.startTimer;
            glColor3f(1, 1, 0);
            drawText(620, 400, countdown > 0 ? to_str(countdown) : "GO!", GLUT_BITMAP_TIMES_ROMAN_24);
        } 
        else {
            glColor3f(1, 1, 1);
            drawText(50, 680, "SPEED: " + to_str((int)(std::abs(car.speedMS)*3.6f)) + " KM/H", GLUT_BITMAP_HELVETICA_18);
            std::string gearNames[] = {"R", "N", "1", "2", "3", "4", "5", "6", "7", "8"};
            drawText(50, 650, "GEAR: " + gearNames[car.currentGear], GLUT_BITMAP_HELVETICA_18);
            
            float rpmRatio = (car.rpm - 1000) / 12000.0f;
            glColor3f(0.1f, 0.1f, 0.1f); glRectf(50, 620, 350, 635);
            glColor3f(rpmRatio > 0.9f ? 1.0f : 0.0f, 0.8f, 0.0f); 
            glRectf(50, 620, 50 + rpmRatio*300, 635);

            if (car.isOffTrack) {
                glColor3f(1, 0, 0); drawText(600, 600, "LOW GRIP - OFF TRACK", GLUT_BITMAP_HELVETICA_12);
            }
        }

        glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
        glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    }
};

// =============================================================
// 6. MAIN CALLBACKS & INITIALIZATION
// =============================================================
void display() {
    glClearColor(0.4f, 0.75f, 1.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CameraSystem::update();

    GLfloat light_pos[] = { car.pos.x + 100.0f, 300.0f, car.pos.z + 100.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    RenderSystem::drawTrack();
    RenderSystem::drawScenery(); 
    RenderSystem::drawF1Car();
    UISystem::render();

    glutSwapBuffers();
}

void timer(int v) {
    car.update(DT, keys, trackPoints);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char k, int x, int y) {
    keys[tolower(k)] = true;
    if(k == 13 && currentState == STATE_MENU) currentState = STATE_STARTING;
    if(tolower(k) == 'e' && car.currentGear < 9) car.currentGear++;
    if(tolower(k) == 'q' && car.currentGear > 0) car.currentGear--;
}

void keyboardUp(unsigned char k, int x, int y) { keys[tolower(k)] = false; }

void initGame() { 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    
    for(float i=0; i<2*M_PI; i+=0.005f) {
        float r = 550.0f + sin(i*4)*120.0f;
        trackPoints.push_back(Vec3(cos(i)*r, 0, sin(i)*r));
    }
    car.alignToTrack(trackPoints);

    srand((unsigned)time(0));
    
    // 2. Generate Awan & Aircraft Statis (DIPERBANYAK)
    for(int i=0; i<100; i++) { // Jumlah total benda di udara (naik jadi 100)
        SceneryObject obj;
        if (i < 50) { 
            // 50 Awan
            obj.type = TYPE_CLOUD;
            obj.x = randFloat(-4000.0f, 4000.0f);
            obj.z = randFloat(-4000.0f, 4000.0f);
            obj.scaleY = randFloat(150.0f, 300.0f); 
            obj.scaleX = randFloat(30.0f, 80.0f);   
            obj.colRadius = 0.0f; 
        } else {
            // 50 Pesawat/Heli (Lebih sering terlihat karena jumlahnya banyak dan radius sempit)
            obj.type = (i % 2 == 0) ? TYPE_PLANE : TYPE_HELI;
            obj.x = randFloat(-2500.0f, 2500.0f); // Jarak dipersingkat agar lebih sering di atas kepala
            obj.z = randFloat(-2500.0f, 2500.0f);
            obj.scaleY = randFloat(150.0f, 300.0f); // Tinggi stabil
            obj.rot = randFloat(0, 360);            
            obj.scaleX = randFloat(0.8f, 1.5f);     // Variasi ukuran
            obj.colRadius = 0.0f; 
        }
        sceneryList.push_back(obj);
    }

    // 3. Generate Pemandangan Alam & Gedung
    for(int i=0; i<800; i++) { 
        float r = randFloat(100.0f, 1200.0f); 
        float theta = randFloat(0, 2 * M_PI);
        
        SceneryObject obj;
        int typeRoll = rand() % 100;

        if (r > 360 && r < 740) continue; 

        obj.x = cos(theta) * r;
        obj.z = sin(theta) * r;
        obj.rot = randFloat(0, 360);
        
        if (typeRoll < 60) { 
            int treeType = rand() % 3;
            if(treeType == 0) obj.type = TYPE_TREE_PINE;
            else if(treeType == 1) obj.type = TYPE_TREE_BIRCH;
            else obj.type = TYPE_TREE_APPLE;
            
            obj.scaleY = randFloat(15.0f, 35.0f); 
            obj.colRadius = 1.5f + (obj.scaleY * 0.05f); 
        } 
        else if (typeRoll < 90) { 
            obj.type = TYPE_BUILDING;
            obj.scaleX = randFloat(25.0f, 50.0f); obj.scaleY = randFloat(50.0f, 250.0f); 
            obj.r = randFloat(0.3f, 0.5f); obj.g = obj.r + randFloat(-0.1f, 0.1f); obj.b = obj.r + randFloat(0.0f, 0.2f);
            obj.colRadius = obj.scaleX * 0.6f; 
        } 
        else {
            obj.type = TYPE_BILLBOARD;
            obj.r = randFloat(0.5f, 1.0f); obj.g = randFloat(0.0f, 0.5f); obj.b = randFloat(0.0f, 0.5f);
            obj.colRadius = 1.5f; 
        }
        sceneryList.push_back(obj);
    }

    // 4. Generate Penonton, Tribun, & Tenda
    for (size_t i = 0; i < trackPoints.size(); i += 8) { 
        Vec3 p1 = trackPoints[i];
        Vec3 p2 = trackPoints[(i + 1) % trackPoints.size()];
        Vec3 n = Vec3(-(p2 - p1).z, 0, (p2 - p1).x).normalize(); 

        float side = (rand() % 2 == 0) ? 1.0f : -1.0f;
        
        if (i % 64 == 0) {
            float offsetDist = TRACK_WIDTH + 18.0f; 
            SceneryObject stand;
            stand.type = TYPE_GRANDSTAND;
            stand.x = p1.x + n.x * offsetDist * side;
            stand.z = p1.z + n.z * offsetDist * side;
            stand.rot = atan2(-n.x * side, -n.z * side) * 180.0f / M_PI; 
            stand.r = randFloat(0.2f, 0.8f); stand.g = randFloat(0.2f, 0.8f); stand.b = randFloat(0.2f, 0.8f);
            stand.colRadius = 15.0f; 
            sceneryList.push_back(stand);
        } 
        else if (i % 64 == 32) {
            float offsetDist = TRACK_WIDTH + 15.0f; 
            SceneryObject tent;
            tent.type = TYPE_TENT;
            tent.x = p1.x + n.x * offsetDist * side;
            tent.z = p1.z + n.z * offsetDist * side;
            tent.rot = atan2(-n.x * side, -n.z * side) * 180.0f / M_PI;
            tent.scaleX = randFloat(15.0f, 25.0f); 
            tent.scaleY = randFloat(8.0f, 15.0f);  
            
            int c = rand() % 3;
            if(c == 0) { tent.r=0.9f; tent.g=0.9f; tent.b=0.9f; }
            else if(c == 1) { tent.r=0.8f; tent.g=0.1f; tent.b=0.1f; }
            else { tent.r=0.1f; tent.g=0.2f; tent.b=0.8f; }

            tent.colRadius = tent.scaleX * 0.5f; 
            sceneryList.push_back(tent);
        }
        else {
            int groupSize = rand() % 4; 
            for (int j = 0; j < groupSize; j++) {
                float offsetDist = (TRACK_WIDTH + 1.5f) + randFloat(3.0f, 10.0f); 
                SceneryObject person;
                person.type = TYPE_CROWD;
                person.x = p1.x + n.x * offsetDist * side + randFloat(-2.0f, 2.0f);
                person.z = p1.z + n.z * offsetDist * side + randFloat(-2.0f, 2.0f);
                person.rot = atan2(-n.x * side, -n.z * side) * 180.0f / M_PI;
                person.colRadius = 0.0f; 
                sceneryList.push_back(person);
            }
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("F1 Simulator Engine v21.0 - Hovering Airspace");
    
    initGame(); 
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(16, timer, 0);
    
    glutMainLoop();
    return 0;
}
