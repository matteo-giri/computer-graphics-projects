/*
 * Lab-01_students.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks.
 *
 * Usage:
 *   Left click to place a control point.
 *		Maximum number of control points allowed is currently set at 64.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
 */


#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <stdlib.h>

 // Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


#define DBOUT( s )            \
{                             \
   std::ostringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}

static unsigned int programId;

unsigned int VAO;
unsigned int VBO;

unsigned int VAO_2;
unsigned int VBO_2;

unsigned int VAO_3;
unsigned int VBO_3;

using namespace glm;

#define MaxNumPts 2000
#define Steps 100
float PointArray[MaxNumPts][2]; //array of control points
float CurveArray[MaxNumPts][2]; //array of curve points

int NumPts = 0;

// Parameters for drag feature
#define POINT_RADIUS 0.03
int selectedIndex = -1;

// Parameters for adaptive suddivision
#define tol_planarita 0.005
int numeroTratti = 0;

// Parameters for catmull-rom spline
float HiddenPointArray[MaxNumPts][2];
int tsize = 100;

// Parameters for continuity spline
float c = 0; //continuity parameter
float continuityArray[MaxNumPts]; //array where to store the continuity of each point

// Window size in pixels
int		width = 500;
int		height = 500;

/* Prototypes */
void addNewPoint(float x, float y);
int main(int argc, char** argv);
void removeFirstPoint();
void removeLastPoint();


void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'f':
		removeFirstPoint();
		glutPostRedisplay();
		break;
	case 'l':
		removeLastPoint();
		glutPostRedisplay();
		break;
	case '0':
		c = 1;
		glutPostRedisplay();
		break;
	case '1':
		c = 0;
		glutPostRedisplay();
		break;
	case '2':
		c = -1;
		glutPostRedisplay();
		break;
	case 27:			// Escape key
		exit(0);
		break;
	}
}
void removeFirstPoint() {
	int i;
	if (NumPts > 0) {
		// Remove the first point, slide the rest down
		NumPts--;
		for (i = 0; i < NumPts; i++) {
			PointArray[i][0] = PointArray[i + 1][0];
			PointArray[i][1] = PointArray[i + 1][1];

			continuityArray[i] = continuityArray[i + 1];
		}
	}
}
void resizeWindow(int w, int h)
{
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

//function to know if a clicked position in the window is already a control point or not
int pointAlreadyExists(float x, float y) 
{
	for (int i = 0; i < NumPts; i++)
	{
		if (abs(PointArray[i][0] - x) <= POINT_RADIUS && abs(PointArray[i][1] - y) <= POINT_RADIUS)
		{
			return i;
		}
	}
	return -1;
}

// Left button presses place a new control point.
void myMouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		// (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		selectedIndex = pointAlreadyExists(xPos, yPos);
		if (selectedIndex == -1) {
			addNewPoint(xPos, yPos);
			glutPostRedisplay();
		}
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		selectedIndex = -1;
	}
}

