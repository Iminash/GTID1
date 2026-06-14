#include "Physics.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

PhysicsSystem car;

PhysicsSystem::PhysicsSystem() {
    maxLap = 3;
    float maxSpeeds[] = { 35.0f, 0.0f, 90.0f, 140.0f, 190.0f, 240.0f, 290.0f, 340.0f };
    float ratios[]    = { -2.8f, 0.0f, 4.0f, 3.2f, 2.6f, 2.1f, 1.7f, 1.3f };
    for (int i = 0; i < 8; ++i) {
        gearMaxSpeed[i] = maxSpeeds[i];
        gearRatios[i] = ratios[i];
    }
    reset();
}

void PhysicsSystem::reset() {
    pos = Vec3(0, WHEEL_RAD, 0); angle = 0; steerAngle = 0; speedMS = 0;
    rpm = 1000; wheelRot = 0; startTimer = 0; throttleSmooth = 0;
    startLights = 0; lightTimer = 0; currentGear = 1; isOffTrack = false;
    currentFOV = 62.0f; lapCount = 0; lapTimer = 0; totalTimer = 0; 
    bestLapTime = 0; crossedStartLine = true;
}

void PhysicsSystem::alignToTrack(const std::vector<Vec3>& track) {
    if(track.size() < 2) return;
    Vec3 dir = (track[1] - track[0]).normalize();
    angle = atan2(dir.x, dir.z);
    pos = track[0] - dir * 15.0f + Vec3(0, WHEEL_RAD, 0);
}

void PhysicsSystem::checkCollisions() {
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

void PhysicsSystem::update(float dt, bool keys[], const std::vector<Vec3>& track) {
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

    checkCollisions();

    rpm = 1000.0f + (std::abs(speedMS) * 265.0f / (std::abs(gearFactor) + 0.35f));
    if (rpm > 13500) rpm = 13500 - (rand()%150);
    wheelRot += (speedMS / WHEEL_RAD) * dt;
    currentFOV = 62.0f + (std::abs(speedMS) * 0.12f);
}
