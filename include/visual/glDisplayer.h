/*
 * glDisplayer.h
 *
 *  Created on: Jan 22, 2010
 *      Author: lars
 */

#ifndef GLDISPLAYER_H_
#define GLDISPLAYER_H_

#include "types.h"

#include "opencl/kernelExecutor.h"
#include "opencl/memoryManager.h"
#include "codec/videoReader.h"
#include "opencl/videoPipeline.h"
#include "opencl/kernel.h"
#include "opencl/fadePipeline.h"

#include <CL/cl.h>

namespace GRAVID{

	/**
	 * starts the glut mainloop that draws the result
	 */
	void glDisplay(int argc, char** argv, const size_t windowWidth, const size_t windowHeight, RenderMode rMode_local);

	/**
	 * initializes the OpenGL renderer
	 */
	void initOpenGL_image(KernelExecutor *pKExec, VideoReader *pReader, MemoryManager *pMemMan);

	/**
	 * initialize OpenGL for video display
	 */
	void initOpenGL_video(VideoPipeline *pVidPipe, Kernel *pKernel, VideoReader *pReader, cl_command_queue cmdQ, Video_Effect vidEffect_local);

	/**
	 * initialize OpenGL for fade effects
	 */
	void initOpenGL_fade(FadePipeline *pFPipe_l, Kernel *pKernel_l, VideoReader *pReader1_l, VideoReader *pReader2_l, 
			      cl_command_queue cmdQ_l, const unsigned int nb_frames);
}

#endif /* GLDISPLAYER_H_ */
