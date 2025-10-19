
/* A framework for writing simple 3D applications with GLUT, with
 * support for animation, for mouse and keyboard events, and for a
 * menu.  Note that you need to uncomment some lines in main() and
 * initGL() to enable various features.  Drawing code must be
 * added to display().  See lines marked with "TODO".  See the
 * GLUT documentation at 
 *    www.opengl.org/resources/libraries/glut/spec3/node1.html
 * or the FreeGLUT documentation at
 *    freeglut.sourceforge.net/docs/api.php
 *
 *    This program must be linked to the GL and glut libraries.  
 * For example, in Linux with the gcc compiler:
 *
 *        gcc -o executableProg glut-starter.c -lGL -lglut
 */

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>   // Consider using freeglut.h instead, if available.
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include <stdio.h>     // (Used only for some information messages to standard out.)
#include <stdlib.h>    // (Used only for exit() function.)
#include <math.h>
#include <jpeglib.h>
#include <jerror.h>

#define DEBUG 1
#define ARC_INDICES 37

struct imgRawImage {
    unsigned int numComponents;
    unsigned long int width, height;
    unsigned char* lpData;
};

// --------------------------------- global variables --------------------------------

int width, height;   // Size of the drawing area, to be set in reshape().

int frameNumber = 0;     // For use in animation.
int roll = 0;
int pitch = 0;

GLuint texture[2];
GLfloat vertices[ARC_INDICES][2];


struct imgRawImage* loadJpegImageFile(char* lpFilename) {
    struct jpeg_decompress_struct info;
    struct jpeg_error_mgr err;

    struct imgRawImage* lpNewImage;

    unsigned long int imgWidth, imgHeight;
    int numComponents;

    unsigned long int dwBufferBytes;
    unsigned char* lpData;

    unsigned char* lpRowBuffer[1];

    FILE* fHandle;

    fHandle = fopen(lpFilename, "rb");
    if(fHandle == NULL) {
        #ifdef DEBUG
            fprintf(stderr, "%s:%u: Failed to read file %s\n", __FILE__, __LINE__, lpFilename);
        #endif
        return NULL; /* ToDo */
    }

    info.err = jpeg_std_error(&err);
    jpeg_create_decompress(&info);

    jpeg_stdio_src(&info, fHandle);
    jpeg_read_header(&info, TRUE);

    jpeg_start_decompress(&info);
    imgWidth = info.output_width;
    imgHeight = info.output_height;
    numComponents = info.num_components;

    #ifdef DEBUG
    fprintf(
        stderr,
        "%s:%u: Reading JPEG with dimensions %lu x %lu and %u components\n",
        __FILE__, __LINE__,
        imgWidth, imgHeight, numComponents
    );
    #endif

    dwBufferBytes = imgWidth * imgHeight * 3; /* We only read RGB, not A */
    if ((lpData = (unsigned char*)malloc(sizeof(unsigned char)*dwBufferBytes)) == NULL) {
        #ifdef DEBUG
        fprintf(stderr, "%s:%u: Allocation of lpData failed\n", __FILE__, __LINE__);
        #endif
        return NULL;
    }

    if ((lpNewImage = (struct imgRawImage*)malloc(sizeof(struct imgRawImage))) == NULL) {
        #ifdef DEBUG
        fprintf(stderr, "%s:%u: Allocation of lpNewImage failed\n", __FILE__, __LINE__);
        #endif
        return NULL;
    }

    lpNewImage->numComponents = numComponents;
    lpNewImage->width = imgWidth;
    lpNewImage->height = imgHeight;
    lpNewImage->lpData = lpData;

    /* Read scanline by scanline */
    while(info.output_scanline < info.output_height) {
        lpRowBuffer[0] = (unsigned char *)(&lpData[3*info.output_width*info.output_scanline]);
        jpeg_read_scanlines(&info, lpRowBuffer, 1);
    }

    jpeg_finish_decompress(&info);
    jpeg_destroy_decompress(&info);
    fclose(fHandle);

    return lpNewImage;
}

void LoadGLTextures() {
    struct imgRawImage *image, *image2;

    image = loadJpegImageFile("./sphere.jpg");

    // Create Texture Name and Bind it as current
    glGenTextures(2, &texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[0]);   // 2d texture (x and y size)

    // Set Texture Parameters
    //  Scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //  Scale linearly when image smaller than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    // Load texture into OpenGL RC
    glTexImage2D(GL_TEXTURE_2D,     // 2D texture
        0,                  // level of detail 0 (normal)
        3,	            // 3 color components
        image->width,       // x size from image
        image->height,      // y size from image
        0,	            // border 0 (normal)
        GL_RGB,             // rgb color data order
        GL_UNSIGNED_BYTE,   // color component types
        image->lpData       // image data itself
    );

    glEnable(GL_TEXTURE_2D);

    image2 = loadJpegImageFile("./ring.jpg");
    glBindTexture(GL_TEXTURE_2D, texture[1]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D,     // 2D texture
        0,                  // level of detail 0 (normal)
        3,	            // 3 color components
        image2->width,      // x size from image
        image2->height,     // y size from image
        0,	            // border 0 (normal)
        GL_RGB,             // rgb color data order
        GL_UNSIGNED_BYTE,   // color component types
        image2->lpData      // image data itself
    );

    glEnable(GL_TEXTURE_2D);
}

