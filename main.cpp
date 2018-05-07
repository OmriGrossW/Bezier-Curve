#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include "glut.h"

using namespace std;

typedef struct point
{
	GLfloat x;
	GLfloat y;
}point;

vector<point> points;
GLuint sceneWidth, sceneHeight, mouseStartX, mouseStartY;
GLfloat perspectiveMatrix[16] = { 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, 0.0f , 0.0f, 0.0f };
bool play, pressed, renderBezier, nWasNotSet;	
int numOfBezierCurves, leftToNextCurve, n;
GLfloat dimmingFactor;

void reshape(int width, int height);
void display();
void disp(int value);
void init();
void initValues();

void drawControlPointsLines();
void drawBezierCurve(int startIndex);
point calcCubicBezierPoint(int startIndex, GLfloat t);
point calcQuadraticBezierPoint(int startIndex, GLfloat t);

void addPoint(GLfloat destX, GLfloat destY);
void addImplicitPoint();

void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);

void subdivide_n_times();
void subdivideCasteljau();
void addLeftRightCurves(vector<point> * new_points, point * original_points);

void PrintAllPoints();

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE);
	sceneWidth = 700;
	sceneHeight = 700;
	glutInitWindowSize(sceneWidth, sceneHeight);
	glutInitWindowPosition(400, 150);
	glViewport(0, 0, (GLsizei)sceneWidth, (GLsizei)sceneHeight);

	glutCreateWindow("Assignment5");
	
	if (argc > 1) {
		nWasNotSet = false;
		sscanf(argv[1], "%d", &n);
	}
	else						// User did not specify value for n, Manual mode, using 'space' key to increment n
		nWasNotSet = true;

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(2, disp, 0);

	glutMainLoop();

	return 0;
}

void disp(int value) {
	if (play)
		glutPostRedisplay();
	glutTimerFunc(5, disp, 0);
}

void display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(perspectiveMatrix);

	glMatrixMode(GL_MODELVIEW);

	drawControlPointsLines();
	if (renderBezier) {
		for (int i = 0; i < numOfBezierCurves; i++)
			drawBezierCurve(i * 3);
	}
	glFlush();
}

void reshape(int width, int height)
{
	sceneWidth = width;
	sceneHeight = height;
	glViewport(0, 0, (GLsizei)sceneWidth, (GLsizei)sceneHeight);
}

void init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluOrtho2D(0.0, sceneWidth, 0.0, sceneHeight);
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	glGetFloatv(GL_PROJECTION_MATRIX, perspectiveMatrix);


	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	glPointSize(5.0f);
	glLineWidth(1.0f);
	play = true;
	pressed = false;
	initValues();
}

void initValues()
{
	renderBezier = false;
	numOfBezierCurves = 0;
	leftToNextCurve = 3;
	dimmingFactor = 1.0f;
	if (nWasNotSet) 
		n = 0;
}
void drawControlPointsLines()
{
	glColor3f(dimmingFactor * 1.0, 0.0, 0.0);
	if (points.size() == 0) return;
	if (points.size() == 1) {
		glBegin(GL_POINTS);
		glVertex3f(points.at(0).x, points.at(0).y, 0.0f);
		glEnd();
	}
	else {
		// Drawing control lines
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < points.size(); i++)
			glVertex3f(points.at(i).x, points.at(i).y, 0.0f);
		glEnd();

		// Drawing control points
		glBegin(GL_POINTS);
		for (int i = 0; i < points.size(); i++) {
			if (i % 3 == 0 && points.size() - i > 1 && i != 0 ) {	// Implicit point
				if (renderBezier) {	// Draw only on render mode
					glColor3f(dimmingFactor * 1.0, dimmingFactor * 1.0, dimmingFactor * 1.0);	// Implicit point, changing color
					glVertex3f(points.at(i).x, points.at(i).y, 0.0f);
					glColor3f(dimmingFactor * 1.0, 0.0, 0.0);
				}
			}
			else 
				glVertex3f(points.at(i).x, points.at(i).y, 0.0f);

		}
		glEnd();
	}
}

