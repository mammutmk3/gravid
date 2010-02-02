/*
 * memoryManager.cpp
 *
 *  Created on: Jan 20, 2010
 *      Author: lars
 */

#include "opencl/memoryManager.h"
#include "opencl/doubleBuffer.h"
#include "visual/writePPM.h"
#include "types.h"

#include <CL/cl.h>
#include <stdexcept>
#include <iostream>

using namespace GRAVID;

MemoryManager::MemoryManager(cl_context clCtx, cl_command_queue cmdQ, VideoInfo vidInf){
	// initialize members
	this->pHost_in = NULL;
	this->pHost_out = NULL;
	this->pDevice_in = NULL;
	this->pDevice_out = NULL;
	this->copyToDevice_event = NULL;
	this->copyFromDevice_event = NULL;
	this->tmpImage = NULL;

	// allocate all the double buffers
	this->pHost_in = new DoubleBuffer(clCtx, cmdQ, vidInf, DB_HOST);
	this->pHost_out = new DoubleBuffer(clCtx, cmdQ, vidInf, DB_HOST);
	this->pDevice_in = new DoubleBuffer(clCtx, cmdQ, vidInf, DB_DEVICE);
	this->pDevice_out = new DoubleBuffer(clCtx, cmdQ, vidInf, DB_DEVICE);

	// assign the initialized command queue
	this->cmdQ = cmdQ;

	// initialize the image format structure
	this->imgFmt.image_channel_data_type = CL_UNSIGNED_INT8;
	this->imgFmt.image_channel_order = CL_RGBA;

	// allocate space for the intermediate image on the device
	this->tmpImage = clCreateImage2D(clCtx, CL_MEM_READ_WRITE, &this->imgFmt,
										vidInf.width, vidInf.height,
										0, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate intermediate image");
}

void MemoryManager::errorHappened(const char* error){
	this->errorMsg = error;
	throw std::logic_error(this->errorMsg);
}

MemoryManager::~MemoryManager(){
	if(NULL != this->tmpImage)
		clReleaseMemObject(this->tmpImage);

	if(NULL != this->pHost_in)
		delete this->pHost_in;
	if(NULL != this->pHost_out)
		delete this->pHost_out;
	if(NULL != this->pDevice_in)
		delete this->pDevice_in;
	if(NULL != this->pDevice_out)
		delete this->pDevice_out;
}

void MemoryManager::copyToDevice(cl_event kernelFinished){
	// wait for the last kernel launch to finsih (if any has been started at all)
	if(NULL != kernelFinished)
		clWaitForEvents(1,&kernelFinished);
	// wait for preceeded copy instruction to the device
	if(NULL != this->copyToDevice_event)
		clWaitForEvents(1,&this->copyToDevice_event);

	// swap the input buffers both on the host and the device side
	this->pHost_in->swap();
	this->pDevice_in->swap();

	// do an asynchronous copy to the device
	this->errorCode = clEnqueueWriteImage(this->cmdQ, this->pDevice_in->getBackBuffer(), CL_FALSE,
											this->pDevice_in->getOrigin(), this->pDevice_in->getRegion(), 0, 0,
											(void *)this->pHost_in->getFrontBuffer_Pointer(),
											0, NULL, &this->copyToDevice_event);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't enqueue copy process to the device");
}

void MemoryManager::copyFromDevice(cl_event kernelFinished){
	// wait for the last kernel launch to finsih (if any has been started at all)
	if(NULL != kernelFinished)
		clWaitForEvents(1,&kernelFinished);
	// wait for preceeded copy instruction to the device
	if(NULL != this->copyFromDevice_event)
		clWaitForEvents(1,&this->copyFromDevice_event);

	// swap the input buffers both on the host and the device side
	this->pHost_out->swap();
	this->pDevice_out->swap();
	// do an asynchronous copy from the device
	this->errorCode = clEnqueueReadImage(this->cmdQ, this->pDevice_out->getFrontBuffer(), CL_FALSE,
											this->pDevice_in->getOrigin(), this->pHost_in->getRegion(), 0, 0,
											(void *)this->pHost_out->getBackBuffer_Pointer(),
											0, NULL, &this->copyFromDevice_event);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't enqueue copy process from the device");
}