// ------------------------ OpenGL initialization and rendering -----------------------

/* initGL() is called just once, by main(), to do initialization of OpenGL state
 * and other global state.
 */
void initGL() {
      
    glClearColor(0.0, 0.0, 0.0, 1.0); // background color

    glEnable(GL_DEPTH_TEST);  // Required for 3D drawing, not usually for 2D.
    
    // TODO: Uncomment the following 4 lines to do some typical initialization for 
    // lighting and materials.

    glEnable(GL_LIGHTING);        // Enable lighting.
    glEnable(GL_LIGHT0);          // Turn on a light.  By default, shines from direction of viewer.
    glEnable(GL_NORMALIZE);       // OpenGL will make all normal vectors into unit normals
    glEnable(GL_COLOR_MATERIAL);  // Material ambient and diffuse colors can be set by glColor*

    GLfloat position[] = {0.0f, 0.0f, 2.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    GLfloat colorWhite[] = { 1.00, 1.00, 1.00, 1.0 };
    GLfloat colorDarkGray[] = { 0.10, 0.10, 0.10, 1.0 };
    GLfloat colorLightGray[] = { 0.75, 0.75, 0.75, 1.0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, colorDarkGray);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, colorLightGray);
    glLightfv(GL_LIGHT0, GL_SPECULAR, colorWhite);

    LoadGLTextures();

    // Generate arc vertices
    for (int i = 0; i < ARC_INDICES; i++) {
        double rad = (i / (double)(ARC_INDICES - 1)) * M_PI;
        vertices[i][0] = cos(rad) * -0.25f;
        vertices[i][1] = sin(rad) * -0.25f;
    }

}  // end initGL()

void setlight(){
    //here you set the lights and parameters, example with one light
    float LightAmbient[] = { 0.1f, 0.1f, 0.05f, 1.0f };
    float LightEmission[] = { 1.0f, 1.0f, 0.8f, 1.0f };
    float LightDiffuse[] = { 1.0f, 1.0f, 0.8f, 1.0f };
    float LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float LightDirection[]={-0.5f, -0.5f, -0.5f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
    //glLightfv(GL_LIGHT0, GL_POSITION, LightDirection);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void setmaterial(){
    //here you set materials, you must declare each one of the colors global or locally like this:
    float MatAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    float MatDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float MatSpecular[] = { 0.1f, 0.1f, 0.0f, 0.1f };
    float MatShininess = 60;
    float black[] = {0.0f,0.0f,0.0f,1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MatAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MatDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, MatSpecular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, MatShininess);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
}

/* display() is set up in main() as the function that is called when the window is
 * first opened, when glutPostRedisplay() is called, and possibly at other times when
 * the window needs to be redrawn.  Usually it will redraw the entire contents of
 * the window.  (Drawing can also be done in other functions, but usually only when
 * single buffer mode is used.  The projection is often set up in intiGL() or reshape()
 * instead of in display().)
 */
void display() {
        // called whenever the display needs to be redrawn

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // For 2D, usually leave out the depth buffer.
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // TODO: INSERT DRAWING CODE HERE

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTranslatef(0.0f, 0.0f, 0.0f);
    glRotatef(roll + 90, 0.0f, 0.0f, 1.0f);
    glRotatef(pitch, 0.0f, 1.0f, 0.0f);
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    GLUquadric *sphere = gluNewQuadric();
    gluQuadricDrawStyle(sphere, GLU_FILL);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluQuadricTexture(sphere, GL_TRUE);
    gluSphere(sphere, 0.9f, 36, 36);
    gluDeleteQuadric(sphere);
    glPopMatrix();

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTranslatef(0.0f, 0.0f, -0.7f);
    glRotatef(roll, 0.0f, 0.0f, 1.0f);
    GLUquadric *ring = gluNewQuadric();
    gluQuadricDrawStyle(ring, GLU_FILL);
    gluQuadricNormals(ring, GLU_SMOOTH);
    gluQuadricTexture(ring, GL_TRUE);
    gluDisk(ring, 0.8f, 1.0f, 72, 10);
    gluDeleteQuadric(ring);
    glPopMatrix();

    glPushMatrix();
    glDisable(GL_TEXTURE_2D);
    glTranslatef(0.0f, 0.0f, -0.9f);
    glLineWidth(4);
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.8f, 0.0f, 0.0f);
    glVertex3f(-0.15f, 0.0f, 0.0f);
    glVertex3f(-0.1f, 0.1f, 0.0f);
    glVertex3f(0.1f, 0.1f, 0.0f);
    glVertex3f(0.15f, 0.0f, 0.0f);
    glVertex3f(0.8f, 0.0f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < ARC_INDICES; i++) {
        glVertex3f(vertices[i][0], vertices[i][1], 0.0f);
    }
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, 0.1f, 0.0f);
    glVertex3f(0.0f, 0.8f, 0.0f);
    glVertex3f(0.3f, 0.2f, 0.0f);
    glVertex3f(0.0f, 0.2f, 0.0f);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();

    glFlush();

    glutSwapBuffers();  // (Required for double-buffered drawing.)
                        // (For GLUT_SINGLE display mode, use glFlush() instead.)
}


