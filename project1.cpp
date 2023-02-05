#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cessna.550>


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

const char* WINDOWTITLE = "OpenGL / GLUT Sample -- Joe Graphics";
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
GLuint	BoxList;				// object display list
GLuint  lighthouseList;
GLuint  RockList;
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:

void	Animate();
void	Display();
void	DoAxesMenu(int);
void	DoColorMenu(int);
void	DoDepthBufferMenu(int);
void	DoDepthFightingMenu(int);
void	DoDepthMenu(int);
void	DoDebugMenu(int);
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

	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);


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


	// draw the box object by calling up its display list:

	//glCallList( BoxList );
	glCallList(lighthouseList);
	glCallList(RockList);

	if (DepthFightingOn != 0)
	{
		glPushMatrix();
		glRotatef(90., 0., 1., 0.);
		glCallList(lighthouseList);
		glPopMatrix();
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
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

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

	glutIdleFunc(Animate);

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


	int offset = -3;
	int segments = 8;
	float radius = 1.f;
	float slope = .07f;
	float dang = 2. * M_PI / (float)(segments -1);
	float ang = 0.;
	float h1, h2;

	glutSetWindow(MainWindow);

	lighthouseList = glGenLists(1);
	glNewList(lighthouseList, GL_COMPILE);

	//Base section
	glColor3f(1., 0., 0.);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, offset, 0);
	for (int i = 0; i < segments-4; i++)
	{
		glVertex3f((radius)*cos(ang), offset, (radius)*sin(ang));
		ang += dang;
	}
	glEnd();

	ang = 0.;
	for (int i = 0; i < 9; i++)
	{
		if (i % 2 == 0) {
			glColor3f(1., 0., 0.);
		}
		else {
			glColor3f(1., 1., 0.);
		}

		ang = 0.;
		h1 = float(i) / 2 + offset;
		h2 = float(i + 1) / 2 + offset;

		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j < segments; j++)
		{
			glVertex3f(radius * cos(ang), h1, radius * sin(ang));
			glVertex3f((radius - slope) * cos(ang), h2, (radius - slope) * sin(ang));
			ang += dang;
		}
		radius -= slope;
		glEnd();
	}

	//Light cover
	glColor3f(1.5, 1., 1.);
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < segments; i++)
	{
		glVertex3f(radius * cos(ang), 4 + offset, radius * sin(ang));
		glVertex3f((radius)*cos(ang), 4.7f + offset, (radius)*sin(ang));

		ang += dang;
	}
	ang = 0.;
	glEnd();

	//Roof section
	glColor3f(1., 1., 1.);
	for (int j = 0; j < 2; j++) {
		glBegin(GL_TRIANGLE_FAN);
		if (j == 1) {
			glVertex3f(0, 4.7 + offset, 0);
		}
		else {
			glVertex3f(0, 5 + offset, 0);
		}
		for (int i = 0; i <= segments; i++)
		{
			glVertex3f((radius + .2) * cos(ang), 4.7 + offset, (radius + .2) * sin(ang));
			ang += dang;
		}
		glEnd();
	}

	glEndList();

	RockList = glGenLists(1);
	glNewList(RockList, GL_COMPILE);
	glTranslatef(0., offset - .4, 0.);
	glScalef(3., 3., 3.);
	glBegin(GL_TRIANGLES);

	glColor3f(0.384277135133743, 0.399005770683289, 0.410938382148743);
	glNormal3f(-0.697185337543488, 0.702534079551697, 0.142753481864929);
	glVertex3f(-0.873154997825623, -0.121425211429596, 0.705753386020660);
	glVertex3f(-0.728239715099335, 0.011649668216705, 0.758593916893005);
	glVertex3f(-0.844372272491455, -0.058297812938690, 0.535654187202454);

	glNormal3f(-0.275874644517899, 0.950226604938507, -0.144785106182098);
	glVertex3f(-0.617226600646973, -0.129174619913101, -0.155518159270287);
	glVertex3f(-0.308615535497665, -0.035711884498596, -0.130150809884071);
	glVertex3f(-0.440998286008835, -0.109151184558868, -0.359890818595886);

	glNormal3f(-0.236974149942398, 0.888893306255341, -0.392061293125153);
	glVertex3f(-0.617226600646973, -0.129174619913101, -0.155518159270287);
	glVertex3f(-0.4574754238128655, 0.021477319300175, 0.089485868811607);
	glVertex3f(-0.308615535497665, -0.035711884498596, -0.130150809884071);

	glNormal3f(0.265318065881729, 0.817649364471436, 0.510936260223389);
	glVertex3f(0.000284023582935, -0.029501378536224, 0.956667363643646);
	glVertex3f(0.266146779060364, -0.069089293479919, 0.881963014602661);
	glVertex3f(0.138556614518166, 0.097998976707458, 0.680827081203461);

	glNormal3f(-0.254978775978088, 0.930317997932434, -0.263617426156998);
	glVertex3f(-0.800457298755646, -0.065555125474930, 0.355866074562073);
	glVertex3f(-0.535489082336426, 0.006905481219292, 0.355297565460205);
	glVertex3f(-0.705398797988892, -0.112276889383793, 0.099039383232594);

	glNormal3f(-0.255004584789276, 0.930415272712708, -0.263249009847641);
	glVertex3f(-0.800457298755646, -0.065555125474930, 0.355866074562073);
	glVertex3f(-0.686746001243591, 0.035275697708130, 0.602087855339050);
	glVertex3f(-0.535489082336426, 0.006905481219292, 0.355297565460205);


	glNormal3f(-0.006703592836857, 0.993714988231659, 0.111737892031670);
	glVertex3f(-0.538170158863068, 0.051156759262085, 0.695012271404266);
	glVertex3f(-0.274486958980560, 0.048274993896484, 0.736459970474243);
	glVertex3f(-0.410304963588715, 0.063634157180786, 0.591718554496765);

	glNormal3f(-0.407259464263916, 0.803778290748596, -0.433682054281235);
	glVertex3f(-0.267326295375824, 0.014238059520721, -0.588641405105591);
	glVertex3f(0.003707100404426, 0.136133491992950, -0.617243051528931);
	glVertex3f(-0.134560599923134, 0.013881325721741, -0.713979363441467);

	glNormal3f(-0.417950093746185, 0.899609982967377, -0.126569002866745);
	glVertex3f(-0.267326295375824, 0.014238059520721, -0.588641405105591);
	glVertex3f(-0.130372121930122, 0.109528183937073, -0.363594710826874);
	glVertex3f(0.003707100404426, 0.136133491992950, -0.617243051528931);

	glNormal3f(-0.058513760566711, 0.895848929882050, 0.440489083528519);
	glVertex3f(0.001831938861869, 0.083842873573303, -0.110991865396500);
	glVertex3f(0.327808022499084, 0.134485572576523, -0.170684874057770);
	glVertex3f(0.151239693164825, 0.232734441757202, -0.393954455852509);

	glNormal3f(-0.120589986443520, 0.977851569652557, 0.171068474650383);
	glVertex3f(0.001831938861869, 0.083842873573303, -0.110991865396500);
	glVertex3f(0.132537111639977, 0.058789346367121, 0.124354645609856);
	glVertex3f(0.327808022499084, 0.134485572576523, -0.170684874057770);

	glNormal3f(-0.001068071462214, 0.994318306446075, 0.106442123651505);
	glVertex3f(0.001831938861869, 0.083842873573303, -0.110991865396500);
	glVertex3f(-0.132485613226891, 0.058497447520494, 0.124422073364258);
	glVertex3f(0.132537111639977, 0.058789346367121, 0.124354645609856);


	glNormal3f(-0.239463165402412, 0.962694525718689, 0.126002952456474);
	glVertex3f(-0.267154514789581, 0.075991526246071, 0.355222880840302);
	glVertex3f(-0.135981529951096, 0.068181872367859, 0.664179205894470);
	glVertex3f(-0.000000030267984, 0.140909090638161, 0.366951823234558);


	glNormal3f(0.127671673893929, 0.991396725177765, 0.028852814808488);
	glVertex3f(0.267153948545456, 0.075988337397575, 0.355221986770630);
	glVertex3f(0.545251667499542, 0.040196463465691, 0.354485273361206);
	glVertex3f(0.467098861932755, 0.058060307055712, 0.086495622992516);

	glNormal3f(0.127382636070251, 0.987930715084076, 0.088127441704273);
	glVertex3f(0.267153948545456, 0.075988337397575, 0.355221986770630);
	glVertex3f(0.405852496623993, 0.036627203226089, 0.595990240573883);
	glVertex3f(0.545251667499542, 0.040196463465691, 0.354485273361206);

	glNormal3f(0.165698945522308, 0.946633815765381, 0.276456594467163);
	glVertex3f(0.532166600227356, -0.076863169670105, 0.814927935600281);
	glVertex3f(0.704617142677307, -0.091265559196472, 0.760882973670959);
	glVertex3f(0.675445377826691, -0.036268025636673, 0.590046763420105);

	glNormal3f(-0.458255261182785, 0.876527428627014, -0.147315084934235);
	glVertex3f(-0.844372272491455, -0.058297812938690, 0.535654187202454);
	glVertex3f(-0.686746001243591, 0.035275697708130, 0.602087855339050);
	glVertex3f(-0.800457298755646, -0.065555125474930, 0.355866074562073);

	glNormal3f(-0.508911073207855, 0.860804617404938, -0.004978803917766);
	glVertex3f(-0.844372272491455, -0.058297812938690, 0.535654187202454);
	glVertex3f(-0.728239715099335, 0.011649668216705, 0.758593916893005);
	glVertex3f(-0.686746001243591, 0.035275697708130, 0.602087855339050);

	glNormal3f(-0.169246912002563, 0.980166912078857, 0.103093571960926);
	glVertex3f(-0.728239715099335, 0.011649668216705, 0.758593916893005);
	glVertex3f(-0.538170158863068, 0.051156759262085, 0.695012271404266);
	glVertex3f(-0.686746001243591, 0.035275697708130, 0.602087855339050);

	glNormal3f(-0.575615882873535, 0.817710220813751, 0.004058169666678);
	glVertex3f(-0.440998286008835, -0.109151184558868, -0.359890818595886);
	glVertex3f(-0.130372121930122, 0.109528183937073, -0.363594710826874);
	glVertex3f(-0.267326295375824, 0.014238059520721, -0.588641405105591);

	glNormal3f(-0.573696792125702, 0.816096305847168, 0.069704972207546);
	glVertex3f(-0.440998286008835, -0.109151184558868, -0.359890818595886);
	glVertex3f(-0.308615535497665, -0.035711884498596, -0.130150809884071);
	glVertex3f(-0.130372121930122, 0.109528183937073, -0.363594710826874);

	glNormal3f(-0.360054284334183, 0.890247344970703, 0.278963267803192);
	glVertex3f(-0.308615535497665, -0.035711884498596, -0.130150809884071);
	glVertex3f(0.001831938861869, 0.083842873573303, -0.110991865396500);
	glVertex3f(-0.130372121930122, 0.109528183937073, -0.363594710826874);

	glNormal3f(-0.352129399776459, 0.930515289306641, -0.100728318095207);
	glVertex3f(-0.308615535497665, -0.035711884498596, -0.130150809884071);
	glVertex3f(-0.132485613226891, 0.058497447520494, 0.124422073364258);
	glVertex3f(0.001831938861869, 0.083842873573303, -0.110991865396500);

	glNormal3f(-0.076171375811100, 0.951120316982269, -0.299279153347015);
	glVertex3f(-0.308615535497665, -0.035711884498596, -0.130150809884071);
	glVertex3f(-0.457475423812866, 0.021477319300175, 0.089485868811607);
	glVertex3f(-0.132485613226891, 0.058497447520494, 0.124422073364258);

	glNormal3f(-0.098164767026901, 0.986371397972107, -0.132042005658150);
	glVertex3f(-0.457475423812866, 0.021477319300175, 0.089485868811607);
	glVertex3f(-0.267154514789581, 0.075991526246071, 0.355222880840302);
	glVertex3f(-0.132485613226891, 0.058497447520494, 0.124422073364258);

	glNormal3f(0.231436595320702, 0.972511351108551, 0.025664644315839);
	glVertex3f(0.138556614518166, 0.097998976707458, 0.680827081203461);
	glVertex3f(0.405852496623993, 0.036627203226089, 0.595990240573883);
	glVertex3f(0.267153948545456, 0.075988337397575, 0.355221986770630);

	glNormal3f(0.335947602987289, 0.818192481994629, 0.466583728790283);
	glVertex3f(0.138556614518166, 0.097998976707458, 0.680827081203461);
	glVertex3f(0.266146779060364, -0.069089293479919, 0.881963014602661);
	glVertex3f(0.405852496623993, 0.036627203226089, 0.595990240573883);

	glNormal3f(0.126808851957321, 0.908644020557404, 0.397851079702377);
	glVertex3f(0.266146779060364, -0.069089293479919, 0.881963014602661);
	glVertex3f(0.532166600227356, -0.076863169670105, 0.814927935600281);
	glVertex3f(0.405852496623993, 0.036627203226089, 0.595990240573883);

	glNormal3f(-0.469767361879349, 0.855072498321533, -0.219475746154785);
	glVertex3f(-0.705398797988892, -0.112276889383793, 0.099039383232594);
	glVertex3f(-0.457475423812866, 0.021477319300175, 0.089485868811607);
	glVertex3f(-0.617226600646973, -0.129174619913101, -0.155518159270287);

	glNormal3f(-0.475542396306992, 0.874910175800323, -0.091605268418789);
	glVertex3f(-0.705398797988892, -0.112276889383793, 0.099039383232594);
	glVertex3f(-0.535489082336426, 0.006905481219292, 0.355297565460205);
	glVertex3f(-0.457475423812866, 0.021477319300175, 0.089485868811607);

	glNormal3f(-0.249286189675331, 0.968221604824066, -0.020085489377379);
	glVertex3f(-0.535489082336426, 0.006905481219292, 0.355297565460205);
	glVertex3f(-0.267154514789581, 0.075991526246071, 0.355222880840302);
	glVertex3f(-0.457475423812866, 0.021477319300175, 0.089485868811607);

	glNormal3f(-0.248111695051193, 0.963573575019836, -0.099832974374294);
	glVertex3f(-0.535489082336426, 0.006905481219292, 0.355297565460205);
	glVertex3f(-0.410304963588715, 0.063634157180786, 0.591718554496765);
	glVertex3f(-0.267154514789581, 0.075991526246071, 0.355222880840302);

	glNormal3f(-0.107024684548378, 0.978185057640076, -0.178044080734253);
	glVertex3f(-0.535489082336426, 0.006905481219292, 0.355297565460205);
	glVertex3f(-0.686746001243591, 0.035275697708130, 0.602087855339050);
	glVertex3f(-0.410304963588715, 0.063634157180786, 0.591718554496765);

	glNormal3f(-0.102286182343960, 0.994734048843384, -0.006458763498813);
	glVertex3f(-0.686746001243591, 0.035275697708130, 0.602087855339050);
	glVertex3f(-0.538170158863068, 0.051156759262085, 0.695012271404266);
	glVertex3f(-0.410304963588715, 0.063634157180786, 0.591718554496765);

	glNormal3f(-0.026165891438723, 0.998996138572693, 0.036361359059811);
	glVertex3f(-0.410304963588715, 0.063634157180786, 0.591718554496765);
	glVertex3f(-0.135981529951096, 0.068181872367859, 0.664179205894470);
	glVertex3f(-0.267154514789581, 0.075991526246071, 0.355222880840302);

	glNormal3f(-0.058445975184441, 0.985479772090912, 0.159416437149048);
	glVertex3f(-0.410304963588715, 0.063634157180786, 0.591718554496765);
	glVertex3f(-0.274486958980560, 0.048274993896484, 0.736459970474243);
	glVertex3f(-0.135981529951096, 0.068181872367859, 0.664179205894470);

	glNormal3f(0.023391352966428, 0.951465070247650, 0.306866347789764);
	glVertex3f(-0.274486958980560, 0.048274993896484, 0.736459970474243);
	glVertex3f(0.000284023582935, -0.029501378536224, 0.956667363643646);
	glVertex3f(-0.135981529951096, 0.068181872367859, 0.664179205894470);

	glNormal3f(0.011405524797738, 0.742421448230743, -0.669836103916168);
	glVertex3f(-0.134560599923134, 0.013881325721741, -0.713979363441467);
	glVertex3f(0.135580912232399, 0.010457456111908, -0.713174462318420);
	glVertex3f(-0.001287056598812, -0.081205606460571, -0.817100882530212);

	glNormal3f(0.010127616114914, 0.613418161869049, -0.789693355560303);
	glVertex3f(-0.134560599923134, 0.013881325721741, -0.713979363441467);
	glVertex3f(0.003707100404426, 0.136133491992950, -0.617243051528931);
	glVertex3f(0.135580912232399, 0.010457456111908, -0.713174462318420);

	glNormal3f(0.148274645209312, 0.693645715713501, -0.704890131950378);
	glVertex3f(0.003707100404426, 0.136133491992950, -0.617243051528931);
	glVertex3f(0.277214229106903, 0.088261485099792, -0.606818795204163);
	glVertex3f(0.135580912232399, 0.010457456111908, -0.713174462318420);

	glNormal3f(0.168783187866211, 0.859093606472015, -0.483187586069107);
	glVertex3f(0.003707100404426, 0.136133491992950, -0.617243051528931);
	glVertex3f(0.151239693164825, 0.232734441757202, -0.393954455852509);
	glVertex3f(0.277214229106903, 0.088261485099792, -0.606818795204163);

	glNormal3f(-0.408805847167969, 0.904535055160522, -0.121218755841255);
	glVertex3f(0.003707100404426, 0.136133491992950, -0.617243051528931);
	glVertex3f(-0.130372121930122, 0.109528183937073, -0.363594710826874);
	glVertex3f(0.151239693164825, 0.232734441757202, -0.393954455852509);

	glColor3f(0., 0., 1.);
	glNormal3f(-0.359557241201401, 0.890520989894867, 0.278730958700180);
	glVertex3f(-0.130372121930122, 0.109528183937073, -0.363594710826874);
	glVertex3f(0.001831938861869, 0.083842873573303, -0.110991865396500);
	glVertex3f(0.151239693164825, 0.232734441757202, -0.393954455852509);

	glNormal3f(0.424748182296753, 0.845840811729431, -0.322710990905762);
	glVertex3f(0.151239693164825, 0.232734441757202, -0.393954455852509);
	glVertex3f(0.460782915353775, 0.073588967323303, -0.403665363788605);
	glVertex3f(0.277214229106903, 0.088261485099792, -0.606818795204163);

	glNormal3f(0.457765817642212, 0.888599514961243, 0.029009578749537);
	glVertex3f(0.151239693164825, 0.232734441757202, -0.393954455852509);
	glVertex3f(0.327808022499084, 0.134485572576523, -0.170684874057770);
	glVertex3f(0.460782915353775, 0.073588967323303, -0.403665363788605);

	glNormal3f(0.613739907741547, 0.775594174861908, 0.147570058703423);
	glVertex3f(0.327808022499084, 0.134485572576523, -0.170684874057770);
	glVertex3f(0.621830642223358, -0.099825918674469, -0.162029832601547);
	glVertex3f(0.460782915353775, 0.073588967323303, -0.403665363788605);

	glNormal3f(0.621617019176483, 0.776110827922821, -0.106038644909859);
	glVertex3f(0.327808022499084, 0.134485572576523, -0.170684874057770);
	glVertex3f(0.467098861932755, 0.058060307055712, 0.086495622992516);
	glVertex3f(0.621830642223358, -0.099825918674469, -0.162029832601547);

	glNormal3f(0.032481238245964, 0.962732136249542, 0.268499016761780);
	glVertex3f(0.327808022499084, 0.134485572576523, -0.170684874057770);
	glVertex3f(0.132537111639977, 0.058789346367121, 0.124354645609856);
	glVertex3f(0.467098861932755, 0.058060307055712, 0.086495622992516);

	glNormal3f(-0.005849236622453, 0.997466444969177, -0.070897899568081);
	glVertex3f(0.132537111639977, 0.058789346367121, 0.124354645609856);
	glVertex3f(0.267153948545456, 0.075988337397575, 0.355221986770630);
	glVertex3f(0.467098861932755, 0.058060307055712, 0.086495622992516);

	glNormal3f(0.222968772053719, 0.953863322734833, -0.201071500778198);
	glVertex3f(0.132537111639977, 0.058789346367121, 0.124354645609856);
	glVertex3f(-0.000000030267984, 0.140909090638161, 0.366951823234558);
	glVertex3f(0.267153948545456, 0.075988337397575, 0.355221986770630);

	glNormal3f(-0.001124770264141, 0.947016835212708, -0.321182042360306);
	glVertex3f(0.132537111639977, 0.058789346367121, 0.124354645609856);
	glVertex3f(-0.132485613226891, 0.058497447520494, 0.124422073364258);
	glVertex3f(-0.000000030267984, 0.140909090638161, 0.366951823234558);

	glNormal3f(-0.222846522927284, 0.953629970550537, -0.202310279011726);
	glVertex3f(-0.132485613226891, 0.058497447520494, 0.124422073364258);
	glVertex3f(-0.267154514789581, 0.075991526246071, 0.355222880840302);
	glVertex3f(-0.000000030267984, 0.140909090638161, 0.366951823234558);

	glNormal3f(0.237205967307091, 0.971054553985596, 0.028041711077094);
	glVertex3f(-0.000000030267984, 0.140909090638161, 0.366951823234558);
	glVertex3f(0.138556614518166, 0.097998976707458, 0.680827081203461);
	glVertex3f(0.267153948545456, 0.075988337397575, 0.355221986770630);

	glNormal3f(-0.117195084691048, 0.975702106952667, 0.185123383998871);
	glVertex3f(-0.000000030267984, 0.140909090638161, 0.366951823234558);
	glVertex3f(-0.135981529951096, 0.068181872367859, 0.664179205894470);
	glVertex3f(0.138556614518166, 0.097998976707458, 0.680827081203461);

	glNormal3f(-0.122375048696995, 0.922843575477600, 0.365217775106430);
	glVertex3f(-0.135981529951096, 0.068181872367859, 0.664179205894470);
	glVertex3f(0.000284023582935, -0.029501378536224, 0.956667363643646);
	glVertex3f(0.138556614518166, 0.097998976707458, 0.680827081203461);

	glNormal3f(0.415121912956238, 0.862483859062195, -0.289474457502365);
	glVertex3f(0.467098861932755, 0.058060307055712, 0.086495622992516);
	glVertex3f(0.719712615013123, -0.059754729270935, 0.097729556262493);
	glVertex3f(0.621830642223358, -0.099825918674469, -0.162029832601547);

	glNormal3f(0.424139916896820, 0.903369605541229, -0.063472859561443);
	glVertex3f(0.467098861932755, 0.058060307055712, 0.086495622992516);
	glVertex3f(0.545251667499542, 0.040196463465691, 0.354485273361206);
	glVertex3f(0.719712615013123, -0.059754729270935, 0.097729556262493);

	glNormal3f(0.249548733234406, 0.947624385356903, -0.199332192540169);
	glVertex3f(0.545251667499542, 0.040196463465691, 0.354485273361206);
	glVertex3f(0.811047971248627, -0.029342934489250, 0.356652021408081);
	glVertex3f(0.719712615013123, -0.059754729270935, 0.097729556262493);

	glNormal3f(0.248003467917442, 0.953299939632416, 0.172375783324242);
	glVertex3f(0.545251667499542, 0.040196463465691, 0.354485273361206);
	glVertex3f(0.675445377826691, -0.036268025636673, 0.590046763420105);
	glVertex3f(0.811047971248627, -0.029342934489250, 0.356652021408081);

	glNormal3f(0.260836154222488, 0.951244473457336, 0.164616018533707);
	glVertex3f(0.545251667499542, 0.040196463465691, 0.354485273361206);
	glVertex3f(0.405852496623993, 0.036627203226089, 0.595990240573883);
	glVertex3f(0.675445377826691, -0.036268025636673, 0.590046763420105);

	glNormal3f(0.253455132246017, 0.910796403884888, 0.325899451971054);
	glVertex3f(0.405852496623993, 0.036627203226089, 0.595990240573883);
	glVertex3f(0.532166600227356, -0.076863169670105, 0.814927935600281);
	glVertex3f(0.675445377826691, -0.036268025636673, 0.590046763420105);

	glNormal3f(0.277302533388138, 0.941997051239014, 0.189063221216202);
	glVertex3f(0.675445377826691, -0.036268025636673, 0.590046763420105);
	glVertex3f(0.842458128929138, -0.073968470096588, 0.532926440238953);
	glVertex3f(0.811047971248627, -0.029342934489250, 0.356652021408081);

	glNormal3f(0.293053418397903, 0.923564255237579, 0.247282862663269);
	glVertex3f(0.675445377826691, -0.036268025636673, 0.590046763420105);
	glVertex3f(0.704617142677307, -0.091265559196472, 0.760882973670959);
	glVertex3f(0.842458128929138, -0.073968470096588, 0.532926440238953);

	glNormal3f(0.419736325740814, 0.850002408027649, 0.318303823471069);
	glVertex3f(0.704617142677307, -0.091265559196472, 0.760882973670959);
	glVertex3f(0.867598712444305, -0.149593919515610, 0.701725542545319);
	glVertex3f(0.842458128929138, -0.073968470096588, 0.532926440238953);

	glEnd();


	glEndList();



	// create the axes:

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
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
