
////////////////////////////////////////////////////////////////////////////
//		  Author: Paul Hutchinson										  //
//	  Assignment: Final Project											  //
//		   Class: CSE 386-A												  //
//	   Professor: Dr. Zmuda												  //
//			Date: December 6, 2012										  //
////////////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <GL/glut.h>
#include <windows.h>
#include <math.h>
#include "Airplane.h"
#include "Grass.h"
#include "Rock.h"
#include "Billboard.h"

typedef void (*ObjectFunc)(void);
double timerInterval = 0.05;
double interval = (int)(timerInterval*1000);
double fx = 79.134;			// x position
double fy = 20;				// y position
double fz;					// z position
double posx = -250;			// x position of viewer
double posy = 20;			// y position of viewer
double posz = 0;			// z position of viewer
double xDir = 0;			// x direction of viewing
double yDir = 0;			// y direction of viewing
double zDir;
double vertAngle;
double moveDir = 0;		// direction of movement (degrees from table center)
double angle;
double speed = 15;			// inches/s
double xFlashlight;			// x position of flashlight
double zFlashlight;			// z position of flashlight
double propAngle = 0;
double propSpeed = 60;
float white[4] = {1,1,1,1};
float black[4] = {0,0,0,1};
float gray[4] = {0.5,0.5,0.5,1};
float fps = 0;
const int NUM_TEXS = 2;
const int W = 200;
const int H = 300;
int runwayLength = 200;
int xAirplane = 300;
int AirplaneSpeed = 10;
int AirplaneDir = 0;
int window_x;
int window_y;
int window_width = 700;
int window_height = 700;
int timeInterval;
int frameCount = 0;
int currentTime = 0, previousTime = 0;
bool flashlightOn = false;	// flashlight on/off
bool binocularsOn = false;
bool imageSaved = false;
bool firstDisplay = true;
GLuint texIDs[NUM_TEXS];
unsigned char image[W*H][3];
unsigned char alpha = 190;
unsigned char bg[] = {	// green
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
	49,  76,  47, alpha,
};

/*
	Creates an instance of an object and performs transformations on it (without color).
*/
void InstanceTransformationWOColor(	GLfloat sx, GLfloat sy, GLfloat sz,
								GLfloat dx, GLfloat dy, GLfloat dz,
								GLfloat deg, GLfloat vx, GLfloat vy, GLfloat vz,
								ObjectFunc func)	{
	glPushMatrix();
	glTranslatef(dx, dy, dz);
	glRotatef(deg, vx, vy, vz);
	glScalef(sx, sy, sz);
	(*func)();
	glPopMatrix();
}

/*
	Converts degrees into radians.
*/
GLdouble d2r(GLdouble theta)	{
	return 3.14159265 / 180 * theta;
}

/*
	Loads a texture into the GL texture object from .raw image formats.
*/
GLuint LoadTextureRAW( const char * filename, int wrap, int dim)	{
	GLuint texture;
	int width, height;
	BYTE * data;
	FILE * file;
	file = std::fopen( filename, "rb" );	// open texture data
	if ( file == NULL ) return 0;
	width = dim;	// allocate buffer
	height = dim;
	data = (BYTE*)malloc( width * height * 3 );
	fread( data, width * height * 3, 1, file );	// read texture data
	fclose( file );
	glGenTextures( 1, &texture );	// allocate a texture name
	glBindTexture( GL_TEXTURE_2D, texture );	// select current texture
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                   GL_LINEAR_MIPMAP_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                   wrap ? GL_REPEAT : GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                   wrap ? GL_REPEAT : GL_CLAMP );
	gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width,
    height, GL_RGB, GL_UNSIGNED_BYTE, data );// build our texture MIP maps
	free( data );		 // free buffer
	return texture;
}

/*
	Prepares the perspective spotlight (flashlight).
*/
void prepareLight1()	{
	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	const float pos[] = {posx, 10, posz, 1};
	if (xDir > 90 || xDir < -90)	{
		xFlashlight = -1;
		zFlashlight = d2r(xDir);	// ?
	} else {
		xFlashlight = 1;
		zFlashlight = d2r(xDir);
	}
	const float dir[] = { xFlashlight , d2r(yDir), zFlashlight, 1 };
	const float white[] = { 1, 1, 1, 1 };
	glLightfv(GL_LIGHT1, GL_POSITION, pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT1, GL_SPECULAR, white);
	glLightfv(GL_LIGHT1, GL_AMBIENT, white);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, dir);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 25);	//25
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 16);
}

