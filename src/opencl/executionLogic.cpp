/*
 * executionLogic.cpp
 *
 *  Created on: Feb 3, 2010
 *      Author: lars
 */

#include "threads.h"
#include <pthread.h>

#include "opencl/executionLogic.h"
#include "opencl/videoPipeline.h"
#include "opencl/kernel.h"

#include <CL/cl.h>

void GRAVID::exec_img_overlay(VideoPipeline *pVidPipe, Kernel *pKernel, VideoReader *pReader, cl_command_queue cmdQ){
	/**
	 * initialization happens only once
	 */
	static cl_event lastKernel = NULL;
	static cl_event copyToDevice = NULL;
	static cl_event copyFromDevice = NULL;
	static bool init_done = false;

	if(!init_done){
		// init the kernel
		pKernel->setKernelArgument(0,pVidPipe->getInputImage_Device());
		pKernel->setKernelArgument(1,pVidPipe->getOutputImage_Device());
		pKernel->setKernelArgument(2,pVidPipe->getFramePos());
		init_done = true;
	}

	pReader->changeFrameBuffer(pVidPipe->getFrame_ForDecoder());
	pReader->decodeNextFrame();

	copyToDevice = pVidPipe->copyToDevice(lastKernel);

	// start the kernel as soon as the copy process is finished
	cl_int errorCode = clEnqueueNDRangeKernel(cmdQ, pKernel->getNativeKernel(),
												2, NULL, pKernel->getGlobalDim(), pKernel->getLocalDim(),
												1, &copyToDevice, &lastKernel);
	if(CL_SUCCESS != errorCode){
		std::cerr << "couldn't enqueue the image overlay kernel" << std::endl;
		exit(1);
	}

	pVidPipe->copyFromDevice(lastKernel);
}


void GRAVID::exec_img_effects(VideoReader *pReader, MemoryManager *pMemMan, KernelExecutor *pKExec){
	static bool isPipelineFilled = false;

	// TODO knowing that there will have to be enough frames in the movie
	// TODO but we don't handle those delicate special cases right now
	// fill the pipeline first
	if(!isPipelineFilled){
		for(int i=0;i<2;i++){
			// decode first frame
			pReader->changeFrameBuffer(pMemMan->getFrame_forDecoder());
			pReader->decodeNextFrame();
			// copy it to the device
			pMemMan->copyToDevice(NULL);
		}
		//execute a first kernel
		pKExec->executeAll(*pMemMan);
		//copy the first frame back
		pMemMan->copyFromDevice(pKExec->getLastKernelEvent());
		isPipelineFilled = true;
	}
	/**
	 * pipeline is filled: work as usual
	 */
	if(pReader->hasNextFrame()){
		// decode a frame
		pReader->changeFrameBuffer(pMemMan->getFrame_forDecoder());
		pReader->decodeNextFrame();
		// make sure no kernel is running
		pMemMan->copyToDevice(pKExec->getLastKernelEvent());
		// execute the kernels
		pKExec->executeAll(*pMemMan);
		// copy the results back
		pMemMan->copyFromDevice(pKExec->getLastKernelEvent());
	}
	else{
		static int pipeLineEmptyPhase = 1;
		/**
		 * has been called to empty the pipeline
		 */
		// process the last 2 unprocessed images
		if(pipeLineEmptyPhase <= 2){
			// copy it to the device
			pMemMan->copyToDevice(pKExec->getLastKernelEvent());
			// execute the kernels
			pKExec->executeAll(*pMemMan);
			// copy the results back
			pMemMan->copyFromDevice(pKExec->getLastKernelEvent());
			pipeLineEmptyPhase++;
		}
		else if(pipeLineEmptyPhase <=4){
			// copy the last 2 result images back
			pMemMan->copyFromDevice(pKExec->getLastKernelEvent());
			pipeLineEmptyPhase++;
		}
	}
}
