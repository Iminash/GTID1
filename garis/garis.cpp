#include <GL/glut.h>

void Garis(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.00f, 0.20f, 0.0f);
    glVertex3f(0.00f, -0.20f, 0.0f);
    glEnd();
    glFlush();
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(640, 480);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutCreateWindow("Membuat Garis");
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glutDisplayFunc(Garis);
    glutMainLoop();
    return 0;
}
