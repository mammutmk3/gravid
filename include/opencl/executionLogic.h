/*
 * executionLogic.h
 *
 *  Created on: Feb 3, 2010
 *      Author: lars
 */

#ifndef EXECUTIONLOGIC_H_
#define EXECUTIONLOGIC_H_

#include "codec/videoReader.h"

#include "opencl/videoPipeline.h"
#include "opencl/kernel.h"
#include "opencl/fadePipeline.h"

#include "opencl/kernelExecutor.h"
#include "opencl/memoryManager.h"

namespace GRAVID{

	void exec_img_overlay(VideoPipeline *pVidPipe, Kernel *pKernel, VideoReader *pReader, cl_command_queue cmdQ);

	void exec_img_effects(VideoReader *pReader_local, MemoryManager *pMemMan_local, KernelExecutor *pKExec);

	void exec_fade_effects(VideoReader *pReader1, VideoReader *pReader2, FadePipeline *pFPipe, Kernel *pKernel, 
				cl_command_queue cmdQ, const unsigned int nb_frames);
}

#endif /* EXECUTIONLOGIC_H_ */
