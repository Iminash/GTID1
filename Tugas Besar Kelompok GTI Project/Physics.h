#ifndef PHYSICS_H
#define PHYSICS_H

#include "Common.h"
#include "Scenery.h"
#include <vector>

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

    float gearMaxSpeed[8];
    float gearRatios[8];

    PhysicsSystem();
    void reset();
    void alignToTrack(const std::vector<Vec3>& track);
    void checkCollisions();
    void update(float dt, bool keys[], const std::vector<Vec3>& track);
};

extern PhysicsSystem car;

#endif
