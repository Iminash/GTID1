#include "UI.h"
#include "Physics.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

float UISystem::totalTime = 0.0f;

std::string UISystem::formatTime(float t) {
    int m = (int)t / 60, s = (int)t % 60, ms = (int)((t - std::floor(t)) * 100);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << m << ":" << std::setw(2) << s << ":" << std::setw(2) << ms;
    return ss.str();
}

void UISystem::drawText(int x, int y, std::string s, void* font) {
    glRasterPos2i(x, y);
    for(size_t i=0; i<s.length(); i++) glutBitmapCharacter(font, s[i]);
}

void UISystem::render() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 1280, 0, 720);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST); glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (currentState == STATE_MENU) {
        glColor3f(1,1,1); drawText(550, 420, "F1 SIMULATOR", GLUT_BITMAP_TIMES_ROMAN_24);
        drawText(560, 380, "PRESS ENTER TO START", GLUT_BITMAP_HELVETICA_12);
    } else if (currentState == STATE_RACING || currentState == STATE_STARTING) {
        // MINIMAP
        float cx = 130, cy = 130, r = 100, sc = 0.06f;
        glColor4f(0, 0, 0, 0.4f); glBegin(GL_TRIANGLE_FAN); for(int i=0; i<32; i++) glVertex2f(cx+std::cos(i*M_PI/16)*r, cy+std::sin(i*M_PI/16)*r); glEnd();
        glColor3f(0.5f, 0.5f, 0.5f); glBegin(GL_LINE_LOOP); for(size_t k=0; k<trackPoints.size(); k++) glVertex2f(cx+trackPoints[k].x*sc, cy+trackPoints[k].z*sc); glEnd();
        glColor3f(0.2f, 1.0f, 0.2f); glBegin(GL_TRIANGLE_FAN); for(int i=0; i<16; i++) glVertex2f(cx+trackPoints[0].x*sc + std::cos(i*M_PI/8)*4, cy+trackPoints[0].z*sc + std::sin(i*M_PI/8)*4); glEnd();
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
        glColor3f(0.3f, 0.3f, 0.3f); glBegin(GL_LINE_LOOP); glVertex2f(bX, bY+20); glVertex2f(bX+200, bY+20); glVertex2f(bX+200, bY+35); glVertex2f(bX, bY+35); glEnd();
    }

    if (currentState == STATE_STARTING) {
        for (int i=0; i<5; i++) {
            glColor3f(i<car.startLights?1.0f:0.1f, 0, 0);
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
