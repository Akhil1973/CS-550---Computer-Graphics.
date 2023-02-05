#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// title of these windows:

const char* WINDOWTITLE = "Akhil Sai Chintala";
const char* GLUITITLE = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT = 4;
const int MIDDLE = 2;
const int RIGHT = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char* ColorNames[] =
{
	(char*)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta",
	(char*)"White",
	(char*)"Black"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

struct Point
{
	float x0, y0, z0;       // initial coordinates
	float x, y, z;        // animated coordinates
};

struct Curve6			// for a 6-point curve
{
	float r, g, b;
	Point p0, p1, p2, p3, p4, p5;
};

struct Curve8			// for an 8-point curve
{
	float r, g, b;
	Point p0, p1, p2, p3, p4, p5, p6, p7;
};

struct Curven			// for an n-point curve
{
	float r, g, b;
	int numpoints;
	Point* points;		// requires you to dynamically create this array 
};


// if you use a curven, you could draw a curve like this:  struct Curven c12 = CreateNpointCurve( 12 );

Curven
CreateNpointCurve(int n)
{
	struct Curven cn;
	cn.numpoints = n;
	cn.points = new struct Point[n];
	// Or you could do this:   cn.points = (struct Point *) malloc( n*sizeof(struct Point) );
	return cn;
}

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE = GL_LINEAR;
const GLfloat FOGDENSITY = 0.30f;
const GLfloat FOGSTART = 1.5f;
const GLfloat FOGEND = 4.f;


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong

//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
int freeze = 0;
int points;
int lines;
int curves;
Curve6 body;
Curve6 wing1;
Curve6 wing2;
Curve6 innerwing1;
Curve6 innerwing2;
Curve6 innerwing3;
Curve6 innerwing4;
float Time;
int ms;
#define MS_IN_THE_ANIMATION_CYCLE	10000

// function prototypes:

void	Animate();
void	Display();
void	DoAxesMenu(int);
void	DoColorMenu(int);
void	DoDepthBufferMenu(int);
void	DoDepthFightingMenu(int);
void	DoDepthMenu(int);
void	DoDebugMenu(int);
void	DoPointMenu(int);
void	DoLineMenu(int);
void	DoCurveMenu(int);
void	DoMainMenu(int);
void	DoProjectMenu(int);
void	DoRasterString(float, float, float, char*);
void	DoStrokeString(float, float, float, float, char*);
float	ElapsedSeconds();
void	InitGraphics();
void	InitLists();
void	InitMenus();
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);

void			Axes(float);

unsigned char* BmpToTexture(char*, int*, int*);
int				ReadInt(FILE*);
short			ReadShort(FILE*);

void			HsvRgb(float[3], float[3]);

void			Cross(float[3], float[3], float[3]);
float			Dot(float[3], float[3]);
float			Unit(float[3], float[3]);

void
RotateX(Point* p, float deg, float xc, float yc, float zc)
{
	float rad = deg * (M_PI / 180.f);         // radians
	float x = p->x0 - xc;
	float y = p->y0 - yc;
	float z = p->z0 - zc;

	float xp = x;
	float yp = y * cos(rad) - z * sin(rad);
	float zp = y * sin(rad) + z * cos(rad);

	p->x = xp + xc;
	p->y = yp + yc;
	p->z = zp + zc;
}

void
RotateY(Point* p, float deg, float xc, float yc, float zc)
{
	float rad = deg * (M_PI / 180.f);         // radians
	float x = p->x0 - xc;
	float y = p->y0 - yc;
	float z = p->z0 - zc;

	float xp = x * cos(rad) + z * sin(rad);
	float yp = y;
	float zp = -x * sin(rad) + z * cos(rad);

	p->x = xp + xc;
	p->y = yp + yc;
	p->z = zp + zc;
}

void
RotateZ(Point* p, float deg, float xc, float yc, float zc)
{
	float rad = deg * (M_PI / 180.f);         // radians
	float x = p->x0 - xc;
	float y = p->y0 - yc;
	float z = p->z0 - zc;

	float xp = x * cos(rad) - y * sin(rad);
	float yp = x * sin(rad) + y * cos(rad);
	float zp = z;

	p->x = xp + xc;
	p->y = yp + yc;
	p->z = zp + zc;
}

void
Translate(Point* p, float transx, float transy, float transz)
{
	p->x = p->x0 + transx;
	p->y = p->y0 + transy;
	p->z = p->z0 + transz;
}

