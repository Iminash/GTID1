#include <GL/glut.h>
#include <stdio.h> // Untuk printf

// Variabel untuk mengontrol sendi lengan
static int shoulder = 0, elbow = 0, wrist = 0;

// Variabel ARRAY untuk mengontrol sendi 3 jari
// Indeks 0 = Jari Telunjuk
// Indeks 1 = Jari Tengah
// Indeks 2 = Jari Manis
static int fingerBase[3] = {0}; 
static int fingerMid[3] = {0};

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
}

// Fungsi untuk menggambar satu jari
void drawFinger(float baseAngle, float midAngle) {
    glPushMatrix();
    glRotatef(baseAngle, 0.0, 0.0, 1.0);
    glTranslatef(0.25f, 0.0f, 0.0f);
    glPushMatrix();
        glScalef(0.5f, 0.15f, 0.5f);
        glutWireCube(1.0);
    glPopMatrix();

    glTranslatef(0.25f, 0.0f, 0.0f);
    glRotatef(midAngle, 0.0, 0.0, 1.0);
    glTranslatef(0.2f, 0.0f, 0.0f);
    glPushMatrix();
        glScalef(0.4f, 0.15f, 0.5f);
        glutWireCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();

    // --- Lengan Atas (Shoulder) ---
    glTranslatef(-2.0, 0.0, 0.0);
    glRotatef((GLfloat)shoulder, 0.0, 0.0, 1.0);
    glTranslatef(1.0, 0.0, 0.0);
    glPushMatrix();
        glScalef(2.0, 0.6, 1.0);
        glutWireCube(1.0);
    glPopMatrix();

    // --- Lengan Bawah (Elbow) ---
    glTranslatef(1.0, 0.0, 0.0);
    glRotatef((GLfloat)elbow, 0.0, 0.0, 1.0);
    glTranslatef(1.0, 0.0, 0.0);
    glPushMatrix();
        glScalef(2.0, 0.6, 1.0);
        glutWireCube(1.0);
    glPopMatrix();

    // --- Telapak Tangan (Wrist & Palm) ---
    glTranslatef(1.0, 0.0, 0.0);
    glRotatef((GLfloat)wrist, 0.0, 0.0, 1.0);
    glTranslatef(0.4f, 0.0f, 0.0f);
    glPushMatrix();
        glScalef(0.8f, 0.8f, 1.0f);
        glutWireCube(1.0);
    glPopMatrix();

    // --- Jari-Jari  ---
    glPushMatrix();
        // Jari Telunjuk (Indeks 0)
        glPushMatrix();
            glTranslatef(0.4f, 0.3f, 0.0f);
            drawFinger(fingerBase[0], fingerMid[0]);
        glPopMatrix();

        // Jari Tengah (Indeks 1)
        glPushMatrix();
            glTranslatef(0.4f, 0.0f, 0.0f);
            drawFinger(fingerBase[1], fingerMid[1]);
        glPopMatrix();

        // Jari Manis (Indeks 2)
        glPushMatrix();
            glTranslatef(0.4f, -0.3f, 0.0f);
            drawFinger(fingerBase[2], fingerMid[2]);
        glPopMatrix();
    glPopMatrix();

    glPopMatrix();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, (GLfloat)w / (GLfloat)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -8.0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        // Kontrol Lengan
        case 's': shoulder = (shoulder + 5) % 360; break;
        case 'S': shoulder = (shoulder - 5) % 360; break;
        case 'e': elbow = (elbow + 5) % 360; break;
        case 'E': elbow = (elbow - 5) % 360; break;
        case 'w': wrist = (wrist + 5) % 360; break;
        case 'W': wrist = (wrist - 5) % 360; break;

        // Jari Telunjuk
        case '1': fingerBase[0] = (fingerBase[0] + 5) % 360; break;
        case '!': fingerBase[0] = (fingerBase[0] - 5) % 360; break;
        case '2': fingerMid[0] = (fingerMid[0] + 5) % 360; break;
        case '@': fingerMid[0] = (fingerMid[0] - 5) % 360; break;

        // Jari Tengah
        case '3': fingerBase[1] = (fingerBase[1] + 5) % 360; break;
        case '#': fingerBase[1] = (fingerBase[1] - 5) % 360; break;
        case '4': fingerMid[1] = (fingerMid[1] + 5) % 360; break;
        case '$': fingerMid[1] = (fingerMid[1] - 5) % 360; break;

        // Jari Manis
        case '5': fingerBase[2] = (fingerBase[2] + 5) % 360; break;
        case '%': fingerBase[2] = (fingerBase[2] - 5) % 360; break;
        case '6': fingerMid[2] = (fingerMid[2] + 5) % 360; break;
        case '^': fingerMid[2] = (fingerMid[2] - 5) % 360; break;

        // Keluar
        case 27: exit(0); break;
        default: break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tangan");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    
    glutMainLoop();
    return 0;
}
