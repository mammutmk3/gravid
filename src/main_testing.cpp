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
		VideoReader reader("videos/testvid5.mpg");
		VideoInfo vidInf = reader.getVideoInfo();
		
		// create the new OpenCL program to process that video
		OpenCLProgram clrFltr("src/opencl/kernels/echoEffect.cl");
		// create the streaminig pipeline
		MemoryManager memMan(clrFltr.getContext(),clrFltr.getCommandQueue(),vidInf);
		// associate the pinned memory with the buffer of ffmpeg
		reader.changeFrameBuffer(memMan.getInputFrame());
		
		// create a kernel for an effect
		Kernel kernel(clrFltr.getProgram(),"echoeffect");


		//frames aus dem video holen		
		size_t pic_size_2d = vidInf.width * vidInf.height * sizeof( RGBA);
		size_t pic_size_3d = vidInf.width * vidInf.height * sizeof( RGBA) * 5;
		RGBA* output_pic = (RGBA*)malloc( vidInf.width * vidInf.height * sizeof( RGBA) );
		RGBA* inpic_3d = (RGBA*)malloc( pic_size_3d );

		// decode the frame
		reader.decodeNextFrame();
		memcpy( inpic_3d, memMan.getInputFrame(), pic_size_2d );
		//memcpy( output_pic, memMan.getInputFrame(), pic_size_2d );
		reader.decodeNextFrame();
		memcpy( inpic_3d + vidInf.width * vidInf.height, memMan.getInputFrame(), pic_size_2d );
		reader.decodeNextFrame();
		memcpy( inpic_3d + vidInf.width * vidInf.height * 2, memMan.getInputFrame(), pic_size_2d );
		reader.decodeNextFrame();
		memcpy( inpic_3d + vidInf.width * vidInf.height * 3, memMan.getInputFrame(), pic_size_2d );
		reader.decodeNextFrame();
		memcpy( inpic_3d + vidInf.width * vidInf.height * 4, memMan.getInputFrame(), pic_size_2d );

		printf("read the frames from the videostream\n");
		
		// for the errorCode
		cl_int errorCode;
		
		// initialize the image format structure
		cl_image_format imgFmt;
		imgFmt.image_channel_data_type = CL_UNSIGNED_INT8;
		imgFmt.image_channel_order = CL_RGBA;
		
		// pinned input 3Dimage on the host side
		cl_mem srcImage3D_host;
		// pinned output 2Dimage on the host side
		cl_mem dstImage2D_host;
		// device input 3Dimage
		cl_mem srcImage3D_dev;
		// device output 2Dimage
		cl_mem dstImage2D_dev;
		// pointer to the raw pinned input host data
		RGBA* srcImage_mapped;
		// pointer to the raw pinned output host data
		RGBA* dstImage_mapped;
		// origin and region sample of the image
		size_t origin[3];
		size_t region[3];
				
		// pinned input 3D-Image on the host
		srcImage3D_host =  clCreateImage3D( clrFltr.getContext(), CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &imgFmt, vidInf.width, vidInf.height, 5, 0, 0, NULL, &errorCode);
		if(CL_SUCCESS != errorCode)
			printf("pinned input 3D-Image on the host\n");

		// input 3D-Img on device
		srcImage3D_dev =  clCreateImage3D( clrFltr.getContext(),CL_MEM_READ_ONLY, &imgFmt, vidInf.width, vidInf.height, 5, 0, 0, NULL, &errorCode);
		if(CL_SUCCESS != errorCode)
			printf("input 3D-Image on device\n");
		
		// allocate the pinned output image on the host
		dstImage2D_host = clCreateBuffer(clrFltr.getContext(), CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, vidInf.byteSize, NULL, &errorCode);
		if(CL_SUCCESS != errorCode)
			printf("output 2D-Image on the host\n");

		// allocate the device output image
		dstImage2D_dev = clCreateBuffer(clrFltr.getContext(), CL_MEM_WRITE_ONLY, vidInf.byteSize, NULL, &errorCode);
		if(CL_SUCCESS != errorCode)
			printf("pinned output 2D-Image on the host\n");
		
		
		// map standard pointers to pinned memory
		origin[0] = 0; origin[1] = 0; origin[2] = 0;
		region[0] = vidInf.width; region[1] = vidInf.height; region[2] = 5;
		size_t dummy;

		// Map to Image
		srcImage_mapped = (RGBA*) clEnqueueMapImage(clrFltr.getCommandQueue(), srcImage3D_host, CL_TRUE, CL_MAP_WRITE, origin, region, &dummy, &dummy, 0, NULL, NULL, &errorCode);
			if(CL_SUCCESS != errorCode)
			  printf("couldn't map to pinned input image\n");
		
		srcImage_mapped = inpic_3d;
		printf("zuweisung des 3d-images\n");
	
			
		// Map to Image
		dstImage_mapped = (RGBA*) clEnqueueMapImage(clrFltr.getCommandQueue(), srcImage3D_host, CL_TRUE, CL_MAP_WRITE, origin, region, &dummy, &dummy, 0, NULL, NULL, &errorCode);
			if(CL_SUCCESS != errorCode)
				printf("couldn't map to pinned input image\n");

		dstImage_mapped = output_pic;
		printf("zuweisung des 2d-output-images\n");
			
			
		cl_event cpyToDev;	
		// copy 3D-Image-Data	
		errorCode = clEnqueueWriteImage(clrFltr.getCommandQueue(), srcImage3D_dev, CL_TRUE, origin, region, 0, 0, (void *)srcImage_mapped, 0, NULL, &cpyToDev);

		if(CL_SUCCESS != errorCode)
			printf("error while copying to device image\n");
		
		// set the kernel parameters
		kernel.setKernelArgument(0, srcImage3D_dev);
		kernel.setKernelArgument(1, dstImage2D_dev);
		int randomness = 1;
		int opacity = 102;
		kernel.setKernelArgument(2, randomness);
		kernel.setKernelArgument(3, opacity);
		
		cl_event kernelsFinished;
		// create a Kernel-executor to launch as many kernels at one frame as wanted
		KernelExecutor kExec( clrFltr.getCommandQueue(), vidInf.width, vidInf.height );
		kExec.addKernel(kernel);
		printf("added the kernel\n");
		kExec.executeAll( cpyToDev );
		kernelsFinished = kExec.getLastKernelEvent();
		printf("executed Kernel\n");
		
		// copy back
		errorCode = clEnqueueReadBuffer(clrFltr.getCommandQueue(), dstImage2D_dev, CL_TRUE, 0, pic_size_2d, (void*)dstImage_mapped, 1, &kernelsFinished, NULL);
		if(CL_SUCCESS != errorCode)
			printf("error while copying the image back");

		char filename[100];
		
		// write the image to output
		sprintf(filename,"pictures/pic%i.ppm", 0);
		writePPM( dstImage_mapped, filename, vidInf.width, vidInf.height);

		
		errorCode = clEnqueueUnmapMemObject(clrFltr.getCommandQueue(), srcImage3D_host, srcImage_mapped,0,NULL,NULL);
		errorCode |= clEnqueueUnmapMemObject(clrFltr.getCommandQueue(), dstImage2D_host, dstImage_mapped,0,NULL,NULL);
		
		// release the pinned and device Images images
		if(NULL != srcImage3D_dev)
			clReleaseMemObject(srcImage3D_dev);
		if(NULL != srcImage3D_host)
			clReleaseMemObject(srcImage3D_host);
		if(NULL != dstImage2D_dev)
			clReleaseMemObject(dstImage2D_dev);
		if(NULL != dstImage2D_host)
			clReleaseMemObject(dstImage2D_host);

	}
	catch(std::logic_error &e){
		// print an error message
		std::cerr << e.what() << std::endl;
		// close the program
		exit(-1);
	}
}
