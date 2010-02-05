/*
 * glOutput.cpp
 *
 *  Created on: Jan 22, 2010
 *      Author: lars
 */

#include "visual/glDisplayer.h"
#include "types.h"

#include "opencl/kernelExecutor.h"
#include "opencl/memoryManager.h"
#include "opencl/executionLogic.h"
#include "opencl/videoPipeline.h"
#include "codec/videoReader.h"
#include "opencl/kernel.h"

#include <GL/glut.h>
#include <CL/cl.h>
#include <iostream>

using namespace GRAVID;

// i really hate doing that, but glut doesn't allow OOP
// so here come the global variables and functions...
unsigned short gl_window_width;
unsigned short gl_window_height;

// the tool chain
KernelExecutor *pKExec;
VideoReader *pReader;
MemoryManager *pMemMan;

// for video effects
VideoPipeline *pVidPipe;
Kernel *pKernel;
cl_command_queue cmdQ;
Video_Effect vidEffect;

// render image effects
void draw_image(){
	static int pipe_empty_phase = 1;

	if(pReader->hasNextFrame()){
		glutPostRedisplay();
	}
	else if(pipe_empty_phase <=4){
		// nessecary to empty the pipeline
		pipe_empty_phase++;
		glutPostRedisplay();
	}
	GRAVID::exec_img_effects(pReader, pMemMan, pKExec);
	glDrawPixels(gl_window_width, gl_window_height, GL_RGBA, GL_UNSIGNED_BYTE, pMemMan->getFrame_forEncoder());
	glFlush ();
}

// render video effects
void draw_video(){
	if(pReader->hasNextFrame()){
		if(GHOST == vidEffect || ECHO_BLUR == vidEffect){
			GRAVID::exec_img_overlay(pVidPipe, pKernel, pReader,cmdQ);
			glDrawPixels(gl_window_width, gl_window_height, GL_RGBA, GL_UNSIGNED_BYTE,pVidPipe->getResultFrame());
			glutPostRedisplay();
		}
		else{
		}
	}
	glFlush();
}

void GRAVID::glDisplay(int argc, char** argv,
						const size_t windowWidth, const size_t windowHeight, RenderMode rMode){
	// save the width and height
	gl_window_width = windowWidth;
	gl_window_height = windowHeight;

	// initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	// center the output
	glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - windowWidth/2,
								glutGet(GLUT_SCREEN_HEIGHT)/2 - windowHeight/2);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("GRAVID video editor");

	// register Callback functions
	if(IMAGE == rMode){
		glutDisplayFunc(draw_image);
	}
	else{
		glutDisplayFunc(draw_video);
	}

	// default initialization
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glPixelZoom( 1.0, -1.0 );
	glRasterPos2d(-1,1);

	// start the gl-Loop
	glutMainLoop();
}

void GRAVID::initOpenGL_image(KernelExecutor *pKExec_local, VideoReader *pReader_local,
								MemoryManager *pMemMan_local){
	pKExec = pKExec_local;
	pReader = pReader_local;
	pMemMan = pMemMan_local;
}

void GRAVID::initOpenGL_video(VideoPipeline *pVidPipe_local, Kernel *pKernel_local, VideoReader *pReader_local,
						cl_command_queue cmdQ_local, Video_Effect vidEffect_local){
	pVidPipe = pVidPipe_local;
	pReader = pReader_local;
	pKernel = pKernel_local;
	cmdQ = cmdQ_local;

	vidEffect = vidEffect_local;
}