/*
	Draws a sphere to represent the sky.
*/
void drawSphere(double r, int lats, int longs) {
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	GLuint texture = LoadTextureRAW("Textures/redsky.raw", TRUE, 512);
	int i, j;
	for(i = 0; i <= lats; i++) {
		double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
        double z0  = sin(lat0);
        double zr0 =  cos(lat0);    
        double lat1 = M_PI * (-0.5 + (double) i / lats);
        double z1 = sin(lat1);
        double zr1 = cos(lat1);
		glBegin(GL_QUAD_STRIP);
			for(j = 0; j <= longs; j++) {
				double lng = 2 * M_PI * (double) (j - 1) / longs;
				double x = cos(lng);
				double y = sin(lng);
				glTexCoord2d((x * zr0), (y * zr0));
				glTexCoord2d((x * zr1), (y * zr1));
				glNormal3f(x * zr0, y * zr0, z0);
				glVertex3f(x * zr0, y * zr0, z0);
				glNormal3f(x * zr1, y * zr1, z1);
				glVertex3f(x * zr1, y * zr1, z1);
           }
           glEnd();
    }
	glDeleteTextures(1, &texture);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Draws the sky using the sphere method.
*/
void Sky() {
	glPushMatrix();
	//	glRotatef(90,1,0,0);
		glTranslatef(0,-10,0);
		glScalef(500,500,500);
		drawSphere(1,100,100);
	glPopMatrix();
 }

/*
	Draws a ground surface with a dirt texture.
*/
void Dirt(void)	{
	glEnable(GL_TEXTURE_2D);
	GLuint texture = LoadTextureRAW("Textures/dirt.raw", TRUE,512);
	glBegin(GL_QUADS);
		glTexCoord2d(0,1);	glVertex3f(-500,0,-500);
		glTexCoord2d(0,-1);	glVertex3f(500,0, -500);
		glTexCoord2d(1,-1);	glVertex3f(500,0, 500);
		glTexCoord2d(1, 1);	glVertex3f(-500,0, 500);
	glEnd();
	glDeleteTextures(1, &texture);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Draws a patch of grass.
*/
void GrassPatch(void)	{
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		GLuint texture = LoadTextureRAW("Textures/grass.raw", TRUE,256);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, Grass_02Verts);
		glNormalPointer(GL_FLOAT, 0,  Grass_02Normals);
		glTexCoordPointer(2, GL_FLOAT, 0,  Grass_02TexCoords);
		// draw arrays
		glScalef(200,200,200);
		glDrawArrays(GL_TRIANGLES, 0,  Grass_02NumVerts);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &texture);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Creates the ground using the dirt and grasspatch methods.
*/
void Ground(void)	{
	InstanceTransformationWOColor(1,1,1, 0,0,0, 0,0,0,0, Dirt);
	InstanceTransformationWOColor(1,0.3,1, -100,0,-190, 0,0,0,0, GrassPatch);
	InstanceTransformationWOColor(1,0.3,1, 0,0,-190, 0,0,0,0, GrassPatch);
	InstanceTransformationWOColor(1,0.3,1, 100,0,-190, 0,0,0,0, GrassPatch);
	InstanceTransformationWOColor(1,0.3,1, -100,0,190, 0,0,0,0, GrassPatch);
	InstanceTransformationWOColor(1,0.3,1, 0,0,	  190, 0,0,0,0, GrassPatch);
	InstanceTransformationWOColor(1,0.3,1, 100,0, 190, 0,0,0,0, GrassPatch);
}

/*
	Creates a segment of road to be placed in the scene.
*/
void RunwaySegment(void)	{
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	GLuint texture = LoadTextureRAW("Textures/asphault.raw", TRUE,256);	//256
	glBegin(GL_QUADS);
		glTexCoord2d(0,1);	glVertex3f(0,0,100);
		glTexCoord2d(0,-1);	glVertex3f(0,0,-100);
		glTexCoord2d(1,-1);	glVertex3f(runwayLength,0,-100);
		glTexCoord2d(1, 1);	glVertex3f(runwayLength,0, 100);
	glEnd();
	glDeleteTextures(1, &texture);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Creates a road through the scene using the RunwaySegment method.
*/
void Runway(void)	{
	InstanceTransformationWOColor(1,1,1, -runwayLength*3,0,0, 0,0,0,0, RunwaySegment);
	InstanceTransformationWOColor(1,1,1, -runwayLength*2,0,0, 0,0,0,0, RunwaySegment);
	InstanceTransformationWOColor(1,1,1, -runwayLength,0,0, 0,0,0,0, RunwaySegment);
	InstanceTransformationWOColor(1,1,1, 0,0,0, 0,0,0,0, RunwaySegment);
	InstanceTransformationWOColor(1,1,1, runwayLength,0,0, 0,0,0,0, RunwaySegment);
	InstanceTransformationWOColor(1,1,1, runwayLength*2,0,0, 0,0,0,0, RunwaySegment);
}

/*
	Renders the wiring between the wings of the airplane.
*/
void AirplaneWiring(void)	{
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		GLuint texture = LoadTextureRAW("Textures/brown.raw", TRUE,128);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, AirplaneVerts);
		glNormalPointer(GL_FLOAT, 0, AirplaneNormals);
		glTexCoordPointer(2, GL_FLOAT, 0, AirplaneTexCoords);
		// draw arrays
		glScalef(200,200,200);
		glDrawArrays(GL_TRIANGLES, 0, 1000);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &texture);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Renders the airplane's three propellors (two large in the front, one small in the back-left.
*/
void AirplanePropellors(void)	{
		glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		GLuint texture = LoadTextureRAW("Textures/grey.raw", TRUE,64);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, AirplaneVerts);
		glNormalPointer(GL_FLOAT, 0, AirplaneNormals);
		glTexCoordPointer(2, GL_FLOAT, 0, AirplaneTexCoords);
		// draw arrays
		glScalef(200,200,200);
		glDrawArrays(GL_TRIANGLES, 0, 5000);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &texture);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Renders the wings of the airplane.