void myMouseMotion(int x, int y) {

	if (selectedIndex != -1) // If we have a selected control point
	{
		//change the coordinates of the control point to be the ones of the mouse
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		PointArray[selectedIndex][0] = xPos;
		PointArray[selectedIndex][1] = yPos;
		glutPostRedisplay();
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void removeLastPoint() {
	if (NumPts > 0) {
		NumPts--;
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void addNewPoint(float x, float y) {
	if (NumPts >= MaxNumPts) {
		removeFirstPoint();
	}
	PointArray[NumPts][0] = x;
	PointArray[NumPts][1] = y;

	continuityArray[NumPts] = c;

	NumPts++;
}
void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}


void init(void)
{
	// VAO for control polygon
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// VAO for curve
	glGenVertexArrays(1, &VAO_2);
	glBindVertexArray(VAO_2);
	glGenBuffers(1, &VBO_2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
	// VAO for hidden control points
	glGenVertexArrays(1, &VAO_3);
	glBindVertexArray(VAO_3);
	glGenBuffers(1, &VBO_3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3);

	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, 500, 500);
}

//function that applies deCasteljau method to create a Bezier curve
void deCasteljau(float tempArray[MaxNumPts][2],float t,float (&result)[3],float NumPts) {
	float pi0[MaxNumPts][2]; //output array
	int n = NumPts-1; //curve degree

	for (int i = 0; i < MaxNumPts; i++) { //copy the array
		pi0[i][0] = tempArray[i][0];
		pi0[i][1] = tempArray[i][1];
	}
	for (int i = 1; i <= n;i++) {
		for (int j = 0; j <= n-i; j++) {
			pi0[j][0] = (1 - t)*pi0[j][0] + t * pi0[j+1][0];
			pi0[j][1] = (1 - t)*pi0[j][1] + t * pi0[j + 1][1];
		}
	}
	result[0] = pi0[0][0];
	result[1] = pi0[0][1];
}

//function that applies deCasteljau method to split a curve
void deCasteljauSplit(float tempArray[MaxNumPts][2], float t, int NumPts, float (&curve1)[MaxNumPts][2], float(&curve2)[MaxNumPts][2]) {
	float pi0[MaxNumPts][2];
	int n = NumPts - 1; //curve degree

	for (int i = 0; i < MaxNumPts; i++) { //copy the array
		pi0[i][0] = tempArray[i][0];
		pi0[i][1] = tempArray[i][1];
	}

	//the first control point of the curve becomes the first control point of curve1, while the last control point becomes the last of curve2
	curve1[0][0] = pi0[0][0];
	curve1[0][1] = pi0[0][1];
	curve2[NumPts - 1][0] = pi0[NumPts - 1][0];
	curve2[NumPts - 1][1] = pi0[NumPts - 1][1];

	for (int i = 1; i <= n; i++) {
		for (int j = 0; j <= n - i; j++) {
			pi0[j][0] = (1 - t)*pi0[j][0] + t * pi0[j + 1][0];
			pi0[j][1] = (1 - t)*pi0[j][1] + t * pi0[j + 1][1];
			//if its the first lerp of the iteration I save the point to the first curve
			if (j == 0) {
				curve1[i][0] = pi0[j][0];
				curve1[i][1] = pi0[j][1];
			}
			//if its the last lerp of the iteration I save the point to the second curve
			if (j == n - i) {
				curve2[NumPts-i-1][0] = pi0[j][0];
				curve2[NumPts-1-i][1] = pi0[j][1];
			}
		}
	}
}

float distanza_punto_retta(float x0, float y0, float m, float b) {
	float distanza = abs(m * x0 - y0 + b) / sqrt(m * m + 1);
	return distanza;
}

//CALCULATE BEZIER CURVE (UNIFORM METHOD)
void uniformBezierCurve() {
	float result[3]; //result point array

	// for each step call deCasteljau algorithm
	for (int i = 0; i <= Steps; i++) {
		deCasteljau(PointArray,(GLfloat)i / Steps, result,NumPts);

		//put the results in the array
		CurveArray[i][0] = result[0];
		CurveArray[i][1] = result[1];
	}
}

//CALCULATE BEZIER CURVE (ADAPTIVE SUBDIVISION METHOD)
void adaptiveBezierCurve(float tempArray[MaxNumPts][2],int NumPts) {
	//Extract external control points
	float p1[2];
	float p2[2];
	for (int i = 0; i < 2; i++) {
		p1[i] = tempArray[0][i];
		p2[i] = tempArray[NumPts - 1][i];
	}

	//Calculate segment between p1 and p2
	float m = (p2[1] - p1[1]) / (p2[0] - p1[0]);
	float b = p1[1] - m * p1[0];

	//flatness test
	int test_planarita = 1;
	for (int i = 1; i < NumPts - 1; i++) {
		float distanza = distanza_punto_retta(tempArray[i][0], tempArray[i][1], m, b);
		if (distanza > tol_planarita)
			test_planarita = 0;
	}

	//if flatness test is good we can draw the segment between p1 and p2
	if (test_planarita == 1) {
		CurveArray[numeroTratti][0] = p1[0];
		CurveArray[numeroTratti][1] = p1[1];
		CurveArray[numeroTratti+1][0] = p2[0];
		CurveArray[numeroTratti+1][1] = p2[1];
		numeroTratti++;
	}
	//else split curve into 2 and repeat process for both
	else {
		float subd_1[MaxNumPts][2];
		float subd_2[MaxNumPts][2];

		deCasteljauSplit(tempArray, 0.5, NumPts, subd_1, subd_2);

		adaptiveBezierCurve(subd_1, NumPts);
		adaptiveBezierCurve(subd_2, NumPts);
	}
}

//CALCULATE INTERP0LATING BEZIER CURVE (CATMULL-ROM SPLINE)
void catmullRomBezierCurve(float tempArray[MaxNumPts][2], int NumPts) {
	float m1[2]; //mi
	float m2[2]; //mi+1
	float p1[2]; //unknown pi plus
	float p2[2]; //unknown pi+1 minus
	for (int i = 0; i < NumPts-1; i++) {
		//calculate mi and mi+1
		if (i == 0) { //if the control point is the first one
			m1[0] = (tempArray[i + 1][0] - tempArray[i][0]) / 2;
			m1[1] = (tempArray[i + 1][1] - tempArray[i][1]) / 2;

			m2[0] = (tempArray[i + 2][0] - tempArray[i][0]) / 2;
			m2[1] = (tempArray[i + 2][1] - tempArray[i][1]) / 2;
		}

		else if (i == NumPts - 2) { //if the control point is the second to last one
			m1[0] = (tempArray[i + 1][0] - tempArray[i-1][0]) / 2;
			m1[1] = (tempArray[i + 1][1] - tempArray[i-1][1]) / 2;

			m2[0] = (tempArray[i + 1][0] - tempArray[i][0]) / 2;
			m2[1] = (tempArray[i + 1][1] - tempArray[i][1]) / 2;
		}
		else { //all the other control points
			m1[0] = (tempArray[i + 1][0] - tempArray[i - 1][0]) / 2;
			m1[1] = (tempArray[i + 1][1] - tempArray[i - 1][1]) / 2;

			m2[0] = (tempArray[i + 2][0] - tempArray[i][0]) / 2;
			m2[1] = (tempArray[i + 2][1] - tempArray[i][1]) / 2;
		}

		//calculate unknown control points:
		p1[0] = tempArray[i][0] + (1.0 / 3.0)*m1[0];
		p1[1] = tempArray[i][1] + (1.0 / 3.0)*m1[1];
		p2[0] = tempArray[i+1][0] - (1.0 / 3.0)*m2[0];
		p2[1] = tempArray[i+1][1] - (1.0 / 3.0)*m2[1];

		//add all 4 control points (pi, pi+1 and the 2 unknowns) in a array
		HiddenPointArray[0][0] = tempArray[i][0];
		HiddenPointArray[0][1] = tempArray[i][1];
		HiddenPointArray[1][0] = p1[0];
		HiddenPointArray[1][1] = p1[1];
		HiddenPointArray[2][0] = p2[0];
		HiddenPointArray[2][1] = p2[1];
		HiddenPointArray[3][0] = tempArray[i+1][0];
		HiddenPointArray[3][1] = tempArray[i+1][1];

		//to draw the hidden control points
		glBindVertexArray(VAO_3);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_3);
		glBufferData(GL_ARRAY_BUFFER, sizeof(HiddenPointArray), &HiddenPointArray[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Draw the control points CP
		glPointSize(3.0);
		glDrawArrays(GL_POINTS, 0, 3);
		glBindVertexArray(0);

		//deCasteljau
		float result[3];		
		for (int j = 0; j <= tsize; j++) {
			deCasteljau(HiddenPointArray, (GLfloat)j / tsize, result,4);
			CurveArray[tsize*i+j][0] = result[0];
			CurveArray[tsize*i+j][1] = result[1];
		}
	}
}

//CALCULATE PIECEWISE BEZIER CURVE WITH CONTINUITY SELECTION
void continuitySplineBezierCurve(float tempArray[MaxNumPts][2], int NumPts) {
	float ri[2]; //ri of current point
	float li[2]; //li of next point
	float p1[2]; //unknown pi plus
	float p2[2]; //unknown pi+1 minus
	for (int i = 0; i < NumPts - 1; i++) {
		//set current continuity value
		float currentC = continuityArray[i + 1];
		float nextC = i == NumPts - 2 ? continuityArray[i + 1] : continuityArray[i + 2];
		//calculate ri and li
		if (i == 0) { //if the control point is the first one
			ri[0] = (1 - currentC)*(tempArray[i + 1][0] - tempArray[i][0])*0.5;
			ri[1] = (1 - currentC)*(tempArray[i + 1][1] - tempArray[i][1])*0.5;

			li[0] = (1 - nextC)*(tempArray[i + 1][0] - tempArray[i][0])*0.5 + (1 + nextC)*(tempArray[i + 2][0] - tempArray[i + 1][0])*0.5;
			li[1] = (1 - nextC)*(tempArray[i + 1][1] - tempArray[i][1])*0.5 + (1 + nextC)*(tempArray[i + 2][1] - tempArray[i + 1][1])*0.5;
		}

		else if (i == NumPts - 2) { //if the control point is the second to last one
			ri[0] = (1 + currentC)*(tempArray[i][0] - tempArray[i - 1][0])*0.5 + (1 - currentC)*(tempArray[i + 1][0] - tempArray[i][0])*0.5;
			ri[1] = (1 + currentC)*(tempArray[i][1] - tempArray[i - 1][1])*0.5 + (1 - currentC)*(tempArray[i + 1][1] - tempArray[i][1])*0.5;

			li[0] = (1 - nextC)*(tempArray[i + 1][0] - tempArray[i][0])*0.5;
			li[1] = (1 - nextC)*(tempArray[i + 1][1] - tempArray[i][1])*0.5;
		}
		else { //all the other control points
			ri[0] = (1 + currentC)*(tempArray[i][0] - tempArray[i - 1][0])*0.5 + (1 - currentC)*(tempArray[i + 1][0] - tempArray[i][0])*0.5;
			ri[1] = (1 + currentC)*(tempArray[i][1] - tempArray[i - 1][1])*0.5 + (1 - currentC)*(tempArray[i + 1][1] - tempArray[i][1])*0.5;

			li[0] = (1 - nextC)*(tempArray[i + 1][0] - tempArray[i][0])*0.5 + (1 + nextC)*(tempArray[i + 2][0] - tempArray[i + 1][0])*0.5;
			li[1] = (1 - nextC)*(tempArray[i + 1][1] - tempArray[i][1])*0.5 + (1 + nextC)*(tempArray[i + 2][1] - tempArray[i + 1][1])*0.5;
		}

		//calculate unknown control points:
		p1[0] = tempArray[i][0] + (1.0 / 3.0)*ri[0];
		p1[1] = tempArray[i][1] + (1.0 / 3.0)*ri[1];
		p2[0] = tempArray[i + 1][0] - (1.0 / 3.0)*li[0];
		p2[1] = tempArray[i + 1][1] - (1.0 / 3.0)*li[1];

		//add all 4 control points (pi, pi+1 and the 2 unknowns) in a array
		HiddenPointArray[0][0] = tempArray[i][0];
		HiddenPointArray[0][1] = tempArray[i][1];
		HiddenPointArray[1][0] = p1[0];
		HiddenPointArray[1][1] = p1[1];
		HiddenPointArray[2][0] = p2[0];
		HiddenPointArray[2][1] = p2[1];
		HiddenPointArray[3][0] = tempArray[i + 1][0];
		HiddenPointArray[3][1] = tempArray[i + 1][1];

		//for drawing hidden control points
		glBindVertexArray(VAO_3);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_3);
		glBufferData(GL_ARRAY_BUFFER, sizeof(HiddenPointArray), &HiddenPointArray[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Draw the control points CP
		glPointSize(3.0);
		glDrawArrays(GL_POINTS, 0, 3);
		glBindVertexArray(0);

		//deCasteljau
		float result[3];
		for (int j = 0; j <= tsize; j++) {
			deCasteljau(HiddenPointArray, (GLfloat)j / tsize, result, 4);
			CurveArray[tsize*i + j][0] = result[0];
			CurveArray[tsize*i + j][1] = result[1];
		}
	}
}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (NumPts > 2) {

		//for uniform bezier curve
		uniformBezierCurve();

		//for adaptive bezier curve
		//numeroTratti = 0;
		//adaptiveBezierCurve(PointArray, NumPts);

		//for catmull-rom spline curve
		//catmullRomBezierCurve(PointArray, NumPts);

		//for continuity spline curve
		//continuitySplineBezierCurve(PointArray, NumPts);

		//draw curve
		glBindVertexArray(VAO_2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glLineWidth(0.5);

		glDrawArrays(GL_LINE_STRIP, 0, 101); //for uniform bezier curve
		//glDrawArrays(GL_LINE_STRIP, 0, numeroTratti + 1); //for adaptive bezier curve		
		//glDrawArrays(GL_LINE_STRIP, 0, (NumPts-1)*tsize+1); //for catmull-rom spline curve
		//glDrawArrays(GL_LINE_STRIP, 0, (NumPts - 1)*tsize + 1); //for continuity spline curve

		glBindVertexArray(0);
	}
	
	// Draw control polygon
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointArray), &PointArray[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Draw the control points CP
	glPointSize(6.0);
	glDrawArrays(GL_POINTS, 0, NumPts);
	// Draw the line segments between CP
	glLineWidth(2.0);
	glDrawArrays(GL_LINE_STRIP, 0, NumPts);
	glBindVertexArray(0);
	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Draw curves 2D");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resizeWindow);
	glutKeyboardFunc(myKeyboardFunc);
	glutMouseFunc(myMouseFunc);
	glutMotionFunc(myMouseMotion);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glutMainLoop();
}
