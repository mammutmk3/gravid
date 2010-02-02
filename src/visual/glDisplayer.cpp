/*
 * glOutput.cpp
 *
 *  Created on: Jan 22, 2010
 *      Author: lars
 */

#include "threads.h"
#include "visual/glDisplayer.h"

#include "opencl/kernelExecutor.h"
#include "opencl/memoryManager.h"
#include "codec/videoReader.h"
#include "types.h"

#include <GL/glut.h>
#include <CL/cl.h>
#include <iostream>
#include <pthread.h>

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

bool firstLaunch = true, secondLaunch = false, encoderStarted = false;
pthread_t decodeThread, displayThread;

void draw(){
	//decode first frame
	if(pReader->hasNextFrame()){
		// make sure no kernel is running and that the decoder has finished it's last frame
		pthread_join(decodeThread,NULL);
		pMemMan->copyToDevice(pKExec->getLastKernelEvent());
		// decode a frame
		pthread_create(&decodeThread,NULL,GRAVID::decode_Frame, (void*)pMemMan->getFrame_forDecoder());
		// the pipelines has to be filled first
		if(!firstLaunch){
			pKExec->executeAll(*pMemMan);
			// wait for the encoder
			if(encoderStarted)
				pthread_join(displayThread,NULL);
			pMemMan->copyFromDevice(pKExec->getLastKernelEvent());
			if(!secondLaunch){
				// encode a frame
				pthread_create(&displayThread,NULL,GRAVID::encode_Frame, (void*)pMemMan->getFrame_forEncoder());
				//glDrawPixels(720, 576, GL_RGBA, GL_UNSIGNED_BYTE, pMemMan->getFrame_forEncoder());
				encoderStarted = true;
			}
			else
				secondLaunch = false;
		}
		else{
			firstLaunch = false;
			secondLaunch = true;
		}
		glutPostRedisplay();
	}
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

	//decode first frame
	if(pReader->hasNextFrame())
		pthread_create(&decodeThread,NULL,GRAVID::decode_Frame, (void*)pMemMan->getFrame_forDecoder());
	// start the gl-Loop
	glutMainLoop();
}

void GRAVID::setToolChain(KernelExecutor &kExec, VideoReader &reader, MemoryManager &memMan){
	pKExec = &kExec;
	pReader = &reader;
	pMemMan = &memMan;
}