*/
void AirplaneWings(void)	{
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		GLuint texture = LoadTextureRAW("Textures/brown.raw", TRUE,64);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, AirplaneVerts);
		glNormalPointer(GL_FLOAT, 0, AirplaneNormals);
		glTexCoordPointer(2, GL_FLOAT, 0, AirplaneTexCoords);
		// draw arrays
		glScalef(200,200,200);
		glDrawArrays(GL_TRIANGLES, 0, 11200);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &texture);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Renders the body of the airplane.
*/
void AirplaneBody(void)	{
	// Yellow plastic material colors.
	const GLfloat ambientColor[4] = {0,0,0,1};
	const GLfloat diffuseColor[4] = {0.5,0.5,0,1};
	const GLfloat specularColor[4] = {0.6,0.6,0.5,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		GLuint texture = LoadTextureRAW("Textures/yellow.raw", TRUE,64);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, AirplaneVerts);
		glNormalPointer(GL_FLOAT, 0, AirplaneNormals);
		glTexCoordPointer(2, GL_FLOAT, 0, AirplaneTexCoords);
		// draw arrays
		glScalef(200,200,200);
		glDrawArrays(GL_TRIANGLES, 0, 13500);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &texture);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Renders the nose of the airplane.
*/
void AirplaneNose(void)	{
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		GLuint texture = LoadTextureRAW("Textures/brown.raw", TRUE,64);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, AirplaneVerts);
		glNormalPointer(GL_FLOAT, 0, AirplaneNormals);
		glTexCoordPointer(2, GL_FLOAT, 0, AirplaneTexCoords);
		// draw arrays
		glScalef(200,200,200);
		glDrawArrays(GL_TRIANGLES, 0, 14000);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &texture);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Renders the transparent windows of the airplane.
*/
void AirplaneWindows(void)	{
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, AirplaneVerts);
		glNormalPointer(GL_FLOAT, 0, AirplaneNormals);
		glTexCoordPointer(2, GL_FLOAT, 0, AirplaneTexCoords);
		// draw arrays
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texIDs[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glEnable(GL_BLEND);
		glScalef(200,200,200);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_TRIANGLES, 0, 15000);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	glPopMatrix();
}

