/*
 * glOutput.cpp
 *
 *  Created on: Jan 22, 2010
 *      Author: lars
 */

#include "visual/glDisplayer.h"

#include "opencl/kernelExecutor.h"
#include "opencl/memoryManager.h"
#include "codec/videoReader.h"
#include "types.h"

#include <GL/glut.h>
#include <CL/cl.h>

#include "cpu_effects/colorFilter.h"

using namespace GRAVID;

// i really hate doing that, but glut doesn't allow OOP
// so here come the global variables and functions...
unsigned short gl_window_width;
unsigned short gl_window_height;
RGBA* gl_display_image;

// the tool chain
KernelExecutor *pKExec;
VideoReader *pReader;
MemoryManager *pMemMan;

// event to determine: if the copy processes to/from the device are finished, the kernels are finished
cl_event cToDev, cFromDev, kernelsFinished;

void draw(){
	// decode the frame
	pReader->decodeNextFrame();
	// copy it to the device
	cToDev = pMemMan->updateInputFrame();

	// start the kernels
	pKExec->executeAll(cToDev);
	kernelsFinished = pKExec->getLastKernelEvent();

	// copy the results back
	cFromDev = pMemMan->updateOutputFrame(kernelsFinished);
	clWaitForEvents(1,&cFromDev);

	// draw the frames
	glDrawPixels(gl_window_width, gl_window_height, GL_RGBA, GL_UNSIGNED_BYTE, gl_display_image);
	glFlush();
	if(pReader->hasNextFrame())
		glutPostRedisplay();
}

void GRAVID::glDisplay(int argc, char** argv,
				const unsigned short windowWidth, const unsigned short windowHeight,
				RGBA* displayImage){

	// save the width and height
	gl_window_width = windowWidth;
	gl_window_height = windowHeight;
	gl_display_image = displayImage;

	// initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	// center the output
	glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - windowWidth/2,
								glutGet(GLUT_SCREEN_HEIGHT)/2 - windowHeight/2);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("GRAVID video editor");

	// register Callback functions
	glutDisplayFunc(draw);

	// default initialization
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glPixelZoom( 1.0, -1.0 );
	glRasterPos2d(-1,1);

	// start the gl-Loop
	glutMainLoop();
}

void GRAVID::setToolChain(KernelExecutor &kExec, VideoReader &reader, MemoryManager &memMan){
	pKExec = &kExec;
	pReader = &reader;
	pMemMan = &memMan;
}
