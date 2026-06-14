#include "Common.h"
#include "Scenery.h"
#include "Physics.h"
#include "Render.h"
#include "UI.h"

#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cctype>

GameState currentState = STATE_MENU;
int cameraMode = 0;
bool keys[256] = { false };
std::vector<Vec3> trackPoints;
Vec3 camPos, camLookAt;

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
        switch (cameraMode) {
            case 1:
                eyeX = car.pos.x + car.fwd.x * 3.2f; eyeY = 0.9f; eyeZ = car.pos.z + car.fwd.z * 3.2f;
                centerX = eyeX + car.fwd.x * 10.0f; centerY = 0.8f; centerZ = eyeZ + car.fwd.z * 10.0f;
                break;
            case 2:
                eyeX = car.pos.x - std::sin(car.angle + M_PI/2)*7.0f; eyeY = 1.8f; eyeZ = car.pos.z - std::cos(car.angle + M_PI/2)*7.0f;
                centerX = car.pos.x + car.fwd.x * 3.0f; centerY = 0.5f; centerZ = car.pos.z + car.fwd.z * 3.0f;
                break;
            case 3:
                eyeX = car.pos.x + std::sin(car.angle + M_PI/2)*7.0f; eyeY = 1.8f; eyeZ = car.pos.z + std::cos(car.angle + M_PI/2)*7.0f;
                centerX = car.pos.x + car.fwd.x * 3.0f; centerY = 0.5f; centerZ = car.pos.z + car.fwd.z * 3.0f;
                break;
            case 4:
                eyeX = car.pos.x + car.fwd.x * 12.0f; eyeY = 2.0f; eyeZ = car.pos.z + car.fwd.z * 12.0f;
                centerX = car.pos.x; centerY = 0.6f; centerZ = car.pos.z;
                break;
            default:
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

void keyboard(unsigned char k, int x, int y) {
    keys[std::tolower(k)] = true;
    
    if (k == 13 && currentState == STATE_MENU) currentState = STATE_STARTING;
    
    if (std::tolower(k) == 'e' && car.currentGear < 7) car.currentGear++;
    if (std::tolower(k) == 'q' && car.currentGear > 0) car.currentGear--;
    
    if (std::tolower(k) == 'f') cameraMode = 1; 
    if (std::tolower(k) == 'v') cameraMode = 0; 

    if (currentState == STATE_FINISHED && std::tolower(k) == 'r') {
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

void initGame() {
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); glEnable(GL_COLOR_MATERIAL);
    std::srand(std::time(0));


    RenderSystem::initTextures();

    // Jalur Sirkuit
    for (float i=0; i<2*M_PI; i+=0.005f) {
        float r = 700.0f + std::cos(i*3)*200.0f; 
        trackPoints.push_back(Vec3(std::cos(i)*r, 0, std::sin(i)*r));
    }
    car.alignToTrack(trackPoints);

    // Objek Udara
    for (int i=0; i<120; i++) {
        SceneryObject obj;
        if (i < 70) { 
            obj.type = TYPE_CLOUD;
            obj.x = randFloat(-5000, 5000); obj.z = randFloat(-5000, 5000);
            obj.scaleY = randFloat(200, 400); obj.scaleX = randFloat(40, 100);
            obj.colRadius = 0;
        } else { 
            obj.type = (rand()%2==0) ? TYPE_PLANE : TYPE_HELI;
            obj.x = randFloat(-3000, 3000); obj.z = randFloat(-3000, 3000);
            obj.scaleY = randFloat(250, 500); obj.rot = randFloat(0, 360);
            obj.scaleX = randFloat(1.0f, 2.0f); obj.colRadius = 0;
        }
        sceneryList.push_back(obj);
    }

    // Lingkungan
    for (int i=0; i<1000; i++) {
        float theta = randFloat(0, 2*M_PI);
        float rTrack = 700.0f + std::cos(theta*3)*200.0f;
        float rPos = randFloat(100, 1500);
        
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
            obj.r = randFloat(0.3f, 0.6f); obj.g = obj.r; obj.b = obj.r + 0.1f;
            obj.colRadius = obj.scaleX * 0.6f;
        } else {
            obj.type = TYPE_BILLBOARD; obj.r = 1; obj.g = 0.2f; obj.b = 0.2f; obj.colRadius = 2.0f;
        }
        sceneryList.push_back(obj);
    }

    // Trackside
    for (size_t i=0; i<trackPoints.size(); i+=10) {
        Vec3 p1 = trackPoints[i], p2 = trackPoints[(i+1)%trackPoints.size()];
        Vec3 n = Vec3(-(p2-p1).z, 0, (p2-p1).x).normalize();
        float side = (i % 20 == 0) ? 1.0f : -1.0f;

        SceneryObject obj;
        if (i % 80 == 0) { 
            obj.type = TYPE_GRANDSTAND; obj.x = p1.x + n.x*40*side; obj.z = p1.z + n.z*40*side;
            obj.rot = std::atan2(-n.x*side, -n.z*side) * 180/M_PI;
            obj.r = randFloat(0.2f, 0.8f); obj.g = 0.2f; obj.b = 0.8f; obj.colRadius = 15.0f;
        } else if (i % 80 == 40) { 
            obj.type = TYPE_TENT; obj.x = p1.x + n.x*30*side; obj.z = p1.z + n.z*30*side;
            obj.rot = std::atan2(-n.x*side, -n.z*side) * 180/M_PI;
            obj.scaleX = 7.5f; obj.scaleY = 4.5f; obj.r = 0.9f; obj.g = 0.1f; obj.b = 0.1f; obj.colRadius = 8.0f;
        } else { 
            obj.type = TYPE_CROWD; obj.x = p1.x + n.x*22*side; obj.z = p1.z + n.z*22*side;
            obj.colRadius = 0; obj.scaleY = randFloat(1.5f, 2.0f);
        }
        sceneryList.push_back(obj);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("F1 SIMULATOR");
    
    initGame();
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeyDown);
    glutTimerFunc(16, timer, 0);
    
    glutMainLoop();
    return 0;
}