/* reshape() is set up in main() as the functin that is called when the window changes size.
 * It is also called when the window is first opened.  The parameters give the size of
 * the drawing area.  If no reshape function is provided, the default is to set the
 * viewport to match the size of the drawing area.
 */
void reshape(int w, int h) {
    width = w;   // Save width and height for possible use elsewhere.
    height = h;
    glViewport(0,0,width,height);  // If you have a reshape function, you MUST call glViewport!
    // TODO: INSERT ANY OTHER CODE TO ACCOUNT FOR WINDOW SIZE (maybe set projection here).
    printf("Reshaped to width %d, height %d\n", width, height);
}


// --------------- support for animation ------------------------------------------

/* You can call startAnimation() to run an animation.  A frame will be drawn every
 * 30 milliseconds (can be changed in the call to glutTimerFunc.  The global frameNumber
 * variable will be incremented for each frame.  Call pauseAnimation() to stop animating.
 */

int animating = 0;      // 0 or 1 to indicate whether an animation is in progress;
                        // do not change directly; call startAnimation() and pauseAnimation()

void updateFrame() {
      // this is called before each frame of the animation.
   // TODO: INSERT CODE TO UPDATE DATA USED IN DRAWING A FRAME
   frameNumber++;
   printf("frame number %d\n", frameNumber);
}

void timerFunction(int timerID) {
      // used for animation; do not call this directly
    if (animating) {
        updateFrame();
        glutTimerFunc(30, timerFunction, 0);  // Next frame in 30 milliseconds.
        glutPostRedisplay(); // Causes display() to be called.
    }
}

void startAnimation() {
      // call this to start or restart the animation
   if ( ! animating ) {
       animating = 1;
       glutTimerFunc(30, timerFunction, 0);
   }
}

void pauseAnimation() {
       // call this to pause the animation
    animating = 0;
}


// --------------- keyboard event functions ---------------------------------------

/* charTyped() is set up in main() to be called when the user types a character.
 * The ch parameter is an actual character such as 'A', '7', or '@'.  The parameters
 * x and y give the mouse position, in pixel coordinates, when the character was typed,
 * with (0,0) at the UPPER LEFT.
 */
void charTyped(unsigned char ch, int x, int y) {
    // TODO: INSERT CHARACTER-HANDLING CODE
    switch(ch) {
        case 27:
            pauseAnimation();
            break;
        case 's':
        case 'S':
            startAnimation();
            break;
    }
    glutPostRedisplay();  // Causes display() to be called.
    printf("User typed %c with ASCII code %d, mouse at (%d,%d)\n", ch, ch, x, y);
}


/* specialKeyPressed() is set up in main() to be called when the user presses certaub keys
 * that do NOT type an actual character, such as an arrow or function key.  The
 * key parameter is a GLUT constant such as GLUT_KEY_LEFT, GLUT_KEY_RIGHT,
 * GLUT_KEY_UP and GLUT_KEY_DOWN for the arrow keys; GLUT_KEY_HOME for the home key,
 * and GLUT_KEY_F1 for a funtion key.  Note that escape, backspace, and delete are
 * considered to be characters and result in a call to charTyped.  The x and y
 * parameters give the mouse position when the key was pressed.
 */
void specialKeyPressed(int key, int x, int y) {
    switch(key) {
        case 100:
            roll += 1;
            if (roll >= 180)
                roll = -180;
            break;
        case 101:
            pitch -= 1;
            if (pitch <=0)
                pitch = 360;
            break;
        case 102:
            roll -= 1;
            if (roll <= -180)
                roll = 180;
            break;
        case 103:
            pitch += 1;
            if (pitch >= 360)
                pitch = 0;
            break;
    }
    // TODO: INSERT KEY-HANDLING CODE
    glutPostRedisplay();  // Causes display() to be called.
    printf("User pressed special key with code %d; mouse at (%d,%d)\n", key, x, y);
}


