#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --- Variabel Kamera & Pergerakan ---
float x = 0.0f, y = 1.75f, z = 15.0f; 
float lx = 0.0f, lz = -1.0f;          
float angle = 0.0f;                   
float moveSpeed = 0.4f;               
float turnSpeed = 0.05f;              

// --- Variabel Animasi ---
float birdX = -20.0f;
float cloudOffset = 0.0f;

// --- Warna Kabut (Sesuai warna langit) ---
GLfloat fogColor[] = {0.5f, 0.8f, 1.0f, 1.0f};

void init() {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Warna Langit
    glEnable(GL_DEPTH_TEST);

    // 1. PENGATURAN CAHAYA (Poin 3: Specular)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    GLfloat light_pos[] = { 10.0f, 20.0f, 10.0f, 1.0f };
    GLfloat white_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lmodel_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light); // Efek mengkilap
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    // 2. PENGATURAN KABUT/FOG (Poin 3)
    glEnable(GL_FOG);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_EXP2); // Kabut semakin tebal di kejauhan
    glFogf(GL_FOG_DENSITY, 0.02f); // Ketebalan kabut
    glHint(GL_FOG_HINT, GL_NICEST);
}

// Fungsi bantu menampilkan teks di layar (Poin 5)
void drawText(float x, float y, const char *string) {
    glRasterPos2f(x, y);
    for (int i = 0; i < strlen(string); i++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, string[i]);
    }
}

// Fungsi menggambar Awan (Poin 1)
void drawCloud(float cx, float cy, float cz) {
    glPushMatrix();
    glTranslatef(cx + cloudOffset, cy, cz);
    glColor3f(1.0f, 1.0f, 1.0f);
    glScalef(1.5f, 0.5f, 1.0f);
    glutSolidSphere(2.0f, 20, 20);
    glPopMatrix();
}

// Fungsi menggambar Gunung (Poin 1)
void drawMountain(float mx, float mz, float h) {
    glPushMatrix();
    glTranslatef(mx, 0.0f, mz);
    glRotatef(-90, 1, 0, 0);
    glColor3f(0.4f, 0.4f, 0.4f); // Abu-abu
    glutSolidCone(15.0, h, 20, 10);
    glPopMatrix();
}

// Fungsi menggambar Batu (Poin 1)
void drawRock(float rx, float rz) {
    glPushMatrix();
    glTranslatef(rx, 0.2f, rz);
    glColor3f(0.5f, 0.5f, 0.5f);
    glScalef(1.2f, 0.8f, 1.2f);
    glutSolidDodecahedron(); // Bentuk batu tidak beraturan
    glPopMatrix();
}

void drawTree(float tx, float tz) {
    glPushMatrix();
    glTranslatef(tx, 0.0f, tz);
    // Batang
    glColor3f(0.4f, 0.25f, 0.1f);
    glPushMatrix();
    glScalef(0.6f, 2.5f, 0.6f);
    glTranslatef(0.0f, 0.5f, 0.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    // Daun
    glColor3f(0.1f, 0.4f, 0.1f);
    glTranslatef(0.0f, 2.0f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(1.8f, 4.0f, 16, 8);
    glPopMatrix();
}

// Fungsi menggambar HUD/UI 2D (Poin 5)
void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600); // Set koordinat layar 2D
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING); // Matikan lighting agar teks jelas
    glColor3f(0.0f, 0.0f, 0.0f); // Teks Hitam
    
    char buf[100];
    sprintf(buf, "Posisi: X:%.1f Z:%.1f", x, z);
    drawText(20, 560, buf);
    
    // Indikator Kompas Sederhana
    float deg = angle * 180 / 3.14159;
    sprintf(buf, "Arah: %.0f Deg", (float)((int)deg % 360));
    drawText(20, 540, buf);
    
    drawText(20, 20, "Kontrol: W,A,S,D untuk Bergerak | ESC untuk Keluar");

    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(x, y, z, x + lx, y, z + lz, 0.0f, 1.0f, 0.0f);

    // 1. Rumput & Elemen Lingkungan
    glColor3f(0.3f, 0.8f, 0.3f);
    glBegin(GL_QUADS);
        glVertex3f(-150.0f, 0.0f, -150.0f); glVertex3f(-150.0f, 0.0f, 150.0f);
        glVertex3f(150.0f, 0.0f, 150.0f); glVertex3f(150.0f, 0.0f, -150.0f);
    glEnd();

    // Gunung di kejauhan
    drawMountain(-50, -80, 40);
    drawMountain(60, -90, 55);
    drawMountain(0, -100, 30);

    // Awan & Batu
    drawCloud(-15, 15, -40);
    drawCloud(20, 18, -60);
    drawRock(5, 5); drawRock(-10, 12); drawRock(20, -5);

    // Pohon
    for(int i = -3; i <= 3; i++) {
        for(int j = -3; j <= 3; j++) {
            if(abs(i) > 1 || abs(j) > 1) drawTree(i*25.0f, j*25.0f);
        }
    }

    // Tampilkan HUD 2D
    drawHUD();

    glutSwapBuffers();
}

void processKeys(unsigned char key, int xx, int yy) {
    switch (key) {
        case 'a': angle -= turnSpeed; break;
        case 'd': angle += turnSpeed; break;
        case 'w': x += lx * moveSpeed; z += lz * moveSpeed; break;
        case 's': x -= lx * moveSpeed; z -= lz * moveSpeed; break;
        case 27: exit(0); break;
    }
    lx = sin(angle);
    lz = -cos(angle);
    glutPostRedisplay();
}

void update(int value) {
    cloudOffset += 0.02f; // Gerakan awan lambat
    if (cloudOffset > 100.0f) cloudOffset = -100.0f;
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w/h, 0.1f, 500.0f);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Simulasi Dunia Advanced - GTI");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(processKeys);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
