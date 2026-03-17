#include <GL/glut.h>

static int shoulder = 0, elbow = 0;

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix(); // Simpan matriks awal

    // --- Lengan Atas (Shoulder) ---
    // Pindah ke titik pivot bahu
    glTranslatef(-1.5, 0.0, 0.0);
    // Rotasi bahu
    glRotatef((GLfloat)shoulder, 0.0, 0.0, 1.0);
    // Pindah sepanjang lengan atas untuk menggambar
    glTranslatef(1.0, 0.0, 0.0);
    
    glPushMatrix(); // Simpan matriks lengan atas
        // Skala untuk membentuk balok lengan atas
        glScalef(2.0, 0.4, 1.0);
        glutWireCube(1.0);
    glPopMatrix(); // Kembalikan matriks setelah scaling

    // --- Lengan Bawah (Elbow) ---
    // Pindah ke ujung lengan atas (titik pivot siku)
    glTranslatef(1.0, 0.0, 0.0);
    // Rotasi siku
    glRotatef((GLfloat)elbow, 0.0, 0.0, 1.0);
    // Pindah sepanjang lengan bawah untuk menggambar
    glTranslatef(1.0, 0.0, 0.0);

    glPushMatrix(); // Simpan matriks lengan bawah
        // Skala untuk membentuk balok lengan bawah
        glScalef(2.0, 0.4, 1.0);
        glutWireCube(1.0);
    glPopMatrix(); // Kembalikan matriks setelah scaling

    glPopMatrix(); // Kembalikan matriks awal
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, (GLfloat)w / (GLfloat)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -5.0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 's':
            shoulder = (shoulder + 5) % 360;
            break;
        case 'S':
            shoulder = (shoulder - 5) % 360;
            break;
        case 'e':
            elbow = (elbow + 5) % 360;
            break;
        case 'E':
            elbow = (elbow - 5) % 360;
            break;
        case 27: // Tombol Escape
            exit(0);
            break;
        default:
            break;
    }
    glutPostRedisplay(); // Minta gambar ulang setelah input
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(700, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Lengan Bergerak");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    
    glutMainLoop();
    return 0;
}
