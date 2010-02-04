/*
 * glOutput.h
 *
 *  Created on: Jan 22, 2010
 *      Author: lars
 */

#ifndef GLOUTPUT_H_
#define GLOUTPUT_H_

#include "types.h"

#include "opencl/kernelExecutor.h"
#include "opencl/memoryManager.h"
#include "codec/videoReader.h"
#include "opencl/videoPipeline.h"
#include "opencl/kernel.h"

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
}

#endif /* GLOUTPUT_H_ */
