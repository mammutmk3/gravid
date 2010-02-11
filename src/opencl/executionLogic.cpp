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
#include "opencl/fadePipeline.h"

#include <CL/cl.h>
#include <iostream>

#include <sys/time.h>

void GRAVID::exec_img_overlay(VideoPipeline *pVidPipe, Kernel *pKernel, VideoReader *pReader, cl_command_queue cmdQ){
	/**
	 * initialization happens only once
	 */
	static cl_event lastKernel = NULL;
	static cl_event copyToDevice = NULL;
	static bool init_done = false;

	if(!init_done){
		// init the kernel
		pKernel->setKernelArgument(0,pVidPipe->getInputImage_Device());
		pKernel->setKernelArgument(1,pVidPipe->getOutputImage_Device());
		init_done = true;
	}
	
	pKernel->setKernelArgument(2,(int)pVidPipe->getFramePos());
	pReader->changeFrameBuffer(pVidPipe->getFrame_ForDecoder());
	pReader->decodeNextFrame();

	timeval start;
	gettimeofday( &start, NULL);
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
	// for time measurement
	timeval end;
	gettimeofday( &end, NULL);
	static float sum = 0;
	static int counter = 0;
	sum += ((end.tv_sec-start.tv_sec)*1e6+(end.tv_usec-start.tv_usec))/1000;
	counter++;
	if(counter >=96)
	  std::cout << "avg time for video_effect: " << sum/counter  << " ms"<< std::endl;
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
	// for time measurement
	static float sum = 0;
	static int counter = 0;
	if(pReader->hasNextFrame()){
		// decode a frame
		pReader->changeFrameBuffer(pMemMan->getFrame_forDecoder());
		pReader->decodeNextFrame();
		timeval start;
		gettimeofday( &start, NULL);
		// make sure no kernel is running
		pMemMan->copyToDevice(pKExec->getLastKernelEvent());
		// execute the kernels
		pKExec->executeAll(*pMemMan);
		// copy the results back
		pMemMan->copyFromDevice(pKExec->getLastKernelEvent());
		// for time measurement
		timeval end;
		gettimeofday( &end, NULL);

		sum += ((end.tv_sec-start.tv_sec)*1e6+(end.tv_usec-start.tv_usec))/1000;
		counter++;
	}
	else{
		std::cout << "avg time for img_effect: " << sum/counter  << " ms"<< std::endl;;
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

void GRAVID::exec_fade_effects(VideoReader *pReader1, VideoReader *pReader2, FadePipeline *pFPipe, Kernel *pKernel, 
				cl_command_queue cmdQ, const unsigned int nb_frames){
  static bool done = false;
  static unsigned int currentFrame = 1;

  if(!done){
    pReader1->changeFrameBuffer(pFPipe->getFirstImage_forDecoder());
    pReader2->changeFrameBuffer(pFPipe->getSecondImage_forDecoder());
    done = true;
  }

  // decode both the input images
  pReader1->decodeNextFrame();
  pReader2->decodeNextFrame();

  timeval start;
  gettimeofday( &start, NULL);
  cl_event copyToDevice = pFPipe->copyToDevice(NULL);

  float percentage = currentFrame / (float)nb_frames;
  pKernel->setKernelArgument(2,percentage);

  // start the kernel as soon as the copy process is finished
  cl_event last_kernel = NULL;
  cl_int errorCode = clEnqueueNDRangeKernel(cmdQ, pKernel->getNativeKernel(),
					    2, NULL, pKernel->getGlobalDim(), pKernel->getLocalDim(),
					    1, &copyToDevice, &last_kernel);
  if(CL_SUCCESS != errorCode){
    std::cerr << "couldn't enqueue fade effect kernel" << std::endl;
    exit(1);
  }

  cl_event copyFromDevice = pFPipe->copyFromDevice(last_kernel);
  clWaitForEvents(1,&copyFromDevice);
  // for time measurement
  timeval end;
  gettimeofday( &end, NULL);
  static float sum = 0;
  sum += ((end.tv_sec-start.tv_sec)*1e6+(end.tv_usec-start.tv_usec))/1000;
  if(currentFrame == nb_frames)
	  std::cout << "avg time for fade_effect: " << sum/nb_frames  << " ms"<< std::endl;
  currentFrame++;
}
