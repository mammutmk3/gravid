/*
 * main.cpp
 *
 *  Created on: Jan 6, 2010
 *      Author: lars
 */

#include "cmdLineParser.h"
#include "threads.h"

#include "codec/videoReader.h"
#include "codec/videoWriter.h"

#include "opencl/openCLProgram.h"
#include "opencl/memoryManager.h"
#include "opencl/kernelExecutor.h"

#include <iostream>
#include <cstdlib>
#include <stdexcept>

#include <pthread.h>

using namespace GRAVID;

int main(int argc, char** argv){

	OpenCLProgram *oclProgram = NULL;
	MemoryManager *memMan = NULL;
	KernelExecutor *kExec = NULL;
	try{
		// parse the command line arguments
		CmdLineParser cmdPars(argc, argv);

		// create the decoder for a video file
		VideoReader reader(cmdPars.getInputVideo());
		VideoInfo vidInf = reader.getVideoInfo();

		// create an encoder
		VideoWriter writer(cmdPars.getOutputFile(), vidInf);

		if(cmdPars.hasVideoEffect()){
			// TODO video effects do yet have to be supportedstd::cout << "noch tiefer der schleife" << std::endl;
			throw std::logic_error("video effects are not supported yet");
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

			bool firstLaunch = true, secondLaunch = false, encoderStarted = false;
			pthread_t decodeThread, displayThread;
			// initialize the thread context
			GRAVID::initThreadVariables(&reader, &writer ,memMan,kExec, vidInf.width, vidInf.height);
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
		}

		writer.finalizeVideo();

		// clean up
		delete kExec; kExec = NULL;
		delete memMan; memMan = NULL;
		delete oclProgram; oclProgram = NULL;
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

		// close the program
		exit(-1);
	}
}
