/*
 * main.cpp
 *
 *  Created on: Jan 6, 2010
 *      Author: lars
 */

#include "cmdLineParser.h"
#include "threads.h"
#include "visual/glDisplayer.h"

#include "codec/videoReader.h"
#include "codec/videoWriter.h"

#include "opencl/openCLProgram.h"
#include "opencl/memoryManager.h"
#include "opencl/kernelExecutor.h"
#include "opencl/videoPipeline.h"
#include "opencl/fadePipeline.h"
#include "opencl/executionLogic.h"

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include <pthread.h>

using namespace GRAVID;

#define FIFO_LENGTH 5

int main(int argc, char** argv){

	OpenCLProgram *oclProgram = NULL;
	MemoryManager *memMan = NULL;
	VideoPipeline *vidPipe = NULL;
	KernelExecutor *kExec = NULL;
	VideoWriter *pVidWriter = NULL;
	Kernel *pKernel = NULL;
	FadePipeline *pFPipe = NULL;
	try{
		// parse the command line arguments
		CmdLineParser cmdPars(argc, argv);

		// create the decoder for a video file
		VideoReader reader(cmdPars.getInputVideo());
		VideoInfo vidInf = reader.getVideoInfo();

		if(cmdPars.hasVideoEffect()){
			// create the new OpenCL program to process that video
			oclProgram = new OpenCLProgram("src/opencl/kernels/video_effects.cl");
			// create the pipeline to execute a video effect
			vidPipe = new VideoPipeline(oclProgram->getContext(), oclProgram->getCommandQueue(), FIFO_LENGTH,vidInf.width, vidInf.height);
			std::string kernelName;
			switch(cmdPars.getVideoEffect()){
				// using switch fall through to reduce code
				case GHOST: kernelName = "ghosteffect";								
				case ECHO_BLUR:{
						if(kernelName.empty())
						  kernelName = "echoblureffect";
						pKernel = new Kernel(oclProgram->getProgram(), kernelName.c_str(), vidInf.width, vidInf.height);
						if(cmdPars.hasOutputFile()){
							// create an encoder
							pVidWriter = new VideoWriter(cmdPars.getOutputFile(), vidInf);
							while(reader.hasNextFrame()){
								GRAVID::exec_img_overlay(vidPipe,pKernel,&reader,oclProgram->getCommandQueue());
								pVidWriter->writeMultiMedFrame(vidPipe->getResultFrame());
							}
							pVidWriter->finalizeVideo();
						}
						else{
							initOpenGL_video(vidPipe, pKernel, &reader, oclProgram->getCommandQueue(),ECHO_BLUR);
							glDisplay(argc, argv, vidInf.width, vidInf.height, VIDEO);
						}
						}break;									
			}
		}
		else if(cmdPars.hasFadeEffect()){
			// create a second video reader
			VideoReader reader2(cmdPars.getInputVideo2());

			oclProgram = new OpenCLProgram("src/opencl/kernels/fade_effects.cl");
			// create the memory pipeline
			pFPipe = new FadePipeline(oclProgram->getContext(), oclProgram->getCommandQueue(), vidInf.width, vidInf.height);
			std::string kernelName;
			switch(cmdPars.getFadeEffect()){
				case CIRCLE: kernelName = "circle_fade";break;
				case ADDITIVE: kernelName = "fadeAdditve";break;
				case BLINDS: kernelName = "fadeBlind";break;
				case TEETH: kernelName = "teeth_fade";break;
			}
			pKernel = new Kernel(oclProgram->getProgram(), kernelName.c_str(), vidInf.width, vidInf.height);
			// set the fixed kernel parameters
			pKernel->setKernelArgument(0,pFPipe->get3DInputImage());
			pKernel->setKernelArgument(1,pFPipe->get2DOutputImage());
			// extract the frame number of the shorter of the two videos
			unsigned int nb_frames = (vidInf.nb_frames < reader2.getVideoInfo().nb_frames)? vidInf.nb_frames: reader2.getVideoInfo().nb_frames;
			if(cmdPars.hasOutputFile()){
				// create an encoder
				pVidWriter = new VideoWriter(cmdPars.getOutputFile(), vidInf);
				for(unsigned int i=0;i<nb_frames;i++){
				exec_fade_effects(&reader, &reader2, pFPipe, pKernel,oclProgram->getCommandQueue(), nb_frames);
				pVidWriter->writeMultiMedFrame(pFPipe->getImage_forEncoder());
				}
				pVidWriter->finalizeVideo();
			}
		  else{
		    // display setup
		    initOpenGL_fade(pFPipe, pKernel, &reader, &reader2, oclProgram->getCommandQueue(), nb_frames);
		    glDisplay(argc, argv, vidInf.width, vidInf.height, FADE);
		  }
		}
		else{
			// create the new OpenCL program to process that video
			oclProgram = new OpenCLProgram("src/opencl/kernels/image_effects.cl");
			// create the streaminig pipeline
			memMan = new MemoryManager(oclProgram->getContext(),oclProgram->getCommandQueue(),vidInf);
			// create a Kernel-executor to launch as many kernels at one frame as wanted
			kExec = new KernelExecutor(oclProgram->getCommandQueue(),
										oclProgram->getProgram(),
										cmdPars.getImgEffects(),
										cmdPars.getNBImgEffects(),
										vidInf);
			if(cmdPars.hasOutputFile()){
				// create an encoder
				pVidWriter = new VideoWriter(cmdPars.getOutputFile(), vidInf);

				bool firstLaunch = true, secondLaunch = false, encoderStarted = false;
				pthread_t decodeThread, displayThread;
				// initialize the thread context
				GRAVID::initThreadVariables(&reader, pVidWriter,memMan,kExec, vidInf.width, vidInf.height);
				while(reader.hasNextFrame()){
					if(!firstLaunch){
						// make sure no kernel is running and that the decoder has finished it's last frame
						pthread_join(decodeThread,NULL);
						memMan->copyToDevice(kExec->getLastKernelEvent());
					}
					// decode a frame
					pthread_create(&decodeThread,NULL,GRAVID::decode_Frame, (void*)memMan->getFrame_forDecoder());
					// only launch e kernel if he has an image to process
					if(!firstLaunch){
						kExec->executeAll(*memMan);
						// wait for the encoder
						if(encoderStarted)
							pthread_join(displayThread,NULL);
						memMan->copyFromDevice(kExec->getLastKernelEvent());
						if(!secondLaunch){
							// encode a frame
							pthread_create(&displayThread,NULL,GRAVID::encode_Frame, (void*)memMan->getFrame_forEncoder());
							encoderStarted = true;
						}
						else
							secondLaunch = false;
					}
					else{
						firstLaunch = false;
						secondLaunch = true;
					}
				}
				pVidWriter->finalizeVideo();
			}
			else{
				// initialize the renderer
				GRAVID::initOpenGL_image(kExec,&reader,memMan);
				GRAVID::glDisplay(argc, argv, vidInf.width, vidInf.height, IMAGE);
			}
		}

		// clean up
		delete kExec; kExec = NULL;
		delete memMan; memMan = NULL;
		delete oclProgram; oclProgram = NULL;
		delete vidPipe; vidPipe = NULL;
		delete pVidWriter; pVidWriter = NULL;
		delete pKernel; pKernel = NULL;
		delete pFPipe; pFPipe = NULL;
	}
	catch(std::logic_error &e){
		// print an error message
		std::cerr << e.what() << std::endl;

		// clean up
		if(NULL != kExec)
			delete kExec;

		if(NULL != memMan)
			delete memMan;

		if(NULL != oclProgram)
			delete oclProgram;

		if(NULL != vidPipe)
			delete vidPipe;

		if(NULL != pVidWriter)
			delete pVidWriter;

		if(NULL != pKernel)
			delete pKernel;

		if(NULL != pFPipe)
			delete pFPipe;

		// close the program
		exit(-1);
	}
}
