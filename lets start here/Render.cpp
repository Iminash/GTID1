// File: Render.cpp
#include "Render.h"
#include "imageloader.h" 
#include <algorithm>
#include <cmath>

GLuint texGrass;
GLuint texBillboard;

void RenderSystem::glShadowProjection(float * l, float * e, float * n) {
    float mat[16];
    float lw = 1.0f; 
    
    float D = -(n[0]*e[0] + n[1]*e[1] + n[2]*e[2]);
    float d = n[0]*l[0] + n[1]*l[1] + n[2]*l[2] + D*lw;
    
    mat[0]  = d - l[0] * n[0];
    mat[1]  = - l[1] * n[0];
    mat[2]  = - l[2] * n[0];
    mat[3]  = - lw   * n[0];
    
    mat[4]  = - l[0] * n[1];
    mat[5]  = d - l[1] * n[1];
    mat[6]  = - l[2] * n[1];
    mat[7]  = - lw   * n[1];
    
    mat[8]  = - l[0] * n[2];
    mat[9]  = - l[1] * n[2];
    mat[10] = d - l[2] * n[2];
    mat[11] = - lw   * n[2];
    
    mat[12] = - l[0] * D;
    mat[13] = - l[1] * D;
    mat[14] = - l[2] * D;
    mat[15] = d - lw * D;
    
    glMultMatrixf(mat);
}

GLuint loadTexture(Image* image) {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image->width, image->height, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    return textureId;
}

void RenderSystem::initTextures() {
    Image* imgB = loadBMP("billboard.bmp"); 
    texBillboard = loadTexture(imgB);
    delete imgB;

    Image* imgC = loadBMP("grass.bmp"); 
    texGrass = loadTexture(imgC);
    delete imgC;
}

void RenderSystem::drawBox(float sx, float sy, float sz) {
    glPushMatrix(); glScalef(sx, sy, sz); glutSolidCube(1.0f); glPopMatrix();
}

void RenderSystem::setRed() { glColor3f(0.9f, 0.0f, 0.0f); }
void RenderSystem::setWhite() { glColor3f(1.0f, 1.0f, 1.0f); }
void RenderSystem::setBlack() { glColor3f(0.05f, 0.05f, 0.05f); }
void RenderSystem::setYellow() { glColor3f(1.0f, 0.8f, 0.0f); }

void RenderSystem::drawWheel(float x, float y, float z, bool isFront, bool isShadow) {
    glPushMatrix();
    glTranslatef(x, y, z);
    if (isFront) glRotatef(car.steerAngle * 45.0f, 0, 1, 0);
    glRotatef(car.wheelRot * 180.0f / M_PI, 1, 0, 0);

    if (!isShadow) setBlack();
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    float thick = isFront ? 0.48f : 0.45f; 
    float rad   = isFront ? 0.40f : 0.55f; 
    glutSolidTorus(thick/2.0f, rad, 15, 15);
    glPopMatrix();

    if (!isShadow) setYellow();
    glPushMatrix();
    glTranslatef(x > 0 ? 0.02f : -0.02f, 0, 0);
    glRotatef(90, 0, 1, 0);
    glutSolidTorus(0.01f, rad * 0.75f, 10, 10);
    glPopMatrix();

    if (!isShadow) glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glScalef(0.1f, 1.0f, 1.0f);
    glutSolidSphere(rad * 0.5f, 10, 10);
    glPopMatrix();
    glPopMatrix();
}

// --> PERBAIKAN: Menambahkan parameter Y agar sudut suspensi presisi ke pusat roda
void RenderSystem::drawSuspension(float x, float y, float z, bool isShadow) {
    if (!isShadow) setBlack();
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.2f, z); glVertex3f(x, y, z);
        glVertex3f(0.0f, 0.4f, z); glVertex3f(x, y, z);
    glEnd();
}

