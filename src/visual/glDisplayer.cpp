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
#include "opencl/fadePipeline.h"
#include "codec/videoReader.h"
#include "opencl/kernel.h"

#include <GL/glut.h>
#include <CL/cl.h>
#include <iostream>

#include <sys/time.h>

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

// for fade effects
FadePipeline *pFPipe;
VideoReader *pReader2;
unsigned int nb_frames;

// to check, how much time elapsed since rendering the last frame
timeval last_time;

// to check, if the necessary waiting-time for fps is reached, so we can draw the next frame
bool fps_ok() {
	timeval cur_time;
	gettimeofday( &cur_time, NULL);
	
	VideoInfo vidInf = pReader->getVideoInfo();
	
	long int time_per_frame = ( vidInf.duration / vidInf.nb_frames ) * 1000000;
	long int cur = cur_time.tv_usec + cur_time.tv_sec * 1000000;
	long int last = last_time.tv_usec + last_time.tv_sec * 1000000;
	
	if((cur-last) < time_per_frame)
		return false;
	
	gettimeofday( &last_time, NULL);
	std::cout << (cur-last) << std::endl;
	return true;
}


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
	// play video with correct timecode
	while ( !fps_ok() ) { }
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
	// play video with correct timecode
	while ( !fps_ok() ) { }
	glFlush();
}

void draw_fade(){
  static unsigned int current_frame = 1;
  if(current_frame < nb_frames){
    exec_fade_effects(pReader, pReader2, pFPipe, pKernel,cmdQ, nb_frames);
    current_frame++;
    glDrawPixels(gl_window_width, gl_window_height, GL_RGBA, GL_UNSIGNED_BYTE,pFPipe->getImage_forEncoder());
    glutPostRedisplay();
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
	switch(rMode){
	case IMAGE: glutDisplayFunc(draw_image); break;
	case VIDEO: glutDisplayFunc(draw_video); break;
	case FADE : glutDisplayFunc(draw_fade);break;
	default : std::cerr << "no valid draw function available" << std::endl;
	}

	// default initialization
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glPixelZoom( 1.0, -1.0 );
	glRasterPos2d(-1,1);

	// start the gl-Loop
	gettimeofday( &last_time, NULL);
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

void GRAVID::initOpenGL_fade(FadePipeline *pFPipe_l, Kernel *pKernel_l, VideoReader *pReader1_l, VideoReader *pReader2_l, 
		      cl_command_queue cmdQ_l, const unsigned int nb_frames_l){
  pFPipe = pFPipe_l;
  pKernel = pKernel_l;
  pReader = pReader1_l;
  pReader2 = pReader2_l;
  cmdQ = cmdQ_l;
  nb_frames = nb_frames_l;
}
