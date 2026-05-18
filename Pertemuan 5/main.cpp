#include <stdlib.h> // standard definitions 
#include <stdio.h> // C I/O (for sprintf) 
#include <math.h> // standard definitions 
#include <GL/glut.h> // GLUT 

double rotAngle = 10; // rotation angle (BEWARE: Global) 
double rotAngle1 = 10; // rotation angle (BEWARE: Global) 

void init() 
{ 
 glClearColor(0, 0, 0, 0); // background color 
 glClearDepth(1.0); // background depth value 
 glMatrixMode(GL_PROJECTION); 
 glLoadIdentity(); 
 gluPerspective(60, 1, 1, 1000); // setup a perspective projection 
 glMatrixMode(GL_MODELVIEW); 
 glLoadIdentity(); 
 gluLookAt( // set up the camera 
 0.0, 0.0, 5.0, // eye position 
 0.0, 0.0, 0.0, // lookat position 
 0.0, 1.0, 0.0); // up direction 
} 

void display() 
{ 
 glClear( 
 GL_COLOR_BUFFER_BIT | // clear the frame buffer (color) 
 GL_DEPTH_BUFFER_BIT); // clear the depth buffer (depths) 
 glPushMatrix(); // save the current camera transform 
 glRotated(rotAngle, 0, 1, 0); // rotate by rotAngle about y-axis 
 glRotated(rotAngle1, 1, 0, 0); // rotate by rotAngle about y axis 
 glEnable(GL_COLOR_MATERIAL); // specify object color 
 glColor3f(1.0, 0.1, 0.1); // redish 
 glutSolidTeapot(1); // draw the teapot 
 glPopMatrix(); // restore the modelview matrix 
 glFlush(); // force OpenGL to render now 
 glutSwapBuffers(); // make the image visible 
} 

void keyboard(unsigned char k, int x, int y) 
{ 
 switch (k) 
 { 
 case 'a': 
 rotAngle += 5; // increase rotation by 5 degrees 
 break; 
 case 'y': 
 rotAngle1 += 5; // increase rotation by 5 degrees 
 break; 
 case 'b': 
 rotAngle1 -= 5; // increase rotation by 5 degrees 
 break; 
 case 'l': 
 rotAngle -= 5; // decrease rotation by 5 degrees 
 break; 
 case 'q': 
 exit(0); // exit 
 } 
 glutPostRedisplay(); // redraw the image now 
} 

int main() 
{ 
 glutInitDisplayMode( // initialize GLUT 
 GLUT_DOUBLE | // use double buffering 
 GLUT_DEPTH | // request memory for z-buffer 
 GLUT_RGB ); // set RGB color mode 
 glutCreateWindow("GLUT Example"); // create the window 
 glutDisplayFunc(display); // call display() to redraw window 
 glutKeyboardFunc(keyboard); // call keyboard() when key is hit 
 init(); // our own initializations 
 glutMainLoop(); // let GLUT take care of everything 
 return 0; 
}