void RenderSystem::drawCarGeometry(bool isShadow) {
    if (!isShadow) setRed();
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

    if (!isShadow) setWhite();
    glPushMatrix(); glTranslatef(0.0f, 0.42f, 1.8f); drawBox(0.35f, 0.02f, 1.2f); glPopMatrix();

    if (!isShadow) setRed();
    glPushMatrix(); glTranslatef(0.0f, 0.35f, -0.4f); drawBox(0.75f, 0.55f, 1.6f); glPopMatrix();

    glBegin(GL_TRIANGLES);
        glVertex3f(-0.30f, 0.60f, -1.0f); glVertex3f(0.30f, 0.60f, -1.0f); glVertex3f(0.0f, 1.0f, -1.0f);
    glEnd();

    glBegin(GL_QUADS);
        if (!isShadow) setRed();
        glVertex3f(-0.37f, 0.60f, -1.2f); glVertex3f(0.37f, 0.60f, -1.2f);
        glVertex3f(0.05f, 0.50f, -2.2f);  glVertex3f(-0.05f, 0.50f, -2.2f);
    glEnd();

    glPushMatrix();
    glTranslatef(0.7f, 0.30f, -0.6f); drawBox(0.6f, 0.55f, 1.4f);
    glTranslatef(-1.4f, 0.0f, 0.0f); drawBox(0.6f, 0.55f, 1.4f);
    glPopMatrix();

    if (!isShadow) setRed();
    glPushMatrix(); glTranslatef(0.0f, 0.85f, -2.0f); drawBox(2.0f, 0.1f, 0.7f); 
    if (!isShadow) setWhite(); glTranslatef(0.0f, 0.06f, 0.0f); drawBox(2.0f, 0.02f, 0.7f); glPopMatrix();

    if (!isShadow) setRed();
    glPushMatrix(); glTranslatef(1.0f, 0.65f, -2.0f); drawBox(0.05f, 0.6f, 0.8f);
    glTranslatef(-2.0f, 0.0f, 0.0f); drawBox(0.05f, 0.6f, 0.8f); glPopMatrix();

    if (!isShadow) glColor3f(0.8f, 0.8f, 0.8f);
    glPushMatrix(); glTranslatef(0.0f, 0.12f, 3.2f); drawBox(2.8f, 0.05f, 0.7f); glPopMatrix();

    if (!isShadow) setWhite();
    glPushMatrix(); glTranslatef(0.0f, 0.62f, -0.35f); glutSolidSphere(0.2f, 12, 12); glPopMatrix();

    // --> PERBAIKAN KOORDINAT BAN: Ban diangkat agar rata dengan sirkuit
    // Roda Depan (Y Center = 0.06f)
    drawSuspension(1.0f, 0.06f, 2.5f, isShadow); drawSuspension(-1.0f, 0.06f, 2.5f, isShadow);
    drawWheel(0.95f, 0.06f, 2.5f, true, isShadow); drawWheel(-0.95f, 0.06f, 2.5f, true, isShadow);
    
    // Roda Belakang (Y Center = 0.21f)
    drawSuspension(1.0f, 0.21f, -1.3f, isShadow); drawSuspension(-1.0f, 0.21f, -1.3f, isShadow);
    drawWheel(1.05f, 0.21f, -1.3f, false, isShadow); drawWheel(-1.05f, 0.21f, -1.3f, false, isShadow);
}

void RenderSystem::drawCar() {
    float lightPos[] = { car.pos.x + 100.0f, 400.0f, car.pos.z + 100.0f };
    float planeNormal[] = { 0.0f, 1.0f, 0.0f };
    float planePoint[] = { 0.0f, 0.05f, 0.0f };

    glPushMatrix();
        glShadowProjection(lightPos, planePoint, planeNormal);
        glTranslatef(car.pos.x, car.pos.y, car.pos.z);
        glRotatef(car.angle * 180.0f / M_PI, 0, 1, 0);

        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.45f); 

        drawCarGeometry(true);

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(car.pos.x, car.pos.y, car.pos.z);
        glRotatef(car.angle * 180.0f / M_PI, 0, 1, 0);
        drawCarGeometry(false);
    glPopMatrix();
}

