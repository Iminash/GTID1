#include <GL/glut.h>

void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity(); // Reset Modelview Matrix

    // Menggambar poligon
    glBegin(GL_POLYGON);
        glColor3f(0.0, 0.0, 0.0); // Hitam
        glVertex3f(-0.5, -0.5, -3.0);
        
        glColor3f(1.0, 0.0, 0.0); // Merah
        glVertex3f(0.5, -0.5, -3.0);
        
        glColor3f(0.0, 0.0, 1.0); // Biru
        glVertex3f(0.5, 0.5, -3.0);
    glEnd();

    glFlush(); // Selesai rendering
}

void Reshape(int x, int y) {
    // Mencegah pembagian dengan nol jika jendela terlalu kecil
    if (y == 0 || x == 0) return;

    // Mengatur proyeksi matrix baru
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Mengatur perspektif
    // Sudut pandang: 40 derajat
    // Aspek rasio: lebar/tinggi
    // Jarak potong terdekat: 0.5
    // Jarak potong terjauh: 20.0
    gluPerspective(40.0, (GLdouble)x / (GLdouble)y, 0.5, 20.0);
    
    // Kembali ke Modelview Matrix
    glMatrixMode(GL_MODELVIEW);
    
    // Menggunakan seluruh window untuk rendering
    glViewport(0, 0, x, y);
}

int main(int argc, char **argv) {
    // Inisialisasi GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(300, 300);
    glutCreateWindow("Intro");
    
    glClearColor(1.0, 1.0, 1.0, 1.0); // Background putih agar poligon terlihat
    
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape); // Mengatur kembali bentuk objek saat window diubah ukurannya
    
    glutMainLoop();
    return 0;
}
