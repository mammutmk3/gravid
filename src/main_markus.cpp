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
#include "visual/writePPM.h"
#include "cpu_effects/edgeDetection.h"

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

	/*
	 * start useful stuff here
	 */

	try{
		// create the decoder for a video file
		VideoReader reader("videos/testvid.mpg");
		VideoInfo vidInf = reader.getVideoInfo();

		// create the new OpenCL program to process that video
		OpenCLProgram clrFltr("src/opencl/kernels/sobel.cl");
		// create the streaminig pipeline
		MemoryManager memMan(clrFltr.getContext(),clrFltr.getCommandQueue(),vidInf);
		// associate the pinned memory with the buffer of ffmpeg
		reader.changeFrameBuffer(memMan.getInputFrame());

		// create a kernel for an effect
		Kernel kernel(clrFltr.getProgram(),"sobel");
		// set the kernel parameters
		kernel.setKernelArgument(0, memMan.getInImage_dev());
		kernel.setKernelArgument(1, memMan.getOutImage_dev());

  //              EdgeDetection egde;

		char filename[100];
		for(int i=0;i<50;i++){
			// decode the frame
			reader.decodeNextFrame();
	
//                        RGBA* output_pic = (RGBA*)malloc( vidInf.width * vidInf.height * sizeof( RGBA) );
//                        egde.sobelOperator( memMan.getInputFrame(), output_pic, vidInf.width, vidInf.height );

			// copy it to the device
			cl_event cToDev = memMan.updateInputFrame();

			// start the kernel
			cl_event sKernel = kernel.start(clrFltr.getCommandQueue(),cToDev, vidInf.width, vidInf.height, 16, 16);

			// copy the results back
			memMan.updateOutputFrame(sKernel);

			// write the image to output
			sprintf(filename,"pictures/pic%i.ppm",i);

			writePPM(memMan.getOutputFrame(),filename,vidInf.width, vidInf.height);
//			writePPM( output_pic,filename,vidInf.width, vidInf.height);
		}
	}
	catch(std::logic_error &e){
		// print an error message
		std::cerr << e.what() << std::endl;
		// close the program
		exit(-1);
	}

	/*
	 * end useful stuff here
	 */
}