void RenderSystem::drawPersonAndFlash(float x, float y, float z, float seedVal, float timeMs, bool isShadow) {
    glPushMatrix();
    glTranslatef(x, y, z);
    
    int seed = (int)(std::abs(seedVal));
    float r = (seed % 3 == 0) ? 0.9f : ((seed % 3 == 1) ? 0.2f : 0.8f);
    float g = (seed % 4 == 0) ? 0.2f : ((seed % 4 == 1) ? 0.8f : 0.9f);
    float b = (seed % 5 == 0) ? 0.1f : ((seed % 5 == 1) ? 0.9f : 0.2f);
    if(r>0.7f && g>0.7f && b>0.7f) { r=1.0f; g=1.0f; b=1.0f; }

    if (!isShadow) glColor3f(0.2f, 0.2f, 0.3f);
    glPushMatrix(); glTranslatef(0, 0.4f, 0); glScalef(0.4f, 0.8f, 0.25f); glutSolidCube(1.0); glPopMatrix();

    if (!isShadow) glColor3f(r, g, b);
    glPushMatrix(); glTranslatef(0, 1.1f, 0); glScalef(0.5f, 0.6f, 0.3f); glutSolidCube(1.0); glPopMatrix();

    if (!isShadow) glColor3f(0.9f, 0.75f, 0.6f);
    glPushMatrix(); glTranslatef(0, 1.55f, 0); glutSolidSphere(0.2, 6, 6); glPopMatrix();

    if (!isShadow) {
        float flashTime = timeMs * 0.003f + seed * 10.0f;
        if (std::sin(flashTime) > 0.99f) { 
            glDisable(GL_LIGHTING); 
            glColor3f(1.0f, 1.0f, 1.0f);
            glPushMatrix(); glTranslatef(0.2f, 1.3f, 0.3f); glutSolidSphere(0.4, 8, 8); glPopMatrix();
            glEnable(GL_LIGHTING);
        }
    }

    glPopMatrix();
}

void RenderSystem::drawTreeGeometry(const SceneryObject& obj, bool isShadow) {
    if (obj.type == TYPE_TREE_PINE) {
        if (!isShadow) glColor3f(0.3f, 0.15f, 0.05f);
        glPushMatrix(); glTranslatef(0, obj.scaleY * 0.1f, 0); glScalef(1.0f, obj.scaleY * 0.2f, 1.0f); glutSolidCube(1.0); glPopMatrix();
        if (!isShadow) glColor3f(0.1f, 0.35f, 0.1f);
        glPushMatrix(); glTranslatef(0, obj.scaleY * 0.2f, 0); glRotatef(-90.0f, 1, 0, 0); 
        glutSolidCone(obj.scaleY * 0.25f, obj.scaleY * 0.5f, 8, 2);
        glTranslatef(0, 0, obj.scaleY * 0.25f); glutSolidCone(obj.scaleY * 0.2f, obj.scaleY * 0.4f, 8, 2); glPopMatrix();
    } else if (obj.type == TYPE_TREE_BIRCH) {
        if (!isShadow) glColor3f(0.9f, 0.9f, 0.9f);
        glPushMatrix(); glTranslatef(0, obj.scaleY * 0.25f, 0); glScalef(0.6f, obj.scaleY * 0.5f, 0.6f); glutSolidCube(1.0); glPopMatrix();
        if (!isShadow) glColor3f(0.6f, 0.8f, 0.2f);
        glPushMatrix(); glTranslatef(0, obj.scaleY * 0.7f, 0); glScalef(1.0f, 1.3f, 1.0f); glutSolidSphere(obj.scaleY * 0.35f, 7, 7); glPopMatrix();
    } else if (obj.type == TYPE_TREE_APPLE) {
        if (!isShadow) glColor3f(0.4f, 0.2f, 0.1f);
        glPushMatrix(); glTranslatef(0, obj.scaleY * 0.2f, 0); glScalef(1.0f, obj.scaleY * 0.4f, 1.0f); glutSolidCube(1.0); glPopMatrix();
        if (!isShadow) glColor3f(0.15f, 0.5f, 0.15f);
        glPushMatrix(); glTranslatef(0, obj.scaleY * 0.7f, 0); glutSolidSphere(obj.scaleY * 0.4f, 8, 8); 
        if (!isShadow) glColor3f(0.9f, 0.1f, 0.1f); 
        float r = obj.scaleY * 0.38f; 
        glPushMatrix(); glTranslatef(r, 0, 0); glutSolidSphere(1.2, 5, 5); glPopMatrix();
        glPushMatrix(); glTranslatef(-r, r*0.5f, 0); glutSolidSphere(1.2, 5, 5); glPopMatrix();
        glPushMatrix(); glTranslatef(0, -r*0.3f, r); glutSolidSphere(1.2, 5, 5); glPopMatrix();
        glPushMatrix(); glTranslatef(0, r*0.4f, -r); glutSolidSphere(1.2, 5, 5); glPopMatrix();
        glPopMatrix(); 
    }
}

