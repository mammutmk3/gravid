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

#include <sys/time.h>

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
VideoInfo pVidInf;
MemoryManager *pMemMan;

// event to determine: if the copy processes to/from the device are finished, the kernels are finished
cl_event cToDev, cFromDev, kernelsFinished;

// to check, how much time elapsed since rendering the last frame
timeval last_time;

// to check, if the necessary waiting-time for fps is reached, so we can draw the next frame
bool check_fps() {
	timeval cur_time;
	gettimeofday( &cur_time, NULL);
	
	int framerate = pVidInf.frame_rate.num / pVidInf.frame_rate.den;
	if ( (cur_time.tv_usec - last_time.tv_usec) >= framerate )
		return true;
	
	return false;
}

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
	
	// todo: hier den timecode einbauen
	while ( !check_fps() ) { }

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
	
	// initialize the fps-counter
	gettimeofday( &last_time, NULL);

	// start the gl-Loop
	glutMainLoop();
}

void GRAVID::setToolChain(KernelExecutor &kExec, VideoReader &reader, MemoryManager &memMan){
	pKExec = &kExec;
	pReader = &reader;
	pVidInf = pReader->getVideoInfo();
	pMemMan = &memMan;
}
