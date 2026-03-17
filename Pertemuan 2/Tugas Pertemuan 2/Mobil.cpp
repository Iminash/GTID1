#include <GL/glut.h>
#include <math.h>

#define PI 3.14159265

void drawCircle(float x, float y, float radius) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i++) {
        float angle = i * PI / 180;
        glVertex2f(x + cos(angle) * radius, y + sin(angle) * radius);
    }
    glEnd();
}

void drawBird(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glLineWidth(2.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);
        glVertex2f(-0.03f, 0.02f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(0.03f, 0.02f);
    glEnd();
    glPopMatrix();
}

void drawTree(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    
    glColor3f(0.4f, 0.2f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-0.02f, 0.0f);
        glVertex2f(0.02f, 0.0f);
        glVertex2f(0.03f, 0.4f);
        glVertex2f(-0.03f, 0.4f);
    glEnd();

    glTranslatef(0.0f, 0.4f, 0.0f);
    glColor3f(0.0f, 0.5f, 0.0f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, 0.0f); glVertex2f(-0.15f, 0.05f); glVertex2f(-0.1f, -0.1f);
        glVertex2f(0.0f, 0.0f); glVertex2f(0.15f, 0.05f); glVertex2f(0.1f, -0.1f);
        glVertex2f(0.0f, 0.0f); glVertex2f(-0.05f, 0.15f); glVertex2f(0.05f, 0.15f);
        glVertex2f(0.0f, 0.0f); glVertex2f(-0.12f, 0.12f); glVertex2f(-0.18f, 0.0f);
        glVertex2f(0.0f, 0.0f); glVertex2f(0.12f, 0.12f); glVertex2f(0.18f, 0.0f);
    glEnd();
    
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
        glColor3f(0.5f, 0.8f, 1.0f);
        glVertex2f(-1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glVertex2f(1.0f, 0.3f);  glVertex2f(-1.0f, 0.3f);

        glColor3f(0.0f, 0.4f, 0.7f);
        glVertex2f(-1.0f, 0.3f);  glVertex2f(1.0f, 0.3f);
        glVertex2f(1.0f, -0.1f); glVertex2f(-1.0f, -0.1f);

        glColor3f(0.9f, 0.8f, 0.5f);
        glVertex2f(-1.0f, -0.1f); glVertex2f(1.0f, -0.1f);
        glVertex2f(1.0f, -1.0f); glVertex2f(-1.0f, -1.0f);
    glEnd();

    glColor3f(1.0f, 0.9f, 0.0f);
    drawCircle(0.7f, 0.7f, 0.15f);

    drawBird(-0.5f, 0.7f);
    drawBird(-0.3f, 0.8f);
    drawBird(-0.6f, 0.85f);

    drawTree(-0.7f, -0.2f);
    drawTree(0.8f, -0.25f);

    glPushMatrix();
        glTranslatef(-0.1f, -0.5f, 0.0f);

        glColor3f(0.8f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
            glVertex2f(-0.4f, 0.0f);
            glVertex2f(0.4f, 0.0f);
            glVertex2f(0.4f, 0.2f);
            glVertex2f(-0.4f, 0.2f);
        glEnd();

        glColor3f(0.6f, 0.0f, 0.0f);
        glBegin(GL_POLYGON);
            glVertex2f(-0.2f, 0.2f);
            glVertex2f(0.2f, 0.2f);
            glVertex2f(0.15f, 0.35f);
            glVertex2f(-0.15f, 0.35f);
        glEnd();

        glColor3f(0.8f, 0.9f, 1.0f);
        glBegin(GL_QUADS);
            glVertex2f(-0.12f, 0.23f);
            glVertex2f(0.12f, 0.23f);
            glVertex2f(0.1f, 0.32f);
            glVertex2f(-0.1f, 0.32f);
        glEnd();

        glPushMatrix();
            glTranslatef(-0.22f, 0.0f, 0.0f);
            glColor3f(0.1f, 0.1f, 0.1f);
            drawCircle(0.0f, 0.0f, 0.08f);
            glColor3f(0.5f, 0.5f, 0.5f);
            drawCircle(0.0f, 0.0f, 0.03f);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(0.22f, 0.0f, 0.0f);
            glColor3f(0.1f, 0.1f, 0.1f);
            drawCircle(0.0f, 0.0f, 0.08f);
            glColor3f(0.5f, 0.5f, 0.5f);
            drawCircle(0.0f, 0.0f, 0.03f);
        glPopMatrix();
    glPopMatrix();

    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Mobil");
    glutDisplayFunc(display);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glutMainLoop();
    return 0;
}
