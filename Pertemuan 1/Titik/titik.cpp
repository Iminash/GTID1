#include <GL/glut.h>

void point(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(5.0f);
    glBegin(GL_POINTS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.25f, 0.25f, 0.0f);
    glEnd();
    glFlush();
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(640, 480);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutCreateWindow("Titik");
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glutDisplayFunc(point);
    glutMainLoop();
    return 0;
}