void drawBezierCurve(int startIndex) {
	int i = startIndex;
	point a = points.at(i);

	// Drawing bezier curves
	glColor3f(0.1, 0.5, 1.0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(a.x, a.y, 0.0f);
	for (GLfloat t = 0.0; t <= 1.0; t += 0.001)
	{
		if (points.size() - i > 3)
			a = calcCubicBezierPoint(i, t);
		else			// if there are only 3 points in the last bezier curve we use quadratic system
			a = calcQuadraticBezierPoint(i, t);
		glVertex3f(a.x, a.y, 0.0f);
	}
	glEnd();

	// Drawing green lines from end to end of each curve to get approximation of original curve
	glColor3f(0.0, 0.7, 0.0);
	glBegin(GL_LINES);
	a = points.at(i);
	glVertex3f(a.x, a.y, 0.0f);
	if (points.size() - i > 3)
		a = points.at(i + 3);
	else		//  if there are only 3 points in the last bezier curve
		a = points.at(i + 2);

	glVertex3f(a.x, a.y, 0.0f);
	glEnd();

}

point calcCubicBezierPoint(int startIndex, GLfloat t)
{
	int i = startIndex;
	point bezierPoint;

	bezierPoint.x = powf((1.0f - t), 3.0f) * points.at(i).x + 3.0f * t * powf((1.0f - t), 2.0f) * points.at(i + 1).x + 3.0f * (1.0f - t) * powf(t, 2.0f)* points.at(i + 2).x + powf(t, 3.0f)* points.at(i + 3).x;
	bezierPoint.y = powf((1.0f - t), 3.0f) * points.at(i).y + 3.0f * t * powf((1.0f - t), 2.0f) * points.at(i + 1).y + 3.0f * (1.0f - t) * powf(t, 2.0f)* points.at(i + 2).y + powf(t, 3.0f)* points.at(i + 3).y;
	return bezierPoint;
}

point calcQuadraticBezierPoint(int startIndex, GLfloat t)
{
	int i = startIndex;
	point bezierPoint;

	bezierPoint.x = powf((1.0f - t), 2.0f) * points.at(i).x + 2.0f * t * (1.0f - t) * points.at(i + 1).x + +powf(t, 2.0f)* points.at(i + 2).x;
	bezierPoint.y = powf((1.0f - t), 2.0f) * points.at(i).y + 2.0f * t * (1.0f - t) * points.at(i + 1).y + +powf(t, 2.0f)* points.at(i + 2).y;
	return bezierPoint;
}

void addPoint(GLfloat destX, GLfloat destY)
{
	point tmpPoint;

	leftToNextCurve--;
	if (leftToNextCurve == 0)
	{
		numOfBezierCurves++;
		leftToNextCurve = 2;
		if (numOfBezierCurves > 1) {
			addImplicitPoint();
		}
	}


	tmpPoint.x = destX;
	tmpPoint.y = destY;
	points.push_back(tmpPoint);
	//printf("Push %f,%f\n", points.at(points.size() - 1).x, points.at(points.size() - 1).y);
}

void addImplicitPoint()
{
	point implicitPoint;
	vector<point>::iterator it = points.end();

	implicitPoint.x = (points.at(points.size() - 1).x + points.at(points.size() - 2).x) / 2.0f;
	implicitPoint.y = (points.at(points.size() - 1).y + points.at(points.size() - 2).y) / 2.0f;

	it--;
	points.insert(it, 1, implicitPoint);
}

void mouse(int button, int state, int x, int y) {
	if (pressed && !renderBezier) {	// On mouse release, mouse button up
		GLfloat x_d = (GLfloat)(x - mouseStartX) / (GLfloat)sceneWidth;
		GLfloat y_d = (GLfloat)(y - mouseStartY) / (GLfloat)sceneHeight;

		printf("Mouse pressed at : X,Y  %d,%d\n\t\tx_d = %f , x_y = %f\n", x, y, (float)x_d, (float)y_d);

		switch (button) {
		case GLUT_LEFT_BUTTON:
			GLfloat distFromStart = (GLfloat)x / (GLfloat)sceneWidth;
			GLfloat destX, destY;

			// Calc destX
			if (distFromStart >= 0.5f)
				destX = (distFromStart - 0.5f) * 2.0f;
			else
				destX = ((0.5f - distFromStart) * -1.0f) * 2.0f;

			// Calc destY
			distFromStart = (GLfloat)y / (GLfloat)sceneHeight;
			if (distFromStart >= 0.5f)
				destY = (distFromStart - 0.5f) * -2.0f;
			else
				destY = (0.5f - distFromStart)  * 2.0f;


			addPoint(destX, destY);
			break;

		}
	}
	else {		// On mouse click, button down
		mouseStartX = x;
		mouseStartY = y;
	}
	pressed = !pressed;
}

void keyboard(unsigned char key, int x, int y) {
	if (pressed)	// if mouse is pressed ignore keyboard action
		return;

	if (key == 'c' || key == 'C') {	// Clear All
		points.clear();
		initValues();
	}
	if (key == 'd' || key == 'D') {	// Render Bezier
		subdivide_n_times();
		renderBezier = true;
		dimmingFactor = 0.37f;
	}
	if (nWasNotSet && key == 32) { //space
		n++;
		printf("n = %d, number of subCurves per original curve: %d\n", n, (int)powf(2.0f , (float)n));
		if (renderBezier)
			subdivideCasteljau();
	}
}

void subdivide_n_times() {
	for (int i = 0; i < n; i++) 
		subdivideCasteljau();
}

void subdivideCasteljau() {
	//applay Casteljau algorithm -  subdivide each curve one time
	vector<point> new_points;
	point original_points[4];
	bool lastCurveIsQuadratic = false;
	//each qubic curve turns to left and righr curve according to slide 27
	for (int i = 0; i < numOfBezierCurves; i++) {
		original_points[0] = points.at(i * 3);
		original_points[1] = points.at(i * 3 + 1);
		original_points[2] = points.at(i * 3 + 2);
		if (((i * 3) + 3) < points.size()) {	// 4 points curve, subdivinding into 2 joining 4 points curves
			original_points[3] = points.at(i * 3 + 3);
			addLeftRightCurves(&new_points, original_points);
			if (i + 1 != numOfBezierCurves)
				new_points.pop_back();
		}
		else {									// last curve is 3 points curve, not doing nothing
			lastCurveIsQuadratic = true;
	//		new_points.push_back(original_points[0]);
	//		new_points.push_back(original_points[1]);
	//		new_points.push_back(original_points[2]);
		}
		
		
	}
	//change the points vector by adding new points to it
	//change number of beizer curves
	points.clear();
	points = new_points;
	numOfBezierCurves = numOfBezierCurves * 2;
	if (lastCurveIsQuadratic)
		numOfBezierCurves -= 2;
}

void addLeftRightCurves(vector<point> * new_points, point * original_points) {
	//this function gets 4 original points , creates the left and the right curves
	// and adds them to the new_points vector
	point p0, p1, p2, p3; //original curve
	point l0, l1, l2, l3; // new leftcurve
	point r0, r1, r2, r3; // new rightcurve

	p0 = original_points[0];
	p1 = original_points[1];
	p2 = original_points[2];
	p3 = original_points[3];


	l0.x = p0.x;
	l0.y = p0.y;

	r3.x = p3.x;
	r3.y = p3.y;

	l1.x = (p0.x + p1.x) / 2.0f;
	l1.y = (p0.y + p1.y) / 2.0f;

	r2.x = (p2.x + p3.x) / 2.0f;
	r2.y = (p2.y + p3.y) / 2.0f;

	l2.x = (l1.x + (p1.x + p2.x) / 2.0f) / 2.0f;
	l2.y = (l1.y + (p1.y + p2.y) / 2.0f) / 2.0f;

	r1.x = (r2.x + (p1.x + p2.x) / 2.0f) / 2.0f;
	r1.y = (r2.y + (p1.y + p2.y) / 2.0f) / 2.0f;

	r0.x = (l2.x + r1.x) / 2.0f;
	r0.y = (l2.y + r1.y) / 2.0f;

	l3.x = r0.x;
	l3.y = r0.y;

	(*new_points).push_back(l0);
	(*new_points).push_back(l1);
	(*new_points).push_back(l2);
	(*new_points).push_back(l3);
	// middle point r0==l3 ,  no need to add twice

	//(*new_points).push_back(r0);			
	(*new_points).push_back(r1);
	(*new_points).push_back(r2);
	(*new_points).push_back(r3);
}

void PrintAllPoints() {
	printf("\n");
	printf("number of curves: %d \n", numOfBezierCurves);
	for (int i = 0; i < points.size(); i++) {
		printf("point %d: %f , %f\n", i, points.at(i).x, points.at(i).y);
	}
}