void RenderSystem::drawBuildingGeometry(const SceneryObject& obj, bool isShadow) {
    glPushMatrix(); glTranslatef(0, obj.scaleY * 0.5f, 0); 
    if (!isShadow) glColor3f(obj.r, obj.g, obj.b);
    glPushMatrix(); glScalef(obj.scaleX, obj.scaleY, obj.scaleX); glutSolidCube(1.0); glPopMatrix();
    
    if (!isShadow) {
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
        glEnd(); glEnable(GL_LIGHTING); 
    }
    glPopMatrix();
}

void RenderSystem::drawObjectGeometry(const SceneryObject& obj, bool isShadow, float timeMs) {
    if (obj.type == TYPE_TREE_PINE || obj.type == TYPE_TREE_BIRCH || obj.type == TYPE_TREE_APPLE) {
        drawTreeGeometry(obj, isShadow);
    } 
    else if (obj.type == TYPE_BUILDING) {
        drawBuildingGeometry(obj, isShadow);
    } 
    else if (obj.type == TYPE_BILLBOARD) {
        float bw = 7.5f; float bh = 3.0f; float bd = 0.5f; 
        
        if (!isShadow) glColor3f(0.2f, 0.2f, 0.2f); 
        glPushMatrix(); glTranslatef(0, 5.0f, 0); glScalef(1.0f, 10.0f, 1.0f); glutSolidCube(1.0); glPopMatrix();
        
        glPushMatrix(); 
        glTranslatef(0, 12.0f, 0); 
        
        if (!isShadow) glColor3f(obj.r, obj.g, obj.b); 
        glBegin(GL_QUADS);
            glNormal3f(0, 0, -1);
            glVertex3f(-bw, -bh, -bd); glVertex3f(-bw, bh, -bd); glVertex3f(bw, bh, -bd); glVertex3f(bw, -bh, -bd);
            glNormal3f(-1, 0, 0);
            glVertex3f(-bw, -bh, -bd); glVertex3f(-bw, -bh, bd); glVertex3f(-bw, bh, bd); glVertex3f(-bw, bh, -bd);
            glNormal3f(1, 0, 0);
            glVertex3f(bw, -bh, bd); glVertex3f(bw, -bh, -bd); glVertex3f(bw, bh, -bd); glVertex3f(bw, bh, bd);
            glNormal3f(0, 1, 0);
            glVertex3f(-bw, bh, -bd); glVertex3f(-bw, bh, bd); glVertex3f(bw, bh, bd); glVertex3f(bw, bh, -bd);
            glNormal3f(0, -1, 0);
            glVertex3f(-bw, -bh, bd); glVertex3f(-bw, -bh, -bd); glVertex3f(bw, -bh, -bd); glVertex3f(bw, -bh, bd);
        glEnd();
        
        if (!isShadow) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texBillboard);
            glDisable(GL_LIGHTING); 
            glColor3f(1.0f, 1.0f, 1.0f); 
        }
        glBegin(GL_QUADS);
            glNormal3f(0, 0, 1);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-bw, -bh, bd);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( bw, -bh, bd);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( bw,  bh, bd);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-bw,  bh, bd);
        glEnd();
        
        if (!isShadow) {
            glEnable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
        }
        glPopMatrix();
    } 
    else if (obj.type == TYPE_TENT) {
        float tW = obj.scaleX * 0.5f, tH = obj.scaleY;        
        if (!isShadow) glColor3f(0.8f, 0.8f, 0.8f); 
        float corners[4][2] = {{tW, tW}, {-tW, tW}, {tW, -tW}, {-tW, -tW}};
        for(int k=0; k<4; k++) { glPushMatrix(); glTranslatef(corners[k][0], tH * 0.5f, corners[k][1]); glScalef(0.4f, tH, 0.4f); glutSolidCube(1.0); glPopMatrix(); }
        if (!isShadow) glColor3f(obj.r, obj.g, obj.b); 
        glPushMatrix(); glTranslatef(0, tH, 0); glRotatef(-90.0f, 1, 0, 0); glRotatef(45.0f, 0, 0, 1); glutSolidCone(obj.scaleX * 0.7f, obj.scaleY * 0.5f, 4, 1); glPopMatrix();
        
        int peopleInTent = 6;
        for(int p = 0; p < peopleInTent; p++) {
            float px = std::fmod((obj.x * 13.0f + p * 17.0f), tW * 1.5f) - (tW * 0.75f);
            float pz = std::fmod((obj.z * 11.0f + p * 23.0f), tW * 1.5f) - (tW * 0.75f);
            drawPersonAndFlash(px, 0.0f, pz, obj.x + p, timeMs, isShadow);
        }
    } 
    else if (obj.type == TYPE_CROWD) {
        drawPersonAndFlash(0, 0, 0, obj.x + obj.z, timeMs, isShadow);
    } 
    else if (obj.type == TYPE_GRANDSTAND) {
        int tiers = 5; float tierDepth = 2.0f, tierHeight = 1.5f, standWidth = 26.0f;    
        if (!isShadow) glColor3f(0.4f, 0.4f, 0.4f);
        glPushMatrix(); glTranslatef(0, (tiers * tierHeight) * 0.5f, -(tiers * tierDepth)); glScalef(standWidth + 1.0f, tiers * tierHeight, 0.5f); glutSolidCube(1.0); glPopMatrix();
        if (!isShadow) glColor3f(0.7f, 0.7f, 0.7f);
        glPushMatrix(); glTranslatef(0, tiers * tierHeight + 1.5f, -(tiers * tierDepth) * 0.5f); glRotatef(12.0f, 1, 0, 0); glScalef(standWidth + 2.0f, 0.4f, tiers * tierDepth + 5.0f); glutSolidCube(1.0); glPopMatrix();

        for (int t = 0; t < tiers; t++) {
            if (!isShadow) glColor3f(obj.r, obj.g, obj.b); 
            glPushMatrix(); glTranslatef(0, t * tierHeight + 0.5f, -t * tierDepth); glScalef(standWidth, 1.0f, tierDepth); glutSolidCube(1.0); glPopMatrix();
            int peopleCount = 14; float spacing = standWidth / peopleCount;
            for(int p = 0; p < peopleCount; p++) {
                float noiseX = std::fmod(obj.x + t * 3.0f + p * 7.0f, 0.8f) - 0.4f;
                float noiseZ = std::fmod(obj.z + t * 5.0f + p * 2.0f, 0.8f) - 0.4f;
                float px = -standWidth * 0.5f + (p + 0.5f) * spacing + noiseX;
                float pz = -t * tierDepth + noiseZ;
                drawPersonAndFlash(px, t * tierHeight + 1.0f, pz, obj.x + t + p, timeMs, isShadow);
            }
        }
    }
}