// --------------- mouse event functions with some typical support code for dragging -----------------

int dragging;        // 0 or 1 to indicate whether a drag operation is in progress
int dragButton;      // which button started the drag operation
int startX, startY;  // mouse position at start of drag
int prevX, prevY;    // previous mouse position during drag

/*  mouseUpDown() is set up in main() to be called when the user presses or releases
 *  a mutton ont he mouse.  The button paramter is one of the contants GLUT_LEFT_BUTTON,
 *  GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON.  The buttonState is GLUT_UP or GLUT_DOWN and
 *  tells whether this is a mouse press or a mouse release event.  x and y give the
 *  mouse position in pixel coordinates, with (0,0) at the UPPER LEFT.
 */
void mouseUpOrDown(int button, int buttonState, int x, int y) {
       // called to respond to mouse press and mouse release events
   if (buttonState == GLUT_DOWN) {  // a mouse button was pressed
       if (dragging)
          return;  // Ignore a second button press during a draw operation.
       // TODO:  INSERT CODE TO RESPOND TO MOUSE PRESS
       dragging = 1;  // Might not want to do this in all cases
       dragButton = button;
       startX = prevX = x;
       startY = prevY = y;
       printf("Mouse button %d down at (%d,%d)\n", button, x, y);
   }
   else {  // a mouse button was released
       if ( ! dragging || button != dragButton )
           return; // this mouse release does not end a drag operation.
       dragging = 0;
       // TODO:  INSERT CODE TO CLEAN UP AFTER THE DRAG (generally not needed)
       printf("Mouse button %d up at (%d,%d)\n", button, x, y);
   }
}

/*  mouseDragged() is set up in main() to be called when the user moves the mouse,
 *  but only when one or more mouse buttons are pressed.  x and y give the position
 *  of the mouse in pixel coordinates.
 */
void mouseDragged(int x, int y) {
        // called to respond when the mouse moves during a drag
    if ( ! dragging )
        return;  // This is not part of a drag that we want to respond to.
    // TODO:  INSERT CODE TO RESPOND TO NEW MOUSE POSITION
    prevX = x;
    prevY = y;
    printf("Mouse dragged to (%d,%d)\n", x, y);
}


// ------------------------------- Menu support ----------------------------------

/* A function to be called when the user selects a command from the menu.  The itemCode
 * is the value that was associated with the command in the call to glutAddMenuEntry()
 * that added the command to the menu.
 */
void doMenu( int itemCode ) {
   if (itemCode == 1) {
       exit(0);
   }
   // TODO: Add support for other commands.
}

/* createMenu() is called in main to add a pop-up menu to the window.  The menu
 * pops up when the user right-clicks the display.
 */
void createMenu() {
   glutCreateMenu(doMenu); // doMenu() will be called when the user selects a command from the menu.
   
   glutAddMenuEntry("Quit", 1);  // Add a command named "Quit" to the menu with item code = 1.
                                 // The code will be passed as a parameter to doMenu() when
                                 // the user selects this command from the menu.
                                 
   // TODO: Add additional menu items.  (It is also possible to have submenus.)
                                
   glutAttachMenu(GLUT_RIGHT_BUTTON);  // Menu will appear when user RIGHT-clicks the display.
}

// ----------------- main routine -------------------------------------------------

int main(int argc, char** argv) {
    glutInit(&argc, argv); // Allows processing of certain GLUT command line options
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);  // Usually, omit GLUT_DEPTH for 2D drawing!
    glutInitWindowSize(720,720);        // size of display area, in pixels
    glutInitWindowPosition(100,100);    // location in window coordinates
    glutCreateWindow("OpenGL Program"); // parameter is window title  
    glutDisplayFunc(display);           // call display() when the window needs to be redrawn
    glutReshapeFunc(reshape);           // call reshape() when the size of the window changes

    /* TODO: Uncomment one or both of the next two lines for keyboard event handling. */
    glutKeyboardFunc(charTyped);        // call charTyped() when user types a character
    glutSpecialFunc(specialKeyPressed); // call specialKeyPressed() when user presses a special key

    /* TODO: Uncomment the next line to handle mouse presses; the next two for mouse dragging. */
    glutMouseFunc(mouseUpOrDown);       // call mouseUpOrDown() for mousedown and mouseup events
    glutMotionFunc(mouseDragged);       // call mouseDragged() when mouse moves, only during a drag gesture
    
    /* TODO: Uncomment the next line to add a popup menu */
    createMenu();

    initGL();

    /* TODO: Uncomment the next line to start a timer-controlled animation. */
    //startAnimation();
    
    glutMainLoop(); // Run the event loop!  This function does not return.
    return 0;
}