/*
	Renders the two large engines of the airplane.
*/
void AirplaneEngine(void)	{
	// Yellow plastic material colors.
	const GLfloat ambientColor[4] = {0,0,0,1};
	const GLfloat diffuseColor[4] = {0.5,0.5,0,1};
	const GLfloat specularColor[4] = {0.6,0.6,0.5,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		GLuint texture = LoadTextureRAW("Textures/yellow.raw", TRUE,64);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, AirplaneVerts);
		glNormalPointer(GL_FLOAT, 0, AirplaneNormals);
		glTexCoordPointer(2, GL_FLOAT, 0, AirplaneTexCoords);
		// draw arrays
		glScalef(200,200,200);
		glDrawArrays(GL_TRIANGLES, 0, AirplaneNumVerts-1000);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &texture);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Renders the wheels of the airplane.
*/
void AirplaneWheels(void)	{
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		GLuint texture = LoadTextureRAW("Textures/black.raw", TRUE,64);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, AirplaneVerts);
		glNormalPointer(GL_FLOAT, 0, AirplaneNormals);
		glTexCoordPointer(2, GL_FLOAT, 0, AirplaneTexCoords);
		// draw arrays
		glScalef(200,200,200);
		glDrawArrays(GL_TRIANGLES, 0, AirplaneNumVerts);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &texture);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

/*
	Renders the image of the billboard, updated by pressing 'i' or 'I'.
*/
void BillboardImage(void)	{
	if (imageSaved) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texIDs[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBegin(GL_POLYGON);
			glTexCoord2d(0,0); glVertex2f(-20,-20);
			glTexCoord2d(1,0); glVertex2f(20, -20);
			glTexCoord2d(1,1); glVertex2f(20, 20);
			glTexCoord2d(0,1); glVertex2f(-20, 20);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	glPushMatrix();
	glRectf(-20,-20, 20, 20);
	glPopMatrix();
}

/*
	Creates a billboard from a .obj file and creates the billboard using the BillboardImage method.
*/
void Billboard(void)	{
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		GLuint texture = LoadTextureRAW("Textures/grey.raw", TRUE,64);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, Billboard2Verts);
		glTexCoordPointer(2, GL_FLOAT, 0, Billboard2TexCoords);
		// draw arrays
		glScalef(80,80,80);
		glDrawArrays(GL_TRIANGLES, 0, Billboard2NumVerts);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
		glDeleteTextures(1, &texture);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	InstanceTransformationWOColor(1.8,1,1, -3,2,12.5, 0,0,0,0, BillboardImage);
}

/*
	Renders the brown square for the decal on the tail of the airplane.
*/
void Square(void)	{
	glColor3f(0.24,0.19,0.2);
	glBegin(GL_QUADS);
		glVertex2f(0,0);
		glVertex2f(0,5);
		glVertex2f(5,5);
		glVertex2f(5,0);
	glEnd();
	glColor3f(1,1,1);
}

/*
	Captures the stencil (blue dot on the decal).
*/
void captureStencil(void)	{
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0x05, 0x05);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glColor3f(0,0,1);
	gluDisk(gluNewQuadric(), 0, 2, 32,32);
	glColor3f(1,1,1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glDisable(GL_STENCIL_TEST);
	glFlush();
}

/*
	Creates the decal (brown square with a blue dot) for the tail of the plane using the Square and captureStencil methods.
*/
void Decal(void)	{
	glClear(GL_STENCIL_BUFFER_BIT);
	glPushMatrix();
		glRotatef(90,0,1,0);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		Square();
		InstanceTransformationWOColor(1,1,1, 2.5,2.5,0, 0,0,0,0, captureStencil);
		glDepthMask(GL_TRUE);		
		glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

	glPopMatrix();
}

/*
	Creates an airplane from the different airplane compenents.
*/
void Airplane(void)	{
	AirplaneWiring();
	AirplanePropellors();
	AirplaneWings();
	AirplaneBody();
	AirplaneNose();
	AirplaneWindows();
	AirplaneEngine();
	AirplaneWheels();
	InstanceTransformationWOColor(1,1,1, -1.5,-4.5,-63, -12,1,0,0, Decal);
	InstanceTransformationWOColor(1,1,1, -0.4,-4.5,-63, -12,1,0,0, Decal);
}

