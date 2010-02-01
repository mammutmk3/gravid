/*
 * threads.cpp
 *
 *  Created on: Feb 1, 2010
 *      Author: lars
 */

#include "threads.h"
#include "types.h"

#include <pthread.h>
#include <CL/cl.h>
#include <stdio.h>
#include <GL/glut.h>

#include "codec/videoReader.h"
#include "codec/videoWriter.h"
#include "opencl/kernelExecutor.h"
#include "opencl/memoryManager.h"

#include "visual/writePPM.h"

using namespace GRAVID;

VideoReader *pReader_thread;
MemoryManager *pMemMan_thread;
KernelExecutor *pKExec_thread;
VideoWriter *pWriter_thread;

unsigned short width_thread, height_thread;

int i_thread = 1;

void GRAVID::initThreadVariables(VideoReader *pReader_local, VideoWriter *pWriter_local,
									MemoryManager *pMemMan_local, KernelExecutor *pKExec_local,
									unsigned short width, unsigned short height){

	pReader_thread = pReader_local;
	pWriter_thread = pWriter_local;
	pMemMan_thread = pMemMan_local;
	pKExec_thread = pKExec_local;

	width_thread = width;
	height_thread = height;
}

void* GRAVID::decode_Frame(void* RGBA_Frame){
	pReader_thread->changeFrameBuffer((RGBA*) RGBA_Frame);
	pReader_thread->decodeNextFrame();
}

void* GRAVID::encode_Frame(void* RGBA_Frame){
	pWriter_thread->writeMultiMedFrame((RGBA*) RGBA_Frame);
	/*char filename[50];
	sprintf(filename,"pictures/pic%i.ppm",i_thread);
	i_thread++;
	GRAVID::writePPM((RGBA*)RGBA_Frame,filename,720,576);*/
}
