#include <GL/glut.h>

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
    
    // Baris Bawah - Kubus 1 (Merah)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(-0.55f, -0.6f);
    glVertex2f(-0.25f, -0.6f);
    glVertex2f(-0.25f, -0.3f);
    glVertex2f(-0.55f, -0.3f);

    // Baris Bawah - Kubus 2 (Hijau)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(-0.15f, -0.6f);
    glVertex2f(0.15f, -0.6f);
    glVertex2f(0.15f, -0.3f);
    glVertex2f(-0.15f, -0.3f);

    // Baris Bawah - Kubus 3 (Kuning)
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex2f(0.25f, -0.6f);
    glVertex2f(0.55f, -0.6f);
    glVertex2f(0.55f, -0.3f);
    glVertex2f(0.25f, -0.3f);

    // Baris Atas - Kubus 4 (Magenta)
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex2f(-0.35f, -0.25f);
    glVertex2f(-0.05f, -0.25f);
    glVertex2f(-0.05f, 0.05f);
    glVertex2f(-0.35f, 0.05f);

    // Baris Atas - Kubus 5 (Cyan)
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex2f(0.05f, -0.25f);
    glVertex2f(0.35f, -0.25f);
    glVertex2f(0.35f, 0.05f);
    glVertex2f(0.05f, 0.05f);

    glEnd();
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(640, 480);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutCreateWindow("Kubus Bertingkat Warna-Warni");
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