void
DrawFourPointCurve(Point* p0, Point* p1, Point* p2, Point* p3)
{
#define NUM_POINTS_IN_CURVE_DRAWING	20
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i <= NUM_POINTS_IN_CURVE_DRAWING; i++)
	{
		float t = (float)i / (float)NUM_POINTS_IN_CURVE_DRAWING;	// t goes from 0. to 1.
		float x = 0.5 * ((2 * p1->x) + (t * (p2->x - p0->x)) + (t * t * ((2 * p0->x) - (5. * p1->x) + (4. * p2->x) - (p3->x))) + (t * t * t * (-p0->x + (3. * p1->x) - (3. * p2->x) + p3->x)));
		float y = 0.5 * ((2 * p1->y) + (t * (p2->y - p0->y)) + (t * t * ((2 * p0->y) - (5. * p1->y) + (4. * p2->y) - (p3->y))) + (t * t * t * (-p0->y + (3. * p1->y) - (3. * p2->y) + p3->y)));
		float z = 0.5 * ((2 * p1->z) + (t * (p2->z - p0->z)) + (t * t * ((2 * p0->z) - (5. * p1->z) + (4. * p2->z) - (p3->z))) + (t * t * t * (-p0->z + (3. * p1->z) - (3. * p2->z) + p3->z)));
		glVertex3f(x, y, z);
	}
	glEnd();
}

void
DrawPoint(Point* p0)
{
	glPointSize(4.);
	glBegin(GL_POINTS);
	glScalef(5., 5., 5.);
	glVertex3f(p0->x, p0->y, p0->z);
	glEnd();
}

void
DrawLine(Point* p0, Point* p1, Point* p2, Point* p3)
{
	
	glBegin(GL_LINE_STRIP);
	glScalef(5., 5., 5.);
	glVertex3f(p0->x, p0->y, p0->z);
	glVertex3f(p1->x, p1->y, p1->z);
	glVertex3f(p2->x, p2->y, p2->z);
	glVertex3f(p3->x, p3->y, p3->z);
	glEnd();

}
// main program:

