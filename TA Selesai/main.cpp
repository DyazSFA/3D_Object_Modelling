#include <GL/glut.h>
// #include <stdlib.h>
// #include <iostream>
#include <stdio.h>
#include "glm.h"
#include "Imageloader.h"
#include <mmsystem.h>

int win_width = 640, win_height = 480,A=0, tx, ty, tz = -3, sudut = 0, rx, ry, rz, step=1;

typedef void (*ButtonCallback)();

void drawBitmapText1(char *string,float x,float y,float z)
{
    char *c;
    glRasterPos3f(x, y,z);
    for (c=string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
}

struct Button
{
    int   x;
    int   y;
    int   w;
    int   h;
    int	  state;
    int	  highlighted;
    char* label;
    ButtonCallback callbackFunction;
};

void Font(void *font,char *text,int x,int y)
{
    glRasterPos2i(x, y);

    while( *text != '\0' )
    {
        glutBitmapCharacter( font, *text );
        ++text;
    }
}

void ButtonDraw(Button *b)
{
    int fontx;
    int fonty;

    if(b)
    {
        /*
         *	We will indicate that the mouse cursor is over the button by changing its
         *	colour.
         */
        if (b->highlighted)
            glColor3f(1.f,0.4f,0.8f); //jika kena hover mouse
        else
            glColor3f(1.0f,0.2f,1.0f); //warna block

        /*
         *	draw background for the button.
         */
        glBegin(GL_QUADS);
        glVertex2i( b->x, b->y      );
        glVertex2i( b->x, b->y+b->h );
        glVertex2i( b->x+b->w, b->y+b->h );
        glVertex2i( b->x+b->w, b->y      );
        glEnd();

        /*
         *	Draw an outline around the button with width 3
         */
        glLineWidth(3);

        /*
         *	The colours for the outline are reversed when the button.
         */
        if (b->state)
            glColor3f(0.4f,0.4f,0.4f); //border atas
        else
            glColor3f(1.0f,0.0f,0.4f);

        glBegin(GL_LINE_STRIP);
        glVertex2i( b->x+b->w, b->y      );
        glVertex2i( b->x, b->y      );
        glVertex2i( b->x, b->y+b->h );
        glEnd();

        if (b->state)
            glColor3f(0.8f,0.8f,0.8f); //border bawah
        else
            glColor3f(0.0f,0.0f,1.0f);

        glBegin(GL_LINE_STRIP);
        glVertex2i( b->x, b->y+b->h );
        glVertex2i( b->x+b->w, b->y+b->h );
        glVertex2i( b->x+b->w, b->y      );
        glEnd();

        glLineWidth(1);


        /*
         *	Calculate the x and y coords for the text string in order to center it.
         */
        fontx = b->x + (b->w - glutBitmapLength(GLUT_BITMAP_HELVETICA_10,(unsigned char*)b->label)) / 2 ;
        fonty = b->y + (b->h+10)/2;

        /*
         *	if the button is pressed, make it look as though the string has been pushed
         *	down. It's just a visual thing to help with the overall look....
         */
        if (b->state)
        {
            fontx+=2;
            fonty+=2;
        }

        /*
         *	If the cursor is currently over the button we offset the text string and draw a shadow
         */
        if(b->highlighted)
        {
            glColor3f(0,0,0); //hover font
            Font(GLUT_BITMAP_HELVETICA_10,b->label,fontx,fonty);
            fontx--;
            fonty--;
        }

        glColor3f(1,1,1);
        Font(GLUT_BITMAP_HELVETICA_10,b->label,fontx,fonty);
    }
}

int ButtonClickTest(Button* b,int x,int y)
{
    if( b)
    {
        /*
         *	If clicked within button area, then return true
         */
        if( x > b->x      &&
                x < b->x+b->w &&
                y > b->y      &&
                y < b->y+b->h )
        {
            return 1;
        }
    }

    /*
     *	otherwise false.
     */
    return 0;
}

void ButtonPassive(Button *b,int x,int y)
{
    if(b)
    {
        /*
         *	if the mouse moved over the control
         */
        if( ButtonClickTest(b,x,y) )
        {
            /*
             *	If the cursor has just arrived over the control, set the highlighted flag
             *	and force a redraw. The screen will not be redrawn again until the mouse
             *	is no longer over this control
             */
            if( b->highlighted == 0 )
            {
                b->highlighted = 1;
                glutPostRedisplay();
            }
        }
        else

            /*
             *	If the cursor is no longer over the control, then if the control
             *	is highlighted (ie, the mouse has JUST moved off the control) then
             *	we set the highlighting back to false, and force a redraw.
             */
            if( b->highlighted == 1 )
            {
                b->highlighted = 0;
                glutPostRedisplay();
            }
    }
}

void ButtonPress(Button *b,int x,int y)
{
    if(b)
    {
        /*
         *	if the mouse click was within the buttons client area,
         *	set the state to true.
         */
        if( ButtonClickTest(b,x,y) )
        {
            b->state = 1;
        }
    }
}

struct Mouse
{
    int x;		/*	the x coordinate of the mouse cursor	*/
    int y;		/*	the y coordinate of the mouse cursor	*/
    int lmb;	/*	is the left button pressed?		*/
    int mmb;	/*	is the middle button pressed?	*/
    int rmb;	/*	is the right button pressed?	*/

    /*
     *	These two variables are a bit odd. Basically I have added these to help replicate
     *	the way that most user interface systems work. When a button press occurs, if no
     *	other button is held down then the co-ordinates of where that click occured are stored.
     *	If other buttons are pressed when another button is pressed it will not update these
     *	values.
     *
     *	This allows us to "Set the Focus" to a specific portion of the screen. For example,
     *	in maya, clicking the Alt+LMB in a view allows you to move the mouse about and alter
     *	just that view. Essentually that viewport takes control of the mouse, therefore it is
     *	useful to know where the first click occured....
     */
    int xpress; /*	stores the x-coord of when the first button press occurred	*/
    int ypress; /*	stores the y-coord of when the first button press occurred	*/
};

Mouse TheMouse = {0,0,0,0,0};

void ButtonRelease(Button *b,int x,int y)
{
    if(b)
    {
        /*
         *	If the mouse button was pressed within the button area
         *	as well as being released on the button.....
         */
        if( ButtonClickTest(b,TheMouse.xpress,TheMouse.ypress) &&
                ButtonClickTest(b,x,y) )
        {
            /*
             *	Then if a callback function has been set, call it.
             */
            if (b->callbackFunction)
            {
                b->callbackFunction();
            }
        }

        /*
         *	Set state back to zero.
         */
        b->state = 0;
    }
}

void TheButtonCallback()
{
    printf("I have been called\n");
    A=1;
}

void TheButtonCallback2()
{
    printf("About Us");
    A=2;
}

void TheButtonCallBack3()
{
    printf("Back");
    A=0;
}

Button MyButton = {270,150, 100,25, 0,0, "MENU UTAMA", TheButtonCallback };
Button MyButton2 = {270,250, 100,25, 0,0, "ABOUT US", TheButtonCallback2 };
Button MyButton3 = {50, 50, 100, 25, 0,0, "Back", TheButtonCallBack3};

void MouseMotion(int x, int y)
{
    /*
     *	Calculate how much the mouse actually moved
     */
    int dx = x - TheMouse.x;
    int dy = y - TheMouse.y;

    /*
     *	update the mouse position
     */
    TheMouse.x = x;
    TheMouse.y = y;


    /*
     *	Check MyButton to see if we should highlight it cos the mouse is over it
     */
    ButtonPassive(&MyButton,x,y);
    ButtonPassive(&MyButton2,x,y);
    ButtonPassive(&MyButton3,x,y);

    /*
     *	Force a redraw of the screen
     */
    glutPostRedisplay();
}

void MousePassiveMotion(int x, int y)
{
    /*
     *	Calculate how much the mouse actually moved
     */
    int dx = x - TheMouse.x;
    int dy = y - TheMouse.y;

    /*
     *	update the mouse position
     */
    TheMouse.x = x;
    TheMouse.y = y;

    /*
     *	Check MyButton to see if we should highlight it cos the mouse is over it
     */
    ButtonPassive(&MyButton,x,y);
    ButtonPassive(&MyButton2,x,y);
    ButtonPassive(&MyButton3,x,y);

    /*
     *	Note that I'm not using a glutPostRedisplay() call here. The passive motion function
     *	is called at a very high frequency. We really don't want much processing to occur here.
     *	Redrawing the screen every time the mouse moves is a bit excessive. Later on we
     *	will look at a way to solve this problem and force a redraw only when needed.
     */
}

void MouseButton(int button,int state,int x, int y)
{
    /*
     *	update the mouse position
     */
    TheMouse.x = x;
    TheMouse.y = y;

    /*
     *	has the button been pressed or released?
     */
    if (state == GLUT_DOWN)
    {
        /*
         *	This holds the location of the first mouse click
         */
        if ( !(TheMouse.lmb || TheMouse.mmb || TheMouse.rmb) )
        {
            TheMouse.xpress = x;
            TheMouse.ypress = y;
        }

        /*
         *	Which button was pressed?
         */
        switch(button)
        {
        case GLUT_LEFT_BUTTON:
            TheMouse.lmb = 1;
            ButtonPress(&MyButton,x,y);
            ButtonPress(&MyButton2,x,y);
            ButtonPress(&MyButton3,x,y);
            break;
        case GLUT_MIDDLE_BUTTON:
            TheMouse.mmb = 1;
            break;
        case GLUT_RIGHT_BUTTON:
            TheMouse.rmb = 1;
            break;
        }
    }
    else
    {
        /*
         *	Which button was released?
         */
        switch(button)
        {
        case GLUT_LEFT_BUTTON:
            TheMouse.lmb = 0;
            ButtonRelease(&MyButton,x,y);
            ButtonRelease(&MyButton2,x,y);
            ButtonRelease(&MyButton3,x,y);
            break;
        case GLUT_MIDDLE_BUTTON:
            TheMouse.mmb = 0;
            break;
        case GLUT_RIGHT_BUTTON:
            TheMouse.rmb = 0;
            break;
        }
    }
}

void Draw2D()
{
    ButtonDraw(&MyButton);
    ButtonDraw(&MyButton2);
    
}

// global
GLfloat angle = 0;
GLuint texId;
GLuint _textureId3;
// end

void resize(int w, int h)
{
    const float ar = (float) w / (float) h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, ar, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void profil()
{
    glPushMatrix();
    glTranslatef(0, 0, -5);
    glBegin(GL_QUADS);
    //glColor3f(1,0,0);
    glNormal3f(0,0,1);
    glTexCoord2d(0,1);
    glVertex3f(-2,2,0);
    glTexCoord2d(1,1);
    glVertex3f(2, 2,0);
    glTexCoord2d(1,0);
    glVertex3f(2, -2,0);
    glTexCoord2d(0,0);
    glVertex3f(-2,-2,0);
    glEnd();
    glPopMatrix();
}

void profilpict()
{
    GLfloat lightPos[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat lightDiffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat lightSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat shine[] = {80};

    glClearColor(0, 0, 0, 1);

    glClearDepth(1);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glMaterialfv(GL_FRONT, GL_AMBIENT, lightAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, lightDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, lightSpecular);

    glMaterialfv(GL_FRONT, GL_SHININESS, shine);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    Image* image3 = loadBMP("menu.bmp");
    loadTextures(image3, &_textureId3,1);
}

GLMmodel *monkey = NULL;

void drawModel()
{
    if (!monkey)
    {
        monkey = glmReadOBJ("sip.obj");
        if (!monkey) exit(0);
        glmUnitize(monkey);
        glmFacetNormals(monkey);
        glmVertexNormals(monkey, 90.0);
        printf("%d\n", monkey->numvertices);
        printf("%d\n", monkey->numtexcoords);
        printf("%d\n", monkey->numnormals);
    }
    glmDraw(monkey, GLM_SMOOTH | GLM_TEXTURE);
    
}
void Draw()
{
    /*
     *	Clear the background
     */
    glClear( GL_COLOR_BUFFER_BIT |
             GL_DEPTH_BUFFER_BIT );

    if(A==0)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        /*
         *	    Set the orthographic viewing transformation
         */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0,win_width,win_height,0,-1,1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        /*
         *	Draw the 2D overlay
         */
        Draw2D();

        /*
         *	Bring the back buffer to the front and vice-versa.
         */
        glLoadIdentity();
        glColor3f(1,1,0);
        drawBitmapText1("WAWASAN NUSANTARA",180,70,0);

    }
    else if(A==1)
    {
        glClearColor(0, 0, 0, 1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45,(win_height==0)?(1):((float)win_width/win_height),1,100);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // glColor3f(1, 0, 0);
        glPushMatrix();
            glTranslatef(tx, 0, tz);
            glRotatef(angle+=0.1, rx, 1, rz);
            // glutWireSphere(7, 20, 20);
            glRotatef(sudut, 1, 0, 0);
            drawModel();
        glPopMatrix(); 
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0,win_width,win_height,0,-1,1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        ButtonDraw(&MyButton3);
    }

    else if(A==2)
    {
        glClearColor(0, 0, 0, 1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, _textureId3);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45,(win_height==0)?(1):((float)win_width/win_height),1,100);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // glColor3f(1, 0, 0);
        glPushMatrix();
        profil();
        profilpict();
        glPopMatrix();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0,win_width,win_height,0,-1,1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        ButtonDraw(&MyButton3);
    }

    glutSwapBuffers();
}

void idle()
{
    glutPostRedisplay();
}

void getImage()
{
    Image *img = loadBMP("tes.bmp");
    loadTextures(img, &texId, 1);
}

void myKeyboard(unsigned char key, int x, int y)
{
    if (key == 'w')
    {
        tz+=1;
    }
    else if (key == 's')
    {
        tz-=1;
    }
    else if (key == 'x')
    {
        rx=1;
        ry=0;
        rz=0;
        sudut+=1;
    }
    else if (key == 'y')
    {
        rx=0;
        ry=1;
        rz=0;
        sudut+=1;
    }
    else if (key == 'z')
    {
        rx=0;
        ry=0;
        rz=1;
        sudut+=1;
    }
    else if (key == 'a')
    {
        tx += step;
    }
    else if (key == 'd')
    {
        tx -= step;
    }
}

void init()
{
    GLfloat lightPos[] = {0, 5, 0.1, 20, 0};
    GLfloat lightAmbient[] = {0.3, 0.3, 0.3, 1};
    GLfloat lightDiffuse[] = {0.4, 0.4, 0.4, 1};
    GLfloat lightSpecular[] = {0.5, 0.5, 0.5, 0.1};
    GLfloat shine[] = {10};

    glClearColor(0, 0, 0, 1);

    glClearDepth(1);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, lightSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shine);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    getImage();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(win_width, win_height);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Load OBJ");
    glutReshapeFunc(resize);
//    glutDisplayFunc(display);
    glutIdleFunc(idle);
    init();
    glutDisplayFunc(Draw);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutKeyboardFunc(myKeyboard);
    glutPassiveMotionFunc(MousePassiveMotion);
    sndPlaySound("Tana_Wolio_-_Lagu_Daerah_Sulawesi_Tenggara_Karaoke_dengan_Lirik-GC7315013EM.wav",SND_ASYNC);
    glutMainLoop();
    return 0;
}
