#include <GL/glut.h>

void Jendela(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    glFlush();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    
    glutInitWindowSize(640, 480);
    
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    
    glutCreateWindow("biru");
    
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    
    glutDisplayFunc(Jendela);
    
    glutMainLoop();
    
    return 0;
}