void RenderSystem::drawScenery() {
    float timeMs = glutGet(GLUT_ELAPSED_TIME);

    float lightPos[] = { car.pos.x + 100.0f, 400.0f, car.pos.z + 100.0f };
    float planeNormal[] = { 0.0f, 1.0f, 0.0f };
    float planePoint[] = { 0.0f, 0.05f, 0.0f }; 

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

        float dx = obj.x - camPos.x;
        float dz = obj.z - camPos.z;
        if((dx*dx + dz*dz) > 800000.0f) continue; 

        glPushMatrix();
            glShadowProjection(lightPos, planePoint, planeNormal);
            glTranslatef(obj.x, 0, obj.z);
            glRotatef(obj.rot, 0, 1, 0);
            
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.0f, 0.0f, 0.0f, 0.45f); 
            
            drawObjectGeometry(obj, true, timeMs);
            
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(obj.x, 0, obj.z);
            glRotatef(obj.rot, 0, 1, 0);
            drawObjectGeometry(obj, false, timeMs);
        glPopMatrix();
    }
}

void RenderSystem::drawTrack() {
    glDisable(GL_LIGHTING);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texGrass);
    glColor3f(1.0f, 1.0f, 1.0f); 
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);       glVertex3f(-6000, 0, -6000); 
        glTexCoord2f(1000.0f, 0.0f);    glVertex3f(6000, 0, -6000);
        glTexCoord2f(1000.0f, 1000.0f); glVertex3f(6000, 0, 6000); 
        glTexCoord2f(0.0f, 1000.0f);    glVertex3f(-6000, 0, 6000); 
    glEnd();
    glDisable(GL_TEXTURE_2D);

    for(size_t i=0; i<trackPoints.size(); i++) {
        Vec3 p1 = trackPoints[i], p2 = trackPoints[(i+1)%trackPoints.size()];
        Vec3 n = Vec3(-(p2-p1).z, 0, (p2-p1).x).normalize();
        
        if (i == 0 || i == 1) { 
            int cols = 24; 
            int rows = 3;  
            Vec3 dir = p2 - p1;
            
            glBegin(GL_QUADS);
            for(int r = 0; r < rows; r++) {
                float f0 = (float)r / rows;
                float f1 = (float)(r + 1) / rows;
                Vec3 row_p1_start = p1 + dir * f0;
                Vec3 row_p1_end   = p1 + dir * f1;

                for(int c = 0; c < cols; c++) {
                    if ((r + c + i) % 2 == 0) glColor3f(1.0f, 1.0f, 1.0f); 
                    else glColor3f(0.05f, 0.05f, 0.05f); 
                    
                    float w0 = -1.0f + (c * 2.0f / cols);
                    float w1 = -1.0f + ((c + 1) * 2.0f / cols);

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
            glColor3f(0.15f, 0.15f, 0.18f); 
            glBegin(GL_QUAD_STRIP);
                glVertex3f(p1.x-n.x*TRACK_WIDTH, 0.02f, p1.z-n.z*TRACK_WIDTH); 
                glVertex3f(p1.x+n.x*TRACK_WIDTH, 0.02f, p1.z+n.z*TRACK_WIDTH);
                glVertex3f(p2.x-n.x*TRACK_WIDTH, 0.02f, p2.z-n.z*TRACK_WIDTH); 
                glVertex3f(p2.x+n.x*TRACK_WIDTH, 0.02f, p2.z+n.z*TRACK_WIDTH); 
            glEnd();
        }

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
