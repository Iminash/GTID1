// File: Scenery.h
#ifndef SCENERY_H
#define SCENERY_H

#include "Common.h"
#include <vector>

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

extern std::vector<SceneryObject> sceneryList;

#endif
