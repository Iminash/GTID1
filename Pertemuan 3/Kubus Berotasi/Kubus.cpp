#include <GL/glut.h>

GLfloat xRotated, yRotated, zRotated;

void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    // Mundurkan kubus agar terlihat
    glTranslatef(0.0, 0.0, -4.0);
    
    // Rotasi kubus berdasarkan variabel global
    glRotatef(xRotated, 1.0, 0.0, 0.0); // Rotasi pada sumbu X
    glRotatef(yRotated, 0.0, 1.0, 0.0); // Rotasi pada sumbu Y
    glRotatef(zRotated, 0.0, 0.0, 1.0); // Rotasi pada sumbu Z
    
    // Skala kubus (membuatnya lebih lebar)
    glScalef(2.0, 1.0, 1.0);
    
    // Gambar kubus kerangka
    glutWireCube(1.0);
    
    // Tukar buffer untuk menampilkan gambar
    glutSwapBuffers();
}

void Reshape(int x, int y) {
    if (y == 0 || x == 0) return;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, (GLdouble)x / (GLdouble)y, 0.5, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, x, y);
}

// Fungsi Idle untuk animasi
void Idle(void) {
    // Tambah nilai rotasi secara terus-menerus
    xRotated += 0.3;
    yRotated += 0.1;
    zRotated += -0.4;
    
    // Panggil fungsi display untuk menggambar ulang
    // Di sini pemanggilan Display() secara langsung lebih efektif daripada glutPostRedisplay()
    Display();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(300, 300);
    glutCreateWindow("Cube example");
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // Inisialisasi sudut rotasi
    xRotated = yRotated = zRotated = 0.0;
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutIdleFunc(Idle); // Daftarkan fungsi Idle
    
    glutMainLoop();
    return 0;
}