/*
	Renders rocks for the scenary.
*/
void Rocks(void)	{
	glPushMatrix();
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_TEXTURE_COORD_ARRAY);
		// set input data to arrays
		glVertexPointer(3, GL_FLOAT, 0, RockVerts);
		glNormalPointer(GL_FLOAT, 0, RockNormals);
		glTexCoordPointer(2, GL_FLOAT, 0, RockTexCoords);
		// draw arrays
		glScalef(200,200,200);
		glDrawArrays(GL_TRIANGLES, 0, RockNumVerts);
		glDisable(GL_VERTEX_ARRAY);
		glDisable(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();
}

/*
	Calculates the frames per second (FPS) of the rendered scene (for testing and experimentation).
*/
void calculateFPS()	{
    frameCount++;
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    //  Calculate time passed
    timeInterval = currentTime - previousTime;
    if(timeInterval > 1000)	{
        //  calculate the number of frames per second
        fps = frameCount / (timeInterval / 1000.0f);
        previousTime = currentTime;
        frameCount = 0;
    }
}

/*
	Writes the FPS reading to the console.
*/
void drawFPS()	{
	calculateFPS();
	std::cout<<fps<<std::endl;
}

/*
	Centers the window to the screen.
*/
void centerOnScreen ()	{
	window_x = (glutGet (GLUT_SCREEN_WIDTH) - window_width)/2;
	window_y = (glutGet (GLUT_SCREEN_HEIGHT) - window_height)/2;
}

/*
	The display method for OpenGL.
*/
void display(void)	{
	//drawFPS();	// for testing/experimentation
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
	gluPerspective(180, GLdouble(glutGet(GLUT_WINDOW_WIDTH))/glutGet(GLUT_WINDOW_WIDTH),3, 1000);
	glLoadIdentity();
	if (abs(xDir) == 360)	{
		xDir = 0;
	}
	angle = d2r(xDir);
	fx = posx+cos(angle);
	fz = posz+sin(angle);
	vertAngle = d2r(yDir);
	gluLookAt(posx, posy, posz, fx, fy, fz, 0, 1, 0);
	fy = posy+sin(vertAngle);
	if (xAirplane < -500)	{
		AirplaneDir = 1;
	} else if (xAirplane > 500) {
		AirplaneDir = 0;
	}
	if (AirplaneDir == 1)	{
		InstanceTransformationWOColor(1,1,1, xAirplane,100,0, 90,0,1,0, Airplane);
	} else if (AirplaneDir == 0)	{
		InstanceTransformationWOColor(1,1,1, xAirplane,100,0, -90,0,1,0, Airplane);
	}
	glAccum(GL_MULT, 0.90);
	glAccum(GL_ACCUM, 0.10);
	glAccum(GL_RETURN, 1);
	InstanceTransformationWOColor(1,1,1, 50,30,-140, -60,0,1,0, Billboard);
	InstanceTransformationWOColor(.5,.5,.5, -150,10,0, -135,0,1,0, Airplane);
	InstanceTransformationWOColor(.5,.5,.5, 100,10,0, 135,0,1,0, Airplane);
	InstanceTransformationWOColor(.5,.5,.5, 0,0,0, 0,0,0,0, Sky);
	InstanceTransformationWOColor(.5,.5,.5, 0,-1,0, 0,0,0,0, Ground);
	InstanceTransformationWOColor(.5,.5,.5, 0,0,0, 0,0,0,0, Runway);

//	InstanceTransformationWOColor(1,1,1, -100,-1,-70, 0,0,0,0, Rocks);

	if (flashlightOn)	{
		prepareLight1();	// perspective spotlight
	} else {
		glDisable(GL_LIGHT1);
	}
	glFlush();
	glutSwapBuffers();
}

/*
	Controls the movements of objects in the scene.
*/
void animate(void)	{
	posx += cos(d2r(moveDir)) * (speed*timerInterval);
	posz += sin(d2r(moveDir)) * (speed*timerInterval);
	// Propellor
	propAngle+=propSpeed;
	// Flying Airplane
	if (AirplaneDir == 0)	{
		xAirplane -= AirplaneSpeed;
	} else {
		AirplaneDir = 1;
		xAirplane += AirplaneSpeed;
	}
}

