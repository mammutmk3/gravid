/*
 * doubleBuffer.cpp
 *
 *  Created on: Jan 30, 2010
 *      Author: lars
 */

#include "types.h"
#include "opencl/doubleBuffer.h"

#include <stdexcept>
#include <CL/cl.h>

#define FLAGS_HOST CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR
#define FLAGS_DEVICE CL_MEM_READ_WRITE

using namespace GRAVID;

DoubleBuffer::DoubleBuffer(cl_context ctx, cl_command_queue cmdQ,
							const VideoInfo vidInf,
							enum DB_Type db_type){
	// initialize members
	this->ctx = ctx;
	this->cmdQ = cmdQ;
	this->width = vidInf.width;
	this->height = vidInf.height;
	this->db_type = db_type;
	this->frontbuffer = NULL;
	this->backbuffer = NULL;
	this->pFrontbuffer = NULL;
	this->pBackbuffer = NULL;

	// describe the internal image representation
	this->imgFmt.image_channel_data_type = CL_UNSIGNED_INT8;
	this->imgFmt.image_channel_order = CL_RGBA;

	// allocate the 2D image objects
	cl_mem_flags flags = (db_type == DB_HOST)? FLAGS_HOST : FLAGS_DEVICE;
	this->frontbuffer = clCreateImage2D(this->ctx, flags, &this->imgFmt,
										this->width, this->height,
										0, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate 2d image");

	this->backbuffer = clCreateImage2D(this->ctx, flags, &this->imgFmt,
										this->width, this->height,
										0, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
			this->errorHappened("couldn't allocate 2d image");

	this->origin[0] = 0; this->origin[1] = 0; this->origin[2] = 0;
	this->region[0] = vidInf.width; this->region[1] = vidInf.height; this->region[2] = 1;
	// only do the mappings for host type double buffers
	// TODO would be better to use inheritance here (maybe later)
	if(DB_HOST == db_type){
		// map the image objects to standard pointers
		size_t dummy;
		this->pFrontbuffer = (RGBA*) clEnqueueMapImage(this->cmdQ, this->frontbuffer,
														CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, this->origin,
														this->region, &dummy, &dummy,
														0, NULL, NULL, &this->errorCode);
		if(CL_SUCCESS != this->errorCode)
					this->errorHappened("couldn't map to image memory space");
		this->pBackbuffer = (RGBA*) clEnqueueMapImage(this->cmdQ, this->backbuffer,
														CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, this->origin,
														this->region, &dummy, &dummy,
														0, NULL, NULL, &this->errorCode);
		if(CL_SUCCESS != this->errorCode)
					this->errorHappened("couldn't map to image memory space");
	}
}

DoubleBuffer::~DoubleBuffer(){
	if(DB_HOST == this->db_type){
		cl_event unmapping[2];
		// unmap the raw pointers
		this->errorCode = clEnqueueUnmapMemObject(this->cmdQ, this->frontbuffer,
													(void*)this->pFrontbuffer, 0, NULL,
													&unmapping[0]);
		if(CL_SUCCESS != this->errorCode)
			this->errorHappened("couldn't unmap frontbuffer");
		this->errorCode = clEnqueueUnmapMemObject(this->cmdQ, this->backbuffer,
													(void*)this->pBackbuffer, 0, NULL,
													&unmapping[1]);
		if(CL_SUCCESS != this->errorCode)
			this->errorHappened("couldn't unmap frontbuffer");

		// wait for the unmapping to finish
		clWaitForEvents(2,unmapping);
	}

	// release the memory objects
	if(NULL != this->frontbuffer)
		clReleaseMemObject(this->frontbuffer);

	if(NULL != this->backbuffer)
		clReleaseMemObject(this->backbuffer);
}

void DoubleBuffer::swap(){
	// swap the OpenCL memory objects
	cl_mem tmp = this->frontbuffer;
	this->frontbuffer = this->backbuffer;
	this->backbuffer = tmp;

	if(DB_HOST == this->db_type){
		// swap the raw pointers as well
		RGBA* tmp1 = this->pFrontbuffer;
		this->pFrontbuffer = this->pBackbuffer;
		this->pBackbuffer = tmp1;
	}
}

void DoubleBuffer::errorHappened(const char* error){
	this->errorMsg = error;
	throw std::logic_error(this->errorMsg);
}
