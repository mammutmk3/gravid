/*
 * main.cpp
 *
 *  http://jerome.jouvie.free.fr/OpenGl/Tutorials/Tutorial10.php#Tutorial
 */


extern "C"{
	#include <libavformat/avformat.h>
}

#include "codec/videoReader.h"
#include "types.h"
#include "visual/writePPM.h"
#include "visual/glDisplayer.h"
#include "cpu_effects/edgeDetection.h"
#include "cpu_effects/echoEffect.h"

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <stdio.h>

#include "opencl/openCLProgram.h"
#include "opencl/memoryManager.h"
#include "opencl/kernel.h"
#include "opencl/kernelExecutor.h"

#include <CL/cl.h>

#include <time.h>

using namespace GRAVID;

int main(int argc, char** argv){

	// register all available codecs in mpeg
	av_register_all();

	/*
	 * start useful stuff here
	 */

	try{
		// create the decoder for a video file
		VideoReader reader("videos/testvid2.mpg");
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
		
		
		// create a Kernel-executor to launch as many kernels at one frame as wanted
		KernelExecutor kExec(clrFltr.getCommandQueue(),vidInf.width, vidInf.height);
		kExec.addKernel(kernel);

		// tell OpenGL what the OpenCL parts are
		setToolChain(kExec, reader, memMan);
		// display the results
		glDisplay(argc,argv,vidInf.width,vidInf.height,memMan.getOutputFrame());

/*
		EdgeDetection edge;
		EchoEffect echEff;
		
		size_t pic_size = vidInf.width * vidInf.height * sizeof( RGBA);

		RGBA* output_pic = (RGBA*)malloc( vidInf.width * vidInf.height * sizeof( RGBA) );
		RGBA* inpic0 = (RGBA*)malloc( vidInf.width * vidInf.height * sizeof( RGBA) );
		RGBA* inpic1 = (RGBA*)malloc( vidInf.width * vidInf.height * sizeof( RGBA) );
		RGBA* inpic2 = (RGBA*)malloc( vidInf.width * vidInf.height * sizeof( RGBA) );
		RGBA* inpic3 = (RGBA*)malloc( vidInf.width * vidInf.height * sizeof( RGBA) );
		RGBA* inpic4 = (RGBA*)malloc( vidInf.width * vidInf.height * sizeof( RGBA) );
				
		char filename[100];
		for(int i=0;i<50;i++){
			// decode the frame
			reader.decodeNextFrame();

			memcpy( inpic4, inpic3, pic_size);
			memcpy( inpic3, inpic2, pic_size);
			memcpy( inpic2, inpic1, pic_size);
			memcpy( inpic1, inpic0, pic_size);
			memcpy( inpic0, memMan.getInputFrame(), pic_size );

			if ( i <= 4 ) {
				continue;
			}

			//echEff.renderEcho( inpic0, inpic1, inpic2, inpic3, inpic4, output_pic, vidInf.width, vidInf.height );

				
		//sobel cpu                 RGBA* output_pic = (RGBA*)malloc( vidInf.width * vidInf.height * sizeof( RGBA) );
		//sobel cpu              edge.sobelOperator( memMan.getInputFrame(), output_pic, vidInf.width, vidInf.height );

		// copy it to the device
		cl_event cToDev = memMan.updateInputFrame();

		// start the kernel
		cl_event sKernel = kernel.start(clrFltr.getCommandQueue(),cToDev, vidInf.width, vidInf.height, 16, 16);

		// copy the results back
		memMan.updateOutputFrame(sKernel);

		// write the image to output
		// sprintf(filename,"pictures/pic%i.ppm",i);

//			writePPM(memMan.getOutputFrame(),filename,vidInf.width, vidInf.height);
//		writePPM( output_pic ,filename,vidInf.width, vidInf.height);*/
	}
	catch(std::logic_error &e){
		// print an error message
		std::cerr << e.what() << std::endl;
		// close the program
		exit(-1);
	}
}