/*
	Reads user input from the keyboard (see console on startup for controls).
*/
void keyboard(unsigned char key, int x, int y)	{
	 if (key == 'c' || key == 'C')	{			// change direction
		moveDir = xDir;
	} else if (key == 'r' || key == 'R')	{	// reset head to look straight
		xDir = moveDir;
		fy = 20;
		yDir = 0;
	} else if (key == 'w' || key == 'W')	{	// start or stop walking
		if (speed == 0)	{						// if stopped, start walking at default speed
			speed = 15;			
		} else if (speed == 15 || speed == 30) {	// if walking, stop walking
			speed = 0;
		}
	} else if (key == 's' || key == 'S') {
		if (flashlightOn)	{
			flashlightOn = false;
		} else {
			flashlightOn = true;
		}
	} else if (key == 'f' || key == 'F') {
		if (speed == 15)	{
			speed = 30;
		} else if (speed == 30)	{
			speed = 15;
		}
	} else if (key == 'b' || key == 'B')	{
		if (binocularsOn)	{
			binocularsOn = false;
		} else	{
			binocularsOn = true;
		}
	} else if(key == 'i' || key == 'I') {
		glReadPixels(20, 100, W, H, GL_RGB, GL_UNSIGNED_BYTE, image);
		imageSaved = true;
		glutPostRedisplay();
		glBindTexture(GL_TEXTURE_2D, texIDs[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}
}

/*
	Reads special user input from the keyboard (see console on startup for controls).
*/
void special(int key, int x, int y)	{
	static const double inc = 10;
	if (key == GLUT_KEY_LEFT) {
		xDir -= inc;
	} else if (key == GLUT_KEY_RIGHT) {
		xDir += inc;
	} else if (key == GLUT_KEY_UP)	{
		yDir += inc;
	} else if (key == GLUT_KEY_DOWN)	{
		yDir -= inc;
	}
}		

/*
	Reshape method modified from class example "Robot3D.cpp".

*/
void myReshape(int w, int h)	{
	glClearAccum(0.0, 0.0, 0.0, 1.0);
	glClear(GL_ACCUM_BUFFER_BIT);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, 1, 1.0, 1000); //500
}

/*
	Idle function, used instead of a timer for smoother animation results/better FPS.
*/
void idle (void)	{
	animate();
    calculateFPS();
    glutPostRedisplay ();
}

/*
	Initializes the texture for the airplane windows.
*/
void init()	{    
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(NUM_TEXS, texIDs);
	glBindTexture(GL_TEXTURE_2D, texIDs[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 20, 0, GL_RGBA, GL_UNSIGNED_BYTE, bg);
}

/*
	Main method for starting the program.
*/
void main(int argc, char **argv)	{
	std::cout <<"CONTROLS"<<std::endl;
	std::cout <<"  left and right arrow keys: change horizontal direction"<<std::endl;  
	std::cout <<"     up and down arrow keys: change elevation direction"<<std::endl; 
	std::cout <<"		     c or C: change direction of movement"<<std::endl;
	std::cout <<"		     r or R: reset head to look straight ahead"<<std::endl;
	std::cout <<"		     w or W: start or stop the viewer walking"<<std::endl;
	std::cout <<"		     s or S: toggle flashlight"<<std::endl;
	std::cout <<"		     f or F: toggle fast movement (run)"<<std::endl;
	std::cout <<"		     i or I: update billboard image with current view"<<std::endl;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(window_width, window_height);
	centerOnScreen ();
	glutInitWindowPosition(window_x, window_y);
	glutCreateWindow("Final Project");
	init();
	glutReshapeFunc(myReshape);
	glutDisplayFunc(display);
//	glutTimerFunc(interval, move, 1);
	glutIdleFunc(idle);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);	// Basic lighting and shading stuff
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glLightfv(GL_LIGHT0, GL_AMBIENT, black);
	glEnable(GL_FOG);
	glFogfv(GL_FOG_COLOR, white);
	glFogf(GL_FOG_START, 300);
	glFogf(GL_FOG_END, 500);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glutMainLoop();
}
