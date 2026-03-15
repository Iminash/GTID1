#include <GL/glut.h>

void SegiEmpat(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glTranslatef(-0.50f, 0.50f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glRectf(-0.18f, 0.18f, 0.18f, -0.18f);
    glFlush();
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(640, 480);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutCreateWindow("Segi Empat");
    glutDisplayFunc(SegiEmpat);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glutMainLoop();
    return 0;
}
