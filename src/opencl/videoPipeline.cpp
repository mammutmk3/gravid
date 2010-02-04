/*
 * videoPipeline.cpp
 *
 *  Created on: Feb 3, 2010
 *      Author: lars
 */

#include "opencl/videoPipeline.h"

using namespace GRAVID;

VideoPipeline::VideoPipeline(cl_context ctx, cl_command_queue cmdQ, const unsigned int fifo_length,
								size_t image_width, size_t image_height){
	// initialize class members
	this->cmdQ = cmdQ;
	this->ctx = ctx;
	this->fifo_length = fifo_length;
	this->image_width = image_width;
	this->image_height = image_height;
	this->devImage2D_out = NULL;
	this->devImage3D_in = NULL;
	this->hostImage2D_in = NULL;
	this->hostImage2D_out = NULL;

	// specify the image format that the images a represented with
	cl_image_format imgFmt;
	imgFmt.image_channel_data_type = CL_UNSIGNED_INT8;
	imgFmt.image_channel_order = CL_RGBA;

	this->hostImage2D_in = clCreateImage2D(this->ctx, CL_MEM_ALLOC_HOST_PTR, &imgFmt,
										this->image_width, this->image_height, 0, NULL,
										&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate 2D input image on the host");

	this->devImage3D_in = clCreateImage3D(this->ctx, CL_MEM_READ_ONLY, &imgFmt,
											this->image_width, this->image_height, this->fifo_length,
											0,0, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate 3D input image on the device");

	this->devImage2D_out = clCreateImage2D(this->ctx, CL_MEM_WRITE_ONLY, &imgFmt,
										this->image_width, this->image_height, 0, NULL,
										&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate 2D output image on the device");

	this->hostImage2D_out = clCreateImage2D(this->ctx, CL_MEM_ALLOC_HOST_PTR, &imgFmt,
										this->image_width, this->image_height, 0, NULL,
										&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate 2D output image on the host");

	/**
	 * map the images on the host to standard pointers, so the decoder and encoder can access them
	 */
	// TODO continue here
}

VideoPipeline::~VideoPipeline(){
	if(NULL != this->hostImage2D_in)
		clReleaseMemObject(this->hostImage2D_in);
}
