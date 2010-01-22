/*
 * memoryManager.cpp
 *
 *  Created on: Jan 20, 2010
 *      Author: lars
 */

#include "opencl/memoryManager.h"
#include "types.h"

#include <CL/cl.h>
#include <stdexcept>

using namespace GRAVID;

MemoryManager::MemoryManager(cl_context clCtx, cl_command_queue cmdQ, VideoInfo vidInf){
	// initialize members
	this->srcImage2D_dev = NULL;
	this->srcImage2D_host = NULL;
	this->dstImage2D_dev = NULL;
	this->dstImage2D_host = NULL;

	// assign the initialized command queue
	this->cmdQ = cmdQ;

	// initialize the image format structure
	this->imgFmt.image_channel_data_type = CL_UNSIGNED_INT8;
	this->imgFmt.image_channel_order = CL_RGBA;

	// allocate the pinned input image on the host
	this->srcImage2D_host = clCreateImage2D(clCtx,
										CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
										&this->imgFmt, vidInf.width, vidInf.height,
										0, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate  pinned input image");

	// allocate the pinned output image on the host
	this->dstImage2D_host = clCreateBuffer(clCtx,
										CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
										vidInf.byteSize, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
			this->errorHappened("couldn't allocate  pinned output image");

	// allocate the device input image
	this->srcImage2D_dev = clCreateImage2D(clCtx, CL_MEM_READ_ONLY,
											&this->imgFmt, vidInf.width, vidInf.height,
											0, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
			this->errorHappened("couldn't allocate  device's input image");

	// allocate the device output image
	this->dstImage2D_dev = clCreateBuffer(clCtx,
										CL_MEM_WRITE_ONLY,
										vidInf.byteSize,
										NULL,
										&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
			this->errorHappened("couldn't allocate device's input image");

	// map standard pointers to pinned memory
	this->origin[0] = 0; this->origin[1] = 0; this->origin[2] = 0;
	this->region[0] = vidInf.width; this->region[1] = vidInf.height; this->region[2] = 1;
	size_t dummy;
	this->srcImage_mapped = (RGBA*) clEnqueueMapImage(this->cmdQ, this->srcImage2D_host,
														CL_TRUE, CL_MAP_WRITE, this->origin,
														this->region, &dummy, &dummy,
														0, NULL, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
				this->errorHappened("couldn't map to pinned input image");
	this->dstImage_mapped = (RGBA*) clEnqueueMapBuffer(this->cmdQ, this->dstImage2D_host,
															CL_TRUE, CL_MAP_READ, 0,
															vidInf.byteSize, 0, NULL, NULL,
															&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
				this->errorHappened("couldn't map to pinned output image");

	if(CL_SUCCESS != this->errorCode)
					this->errorHappened("couldn't create the sampler object");
}

void MemoryManager::errorHappened(const char* error){
	this->errorMsg = error;
	throw std::logic_error(this->errorMsg);
}

MemoryManager::~MemoryManager(){
	//unmap the pointers to the pinned memory buffers
	this->errorCode = clEnqueueUnmapMemObject(this->cmdQ, srcImage2D_host, srcImage_mapped,0,NULL,NULL);
	this->errorCode |= clEnqueueUnmapMemObject(this->cmdQ, dstImage2D_host, dstImage_mapped,0,NULL,NULL);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't unmap pinned memory");

	// release the pinned and device Images images
	if(NULL != this->srcImage2D_host)
		clReleaseMemObject(srcImage2D_host);
	if(NULL != this->srcImage2D_dev)
		clReleaseMemObject(srcImage2D_dev);
	if(NULL != this->dstImage2D_host)
		clReleaseMemObject(dstImage2D_host);
	if(NULL != this->dstImage2D_dev)
		clReleaseMemObject(dstImage2D_dev);
}

cl_event MemoryManager::updateInputFrame(){
	// writing decoded data from pinned host memory to device memory (asynchronously)
	this->errorCode = clEnqueueWriteImage(this->cmdQ, this->srcImage2D_dev, CL_FALSE,
											this->origin, this->region, 0, 0, (void *)this->srcImage_mapped,
											0, NULL, &this->copyToDevice);
	if(CL_SUCCESS != this->errorCode)
			this->errorHappened("error while copying to device image");

	// return the track event
	return this->copyToDevice;
}

cl_event MemoryManager::updateOutputFrame(const cl_event &waitingFor){

	// copy back the image as soon as the kernel is finished
	this->errorCode = clEnqueueReadBuffer(this->cmdQ, this->dstImage2D_dev, CL_FALSE,
							0, sizeof(RGBA)*this->region[0]*this->region[1],
							(void*)this->dstImage_mapped, 1, &waitingFor, &this->copyFromDevice);

	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't read output image from device buffer");

	// return the track event
	return this->copyFromDevice;
}