int
main(int argc, char* argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit(&argc, argv);

	// setup all the graphics stuff:

	InitGraphics();

	// create the display structures that will not change:

	InitLists();

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset();

	// setup all the user interface stuff:

	InitMenus();

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow(MainWindow);
	glutMainLoop();

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate()
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	// force a call to Display( ) next time it is convenient:
	//int ms = glutGet(GLUT_ELAPSED_TIME);	// milliseconds
	ms = ms + 1;
	ms %= MS_IN_THE_ANIMATION_CYCLE;
	Time = (float)ms / (float)MS_IN_THE_ANIMATION_CYCLE;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// draw the complete scene:

void
Display()
{
	// set which window we want to do the graphics into:

	glutSetWindow(MainWindow);


	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
	if (DepthBufferOn == 0)
		glDisable(GL_DEPTH_TEST);
#endif


	// specify shading to be flat:

	glShadeModel(GL_FLAT);


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
	else
		gluPerspective(70.f, 1.f, 0.1f, 1000.f);


	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:

	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.5f, 0.f, 0.f, 1.f, 0.f);


	// rotate the scene:

	glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
	glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);


	// uniformly scale the scene:

	if (Scale < MINSCALE)
		Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);


	// set the fog parameters:

	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}


	// possibly draw the axes:

	if (AxesOn != 0)
	{
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}


	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable(GL_NORMALIZE);


	// Animation
	Translate(&body.p0, 0., 0., 0.25 * -sin(4. * M_PI * Time));
	Translate(&body.p5, 0., 0., 0.25 * -sin(4. * M_PI * Time));

	Translate(&wing1.p2, 0., 0., 0.25 * -sin(4. * M_PI * Time) );
	Translate(&wing1.p3, 0., 0., 0.25 * -sin(4. * M_PI * Time) );
	Translate(&wing1.p4, 0., 0., 0.25 * -sin(4. * M_PI * Time) );


	Translate(&wing2.p2, 0., 0., 0.25 * -sin(4. * M_PI * Time) );
	Translate(&wing2.p3, 0., 0., 0.25 * -sin(4. * M_PI * Time) );
	Translate(&wing2.p4, 0., 0., 0.25 * -sin(4. * M_PI * Time) );



	Translate(&innerwing1.p2, 0., 0., -0.25 * sin(4. * M_PI * Time));

	Translate(&innerwing2.p2, 0., 0., -0.25 * sin(4. * M_PI * Time));

	Translate(&innerwing3.p2, 0., 0., -0.25 * sin(4. * M_PI * Time));



	Translate(&innerwing4.p2, 0., 0., -0.25 * sin(4. * M_PI * Time));


	if (!lines)
	{
		glColor3f(1., 1., 1.);
		DrawLine(&body.p0, &body.p1, &body.p2, &body.p3);	
		DrawLine(&body.p1, &body.p2, &body.p3, &body.p4);	
		DrawLine(&body.p2, &body.p3, &body.p4, &body.p1);	
		DrawLine(&body.p3, &body.p4, &body.p1, &body.p5);
		DrawLine(&body.p4, &body.p1, &body.p5, &body.p2);
		DrawLine(&body.p2, &body.p1, &body.p0, &body.p4);



		glColor3f(1., 1., 1.);
		DrawLine(&wing1.p0, &wing1.p1, &wing1.p2, &wing1.p3);	
		DrawLine(&wing1.p1, &wing1.p2, &wing1.p3, &wing1.p4);	
		DrawLine(&wing1.p2, &wing1.p3, &wing1.p4, &wing1.p5);	
		DrawLine(&wing1.p3, &wing1.p4, &wing1.p5, &wing1.p0);

		glColor3f(1., 1., 1.);
		DrawLine(&wing2.p0, &wing2.p1, &wing2.p2, &wing2.p3);	
		DrawLine(&wing2.p1, &wing2.p2, &wing2.p3, &wing2.p4);	
		DrawLine(&wing2.p2, &wing2.p3, &wing2.p4, &wing2.p5);	
		DrawLine(&wing2.p3, &wing2.p4, &wing2.p5, &wing2.p0);

		glColor3f(1., 1., 1.);
		DrawLine(&innerwing1.p0, &innerwing1.p1, &innerwing1.p2, &innerwing1.p3);	// p0-p1-p2-p3
		DrawLine(&innerwing1.p1, &innerwing1.p2, &innerwing1.p3, &innerwing1.p0);	
		glColor3f(1., 1., 1.);
		DrawLine(&innerwing2.p0, &innerwing2.p1, &innerwing2.p2, &innerwing2.p3);	// p0-p1-p2-p3
		DrawLine(&innerwing2.p1, &innerwing2.p2, &innerwing2.p3, &innerwing2.p0);	

		glColor3f(1., 1., 1.);
		DrawLine(&innerwing3.p0, &innerwing3.p1, &innerwing3.p2, &innerwing3.p3);	// p0-p1-p2-p3
		DrawLine(&innerwing3.p1, &innerwing3.p2, &innerwing3.p3, &innerwing3.p0);	// p1-p2-p3-p4
		
		glColor3f(1., 1., 1.);
		DrawLine(&innerwing4.p0, &innerwing4.p1, &innerwing4.p2, &innerwing4.p3);	// p0-p1-p2-p3
		DrawLine(&innerwing4.p1, &innerwing4.p2, &innerwing4.p3, &innerwing4.p0);	
	}



	if (curves)
	{
		glColor3f(body.r, body.g, body.b);
		DrawFourPointCurve(&body.p0, &body.p1, &body.p2, &body.p3);	// p0-p1-p2-p3
		DrawFourPointCurve(&body.p1, &body.p2, &body.p3, &body.p4);	// p1-p2-p3-p4
		DrawFourPointCurve(&body.p2, &body.p3, &body.p4, &body.p1);	// p2-p3-p4-p5	(p5 is the 6th point)
		DrawFourPointCurve(&body.p3, &body.p4, &body.p1, &body.p5);
		DrawFourPointCurve(&body.p4, &body.p1, &body.p5, &body.p2);
		DrawFourPointCurve(&body.p2, &body.p1, &body.p0, &body.p4);



		glColor3f(wing1.r, wing1.g, wing1.b);
		DrawFourPointCurve(&wing1.p0, &wing1.p1, &wing1.p2, &wing1.p3);	// p0-p1-p2-p3
		DrawFourPointCurve(&wing1.p1, &wing1.p2, &wing1.p3, &wing1.p4);	// p1-p2-p3-p4
		DrawFourPointCurve(&wing1.p2, &wing1.p3, &wing1.p4, &wing1.p5);	// p2-p3-p4-p5	(p5 is the 6th point)
		DrawFourPointCurve(&wing1.p3, &wing1.p4, &wing1.p5, &wing1.p0);

		glColor3f(wing2.r, wing2.g, wing2.b);
		DrawFourPointCurve(&wing2.p0, &wing2.p1, &wing2.p2, &wing2.p3);	// p0-p1-p2-p3
		DrawFourPointCurve(&wing2.p1, &wing2.p2, &wing2.p3, &wing2.p4);	// p1-p2-p3-p4
		DrawFourPointCurve(&wing2.p2, &wing2.p3, &wing2.p4, &wing2.p5);	// p2-p3-p4-p5	(p5 is the 6th point)
		DrawFourPointCurve(&wing2.p3, &wing2.p4, &wing2.p5, &wing2.p0);

		glColor3f(innerwing1.r, innerwing1.g, innerwing1.b);
		DrawFourPointCurve(&innerwing1.p0, &innerwing1.p1, &innerwing1.p2, &innerwing1.p3);	// p0-p1-p2-p3
		DrawFourPointCurve(&innerwing1.p1, &innerwing1.p2, &innerwing1.p3, &innerwing1.p0);	// p1-p2-p3-p4
		//DrawFourPointCurve(&innerwing1.p2, &innerwing1.p3, &innerwing1.p4, &innerwing1.p5);	// p2-p3-p4-p5	(p5 is the 6th point)

		glColor3f(innerwing2.r, innerwing2.g, innerwing2.b);
		DrawFourPointCurve(&innerwing2.p0, &innerwing2.p1, &innerwing2.p2, &innerwing2.p3);	// p0-p1-p2-p3
		DrawFourPointCurve(&innerwing2.p1, &innerwing2.p2, &innerwing2.p3, &innerwing2.p0);	// p1-p2-p3-p4
		//DrawFourPointCurve(&innerwing2.p2, &innerwing2.p3, &innerwing2.p4, &innerwing2.p5);	// p2-p3-p4-p5	(p5 is the 6th point)

		glColor3f(innerwing3.r, innerwing3.g, innerwing3.b);
		DrawFourPointCurve(&innerwing3.p0, &innerwing3.p1, &innerwing3.p2, &innerwing3.p3);	// p0-p1-p2-p3
		DrawFourPointCurve(&innerwing3.p1, &innerwing3.p2, &innerwing3.p3, &innerwing3.p0);	// p1-p2-p3-p4
		//DrawFourPointCurve(&innerwing3.p2, &innerwing3.p3, &innerwing3.p4, &innerwing3.p5);	// p2-p3-p4-p5	(p5 is the 6th point)

		glColor3f(innerwing4.r, innerwing4.g, innerwing4.b);
		DrawFourPointCurve(&innerwing4.p0, &innerwing4.p1, &innerwing4.p2, &innerwing4.p3);	// p0-p1-p2-p3
		DrawFourPointCurve(&innerwing4.p1, &innerwing4.p2, &innerwing4.p3, &innerwing4.p0);	// p1-p2-p3-p4
		//DrawFourPointCurve(&innerwing4.p2, &innerwing4.p3, &innerwing4.p4, &innerwing4.p5);	// p2-p3-p4-p5	(p5 is the 6th point)
	}

	if (points)
	{
		glColor3f(1., 1., 1.);
		DrawPoint(&body.p0);
		DrawPoint(&body.p1);
		DrawPoint(&body.p2);
		DrawPoint(&body.p3);
		DrawPoint(&body.p4);
		DrawPoint(&body.p5);


		DrawPoint(&wing1.p0);
		DrawPoint(&wing1.p1);
		DrawPoint(&wing1.p2);
		DrawPoint(&wing1.p3);
		DrawPoint(&wing1.p4);
		DrawPoint(&wing1.p5);

		DrawPoint(&wing2.p0);
		DrawPoint(&wing2.p1);
		DrawPoint(&wing2.p2);
		DrawPoint(&wing2.p3);
		DrawPoint(&wing2.p4);
		DrawPoint(&wing2.p5);

		DrawPoint(&innerwing1.p0);
		DrawPoint(&innerwing1.p1);
		DrawPoint(&innerwing1.p2);
		DrawPoint(&innerwing1.p3);
		/*DrawPoint(&innerwing1.p4);
		DrawPoint(&innerwing1.p5);*/

		DrawPoint(&innerwing2.p0);
		DrawPoint(&innerwing2.p1);
		DrawPoint(&innerwing2.p2);
		DrawPoint(&innerwing2.p3);
		/*DrawPoint(&innerwing2.p4);
		DrawPoint(&innerwing2.p5);*/

		DrawPoint(&innerwing3.p0);
		DrawPoint(&innerwing3.p1);
		DrawPoint(&innerwing3.p2);
		DrawPoint(&innerwing3.p3);
		/*DrawPoint(&innerwing3.p4);
		DrawPoint(&innerwing3.p5);*/

		DrawPoint(&innerwing4.p0);
		DrawPoint(&innerwing4.p1);
		DrawPoint(&innerwing4.p2);
		DrawPoint(&innerwing4.p3);/*
		DrawPoint(&innerwing4.p4);
		DrawPoint(&innerwing4.p5);*/
	}

