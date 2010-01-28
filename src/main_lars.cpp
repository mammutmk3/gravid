/*
 * main.cpp
 *
 *  Created on: Jan 6, 2010
 *      Author: lars
 */


extern "C"{
	#include <libavformat/avformat.h>
}

#include "codec/videoReader.h"
#include "types.h"
#include "visual/glDisplayer.h"
#include "opencl/kernelExecutor.h"

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <stdio.h>

#include "opencl/openCLProgram.h"
#include "opencl/memoryManager.h"
#include "opencl/kernel.h"

#include <CL/cl.h>

using namespace GRAVID;

int main(int argc, char** argv){
	// register all available codecs in mpeg
	av_register_all();

	try{
		// create the decoder for a video file
		VideoReader reader("videos/video1.mpg");
		VideoInfo vidInf = reader.getVideoInfo();

		// create the new OpenCL program to process that video
		OpenCLProgram clrFltr("src/opencl/kernels/colorFilters.cl");
		// create the streaminig pipeline
		MemoryManager memMan(clrFltr.getContext(),clrFltr.getCommandQueue(),vidInf);
		// associate the pinned memory with the buffer of ffmpeg
		reader.changeFrameBuffer(memMan.getInputFrame());

		// create a kernel for an effect
		Kernel kernel(clrFltr.getProgram(),"sepiaFilter");
		// set the kernel parameters
		kernel.setKernelArgument(0, memMan.getInImage_dev());
		kernel.setKernelArgument(1, memMan.getOutImage_dev());

		// create a Kernel-executor to launch as many kernels at one frame as wanted
		KernelExecutor kExec(clrFltr.getCommandQueue(),vidInf.width, vidInf.height);
		kExec.addKernel(kernel);

		// tell OpenGL what the OpenCL parts are
		setToolChain(kExec, reader, memMan);
		// display the results
		glDisplay(argc,argv,vidInf.width,vidInf.height,memMan.getOutputFrame());
	}
	catch(std::logic_error &e){
		// print an error message
		std::cerr << e.what() << std::endl;
		// close the program
		exit(-1);
	}
}