#ifdef DEMO_Z_FIGHTING
	if (DepthFightingOn != 0)
	{
		glPushMatrix();
		glRotatef(90.f, 0.f, 1.f, 0.f);
		glCallList(BoxList);
		glPopMatrix();
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable(GL_DEPTH_TEST);
	glColor3f(0.f, 1.f, 1.f);
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.f, 100.f, 0.f, 100.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1.f, 1.f, 1.f);
	DoRasterString(5., 5., 0., (char*)"Akhil's Project 6 - Butterfly");

	// swap the double-buffered framebuffers:

	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}


void
DoAxesMenu(int id)
{
	AxesOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoColorMenu(int id)
{
	WhichColor = id - RED;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDebugMenu(int id)
{
	DebugOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void DoPointMenu(int id)
{
	points = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoLineMenu(int id)
{
	lines = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoCurveMenu(int id)
{
	curves = id;
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthBufferMenu(int id)
{
	DepthBufferOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthFightingMenu(int id)
{
	DepthFightingOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthMenu(int id)
{
	DepthCueOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// main menu callback:

void
DoMainMenu(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		break;

	case QUIT:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoProjectMenu(int id)
{
	WhichProjection = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// use glut to display a string of characters using a raster font:

void
DoRasterString(float x, float y, float z, char* s)
{
	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);

	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString(float x, float y, float z, float ht, char* s)
{
	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05f + 33.33f);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus()
{
	glutSetWindow(MainWindow);

	int numColors = sizeof(Colors) / (3 * sizeof(int));
	int colormenu = glutCreateMenu(DoColorMenu);
	for (int i = 0; i < numColors; i++)
	{
		glutAddMenuEntry(ColorNames[i], i);
	}

	int axesmenu = glutCreateMenu(DoAxesMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthcuemenu = glutCreateMenu(DoDepthMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);


	int depthbuffermenu = glutCreateMenu(DoDepthBufferMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthfightingmenu = glutCreateMenu(DoDepthFightingMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int point = glutCreateMenu(DoPointMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int line = glutCreateMenu(DoLineMenu);
	glutAddMenuEntry("Off", 1);
	glutAddMenuEntry("On", 0);


	int curve = glutCreateMenu(DoCurveMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);


	int projmenu = glutCreateMenu(DoProjectMenu);
	glutAddMenuEntry("Orthographic", ORTHO);
	glutAddMenuEntry("Perspective", PERSP);

	int mainmenu = glutCreateMenu(DoMainMenu);
	glutAddSubMenu("Axes", axesmenu);
	glutAddSubMenu("Axis Colors", colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu("Depth Buffer", depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu("Depth Fighting", depthfightingmenu);
#endif

	glutAddSubMenu("Depth Cue", depthcuemenu);
	glutAddSubMenu("Projection", projmenu);
	glutAddSubMenu("Control Points", point);
	glutAddSubMenu("Control Lines", line);
	glutAddSubMenu("Curves", curve);
	glutAddMenuEntry("Reset", RESET);
	glutAddSubMenu("Debug", debugmenu);
	glutAddMenuEntry("Quit", QUIT);

	// attach the pop-up menu to the right mouse button:

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics()
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	// set the framebuffer clear values:

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(-1, NULL, 0);

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	//glutIdleFunc(Animate);
	glutIdleFunc(NULL);
	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// body
	body.r = 0.;	body.g = 0.;	body.b = 1.;
	body.p0.x = body.p0.x0 = 0.5;
	body.p0.y = body.p0.y0 = 2.0;
	body.p0.z = body.p0.z0 = 0.8;

	body.p1.x = body.p1.x0 = 0.;
	body.p1.y = body.p1.y0 = 1.5;
	body.p1.z = body.p1.z0 = 0.8;

	body.p2.x = body.p2.x0 = -0.2;
	body.p2.y = body.p2.y0 = 0.7;
	body.p2.z = body.p2.z0 = 0.8;

	body.p3.x = body.p3.x0 = 0.;
	body.p3.y = body.p3.y0 = 0.2;
	body.p3.z = body.p3.z0 = 0.8;

	body.p4.x = body.p4.x0 = 0.2;
	body.p4.y = body.p4.y0 = 0.7;
	body.p4.z = body.p4.z0 = 0.8;

	body.p5.x = body.p5.x0 = -0.5;
	body.p5.y = body.p5.y0 = 2.0;
	body.p5.z = body.p5.z0 = 0.8;



	//wing1
	wing1.r = 1.;	wing1.g = 0.;	wing1.b = 0.;
	wing1.p0.x = wing1.p0.x0 = -0.5;
	wing1.p0.y = wing1.p0.y0 = 0.7;
	wing1.p0.z = wing1.p0.z0 = 0.8;

	wing1.p1.x = wing1.p1.x0 = 0.;
	wing1.p1.y = wing1.p1.y0 = 1.5;
	wing1.p1.z = wing1.p1.z0 = 0.8;

	wing1.p2.x = wing1.p2.x0 = 1.2;
	wing1.p2.y = wing1.p2.y0 = 1.65;
	wing1.p2.z = wing1.p2.z0 = 1.2;

	wing1.p3.x = wing1.p3.x0 = 0.7;
	wing1.p3.y = wing1.p3.y0 = 0.9;
	wing1.p3.z = wing1.p3.z0 = 1.2;

	wing1.p4.x = wing1.p4.x0 = 1.05;
	wing1.p4.y = wing1.p4.y0 = 0.45;
	wing1.p4.z = wing1.p4.z0 = 1.2;

	wing1.p5.x = wing1.p5.x0 = 0.;
	wing1.p5.y = wing1.p5.y0 = 0.2;
	wing1.p5.z = wing1.p5.z0 = 0.8;

	//wing2
	wing2.r = 1.;	wing2.g = 0.5;	wing2.b = 0.;
	wing2.p0.x = wing2.p0.x0 = 0.50;
	wing2.p0.y = wing2.p0.y0 = 0.7;
	wing2.p0.z = wing2.p0.z0 = 0.8;

	wing2.p1.x = wing2.p1.x0 = 0.;
	wing2.p1.y = wing2.p1.y0 = 1.5;
	wing2.p1.z = wing2.p1.z0 = 0.8;

	wing2.p2.x = wing2.p2.x0 = -1.2;
	wing2.p2.y = wing2.p2.y0 = 1.65;
	wing2.p2.z = wing2.p2.z0 = 1.2;

	wing2.p3.x = wing2.p3.x0 = -0.7;
	wing2.p3.y = wing2.p3.y0 = 0.9;
	wing2.p3.z = wing2.p3.z0 = 1.2;

	wing2.p4.x = wing2.p4.x0 = -1.05;
	wing2.p4.y = wing2.p4.y0 = 0.45;
	wing2.p4.z = wing2.p4.z0 = 1.2;

	wing2.p5.x = wing2.p5.x0 = 0.;
	wing2.p5.y = wing2.p5.y0 = 0.2;
	wing2.p5.z = wing2.p5.z0 = 0.8;

	//innerwing1
	innerwing1.r = 1.;	innerwing1.g = 1.;	innerwing1.b = 0.;
	innerwing1.p0.x = innerwing1.p0.x0 = 0.5;
	innerwing1.p0.y = innerwing1.p0.y0 = 0.9;
	innerwing1.p0.z = innerwing1.p0.z0 = 0.8;

	innerwing1.p1.x = innerwing1.p1.x0 = 0.;
	innerwing1.p1.y = innerwing1.p1.y0 = 1.5;
	innerwing1.p1.z = innerwing1.p1.z0 = 0.8;

	innerwing1.p2.x = innerwing1.p2.x0 = 0.9;
	innerwing1.p2.y = innerwing1.p2.y0 = 1.5;
	innerwing1.p2.z = innerwing1.p2.z0 = 1.2;

	innerwing1.p3.x = innerwing1.p3.x0 = 0.2;
	innerwing1.p3.y = innerwing1.p3.y0 = 0.9;
	innerwing1.p3.z = innerwing1.p3.z0 = 0.8;


	//innerwing2
	innerwing2.r = 0.5;	innerwing2.g = 1.;	innerwing2.b = 0.;
	innerwing2.p0.x = innerwing2.p0.x0 = -0.50;
	innerwing2.p0.y = innerwing2.p0.y0 = 0.9;
	innerwing2.p0.z = innerwing2.p0.z0 = 0.8;

	innerwing2.p1.x = innerwing2.p1.x0 = 0.;
	innerwing2.p1.y = innerwing2.p1.y0 = 1.5;
	innerwing2.p1.z = innerwing2.p1.z0 = 0.8;

	innerwing2.p2.x = innerwing2.p2.x0 = -0.9;
	innerwing2.p2.y = innerwing2.p2.y0 = 1.5;
	innerwing2.p2.z = innerwing2.p2.z0 = 1.2;

	innerwing2.p3.x = innerwing2.p3.x0 = -0.2;
	innerwing2.p3.y = innerwing2.p3.y0 = 0.9;
	innerwing2.p3.z = innerwing2.p3.z0 = 0.8;


	//innerwing3
	innerwing3.r = 0.;	innerwing3.g = 1.;	innerwing3.b = 0.5;
	innerwing3.p0.x = innerwing3.p0.x0 = -0.50;
	innerwing3.p0.y = innerwing3.p0.y0 = 0.9;
	innerwing3.p0.z = innerwing3.p0.z0 = 0.8;

	innerwing3.p1.x = innerwing3.p1.x0 = 0.2;
	innerwing3.p1.y = innerwing3.p1.y0 = 0.9;
	innerwing3.p1.z = innerwing3.p1.z0 = 0.8;

	innerwing3.p2.x = innerwing3.p2.x0 = 0.6;
	innerwing3.p2.y = innerwing3.p2.y0 = 0.65;
	innerwing3.p2.z = innerwing3.p2.z0 = 1.2;

	innerwing3.p3.x = innerwing3.p3.x0 = 0.;
	innerwing3.p3.y = innerwing3.p3.y0 = 0.2;
	innerwing3.p3.z = innerwing3.p3.z0 = 0.8;


	//innerwing4
	innerwing4.r = 0.;	innerwing4.g = 1.;	innerwing4.b = 1.;
	innerwing4.p0.x = innerwing4.p0.x0 = 0.5;
	innerwing4.p0.y = innerwing4.p0.y0 = 0.9;
	innerwing4.p0.z = innerwing4.p0.z0 = 0.8;

	innerwing4.p1.x = innerwing4.p1.x0 = -0.2;
	innerwing4.p1.y = innerwing4.p1.y0 = 0.9;
	innerwing4.p1.z = innerwing4.p1.z0 = 0.8;

	innerwing4.p2.x = innerwing4.p2.x0 = -0.6;
	innerwing4.p2.y = innerwing4.p2.y0 = 0.65;
	innerwing4.p2.z = innerwing4.p2.z0 = 1.2;

	innerwing4.p3.x = innerwing4.p3.x0 = 0.;
	innerwing4.p3.y = innerwing4.p3.y0 = 0.2;
	innerwing4.p3.z = innerwing4.p3.z0 = 0.8;


}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists()
{
	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow(MainWindow);

	// create the object:

	// create the axes:

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	//Axes(1.5);
	glLineWidth(1.);
	glEndList();
}


// the keyboard callback:

void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;
	case 'f':
	case 'F':
		freeze = !freeze;
		if (freeze)
			glutIdleFunc(NULL);
		else
			glutIdleFunc(Animate);
		break;
	case 'b':
	case 'B':
		points = !points;
		break;
	case 'v':
	case 'V':
		lines = !lines;
		break;
	case 'q':
	case 'Q':
	case ESCAPE:
		DoMainMenu(QUIT);	// will not return here
		break;				// happy compiler

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// called when the mouse button transitions down or up:

void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	case SCROLL_WHEEL_UP:
		Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	case SCROLL_WHEEL_DOWN:
		Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion(int x, int y)
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		Xrot += (ANGFACT * dy);
		Yrot += (ANGFACT * dx);
	}

	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset()
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale = 1.0;
	ShadowsOn = 0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	freeze = 0;
	points = 1;
	lines = 1;
	curves = 1;
	


}


// called when user resizes the window:

void
Resize(int width, int height)
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// handle a change to the window's visibility:

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = { 0.f, 1.f, 0.f, 1.f };

static float xy[] = { -.5f, .5f, .5f, -.5f };

static int xorder[] = { 1, 2, -3, 4 };

static float yx[] = { 0.f, 0.f, -.5f, .5f };

static float yy[] = { 0.f, .6f, 1.f, 1.f };

static int yorder[] = { 1, 2, 3, -2, 4 };

static float zx[] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact * xx[j], fact * xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact * yx[j], base + fact * yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact * zy[j], base + fact * zx[j]);
	}
	glEnd();

}

// read a BMP file into a Texture:

#define VERBOSE				false
#define BMP_MAGIC_NUMBER	0x4d42
#ifndef BI_RGB
#define BI_RGB				0
#define BI_RLE8				1
#define BI_RLE4				2
#endif


// bmp file header:
struct bmfh
{
	short bfType;		// BMP_MAGIC_NUMBER = "BM"
	int bfSize;		// size of this file in bytes
	short bfReserved1;
	short bfReserved2;
	int bfOffBytes;		// # bytes to get to the start of the per-pixel data
} FileHeader;

// bmp info header:
struct bmih
{
	int biSize;		// info header size, should be 40
	int biWidth;		// image width
	int biHeight;		// image height
	short biPlanes;		// #color planes, should be 1
	short biBitCount;	// #bits/pixel, should be 1, 4, 8, 16, 24, 32
	int biCompression;	// BI_RGB, BI_RLE4, BI_RLE8
	int biSizeImage;
	int biXPixelsPerMeter;
	int biYPixelsPerMeter;
	int biClrUsed;		// # colors in the palette
	int biClrImportant;
} InfoHeader;



// read a BMP file into a Texture:

unsigned char*
BmpToTexture(char* filename, int* width, int* height)
{
	FILE* fp;
#ifdef _WIN32
	errno_t err = fopen_s(&fp, filename, "rb");
	if (err != 0)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}
#else
	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}
#endif

	FileHeader.bfType = ReadShort(fp);


	// if bfType is not BMP_MAGIC_NUMBER, the file is not a bmp:

	if (VERBOSE) fprintf(stderr, "FileHeader.bfType = 0x%0x = \"%c%c\"\n",
		FileHeader.bfType, FileHeader.bfType & 0xff, (FileHeader.bfType >> 8) & 0xff);
	if (FileHeader.bfType != BMP_MAGIC_NUMBER)
	{
		fprintf(stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType);
		fclose(fp);
		return NULL;
	}


	FileHeader.bfSize = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "FileHeader.bfSize = %d\n", FileHeader.bfSize);

	FileHeader.bfReserved1 = ReadShort(fp);
	FileHeader.bfReserved2 = ReadShort(fp);

	FileHeader.bfOffBytes = ReadInt(fp);


	InfoHeader.biSize = ReadInt(fp);
	InfoHeader.biWidth = ReadInt(fp);
	InfoHeader.biHeight = ReadInt(fp);

	const int nums = InfoHeader.biWidth;
	const int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort(fp);

	InfoHeader.biBitCount = ReadShort(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biBitCount = %d\n", InfoHeader.biBitCount);

	InfoHeader.biCompression = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biCompression = %d\n", InfoHeader.biCompression);

	InfoHeader.biSizeImage = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biSizeImage = %d\n", InfoHeader.biSizeImage);

	InfoHeader.biXPixelsPerMeter = ReadInt(fp);
	InfoHeader.biYPixelsPerMeter = ReadInt(fp);

	InfoHeader.biClrUsed = ReadInt(fp);
	if (VERBOSE)	fprintf(stderr, "InfoHeader.biClrUsed = %d\n", InfoHeader.biClrUsed);

	InfoHeader.biClrImportant = ReadInt(fp);

	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );

	// pixels will be stored bottom-to-top, left-to-right:
	unsigned char* texture = new unsigned char[3 * nums * numt];
	if (texture == NULL)
	{
		fprintf(stderr, "Cannot allocate the texture array!\n");
		return NULL;
	}

	// extra padding bytes:

	int requiredRowSizeInBytes = 4 * ((InfoHeader.biBitCount * InfoHeader.biWidth + 31) / 32);
	if (VERBOSE)	fprintf(stderr, "requiredRowSizeInBytes = %d\n", requiredRowSizeInBytes);

	int myRowSizeInBytes = (InfoHeader.biBitCount * InfoHeader.biWidth + 7) / 8;
	if (VERBOSE)	fprintf(stderr, "myRowSizeInBytes = %d\n", myRowSizeInBytes);

	int numExtra = requiredRowSizeInBytes - myRowSizeInBytes;
	if (VERBOSE)	fprintf(stderr, "New NumExtra padding = %d\n", numExtra);


	// this function does not support compression:

	if (InfoHeader.biCompression != 0)
	{
		fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
		fclose(fp);
		return NULL;
	}

	// we can handle 24 bits of direct color:
	if (InfoHeader.biBitCount == 24)
	{
		rewind(fp);
		fseek(fp, FileHeader.bfOffBytes, SEEK_SET);
		int t;
		unsigned char* tp;
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (int s = 0; s < nums; s++, tp += 3)
			{
				*(tp + 2) = fgetc(fp);		// b
				*(tp + 1) = fgetc(fp);		// g
				*(tp + 0) = fgetc(fp);		// r
			}

			for (int e = 0; e < numExtra; e++)
			{
				fgetc(fp);
			}
		}
	}

	// we can also handle 8 bits of indirect color:
	if (InfoHeader.biBitCount == 8 && InfoHeader.biClrUsed == 256)
	{
		struct rgba32
		{
			unsigned char r, g, b, a;
		};
		struct rgba32* colorTable = new struct rgba32[InfoHeader.biClrUsed];

		rewind(fp);
		fseek(fp, sizeof(struct bmfh) + InfoHeader.biSize - 2, SEEK_SET);
		for (int c = 0; c < InfoHeader.biClrUsed; c++)
		{
			colorTable[c].r = fgetc(fp);
			colorTable[c].g = fgetc(fp);
			colorTable[c].b = fgetc(fp);
			colorTable[c].a = fgetc(fp);
			if (VERBOSE)	fprintf(stderr, "%4d:\t0x%02x\t0x%02x\t0x%02x\t0x%02x\n",
				c, colorTable[c].r, colorTable[c].g, colorTable[c].b, colorTable[c].a);
		}

		rewind(fp);
		fseek(fp, FileHeader.bfOffBytes, SEEK_SET);
		int t;
		unsigned char* tp;
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (int s = 0; s < nums; s++, tp += 3)
			{
				int index = fgetc(fp);
				*(tp + 0) = colorTable[index].r;	// r
				*(tp + 1) = colorTable[index].g;	// g
				*(tp + 2) = colorTable[index].b;	// b
			}

			for (int e = 0; e < numExtra; e++)
			{
				fgetc(fp);
			}
		}

		delete[] colorTable;
	}

	fclose(fp);

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt(FILE* fp)
{
	const unsigned char b0 = fgetc(fp);
	const unsigned char b1 = fgetc(fp);
	const unsigned char b2 = fgetc(fp);
	const unsigned char b3 = fgetc(fp);
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

short
ReadShort(FILE* fp)
{
	const unsigned char b0 = fgetc(fp);
	const unsigned char b1 = fgetc(fp);
	return (b1 << 8) | b0;
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb(float hsv[3], float rgb[3])
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while (h >= 6.)	h -= 6.;
	while (h < 0.) 	h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;

	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:

	float i = (float)floor(h);
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - s * f);
	float t = v * (1.f - (s * (1.f - f)));

	float r = 0., g = 0., b = 0.;			// red, green, blue
	switch ((int)i)
	{
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}
