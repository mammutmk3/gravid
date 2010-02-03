/*
 * kernel.cpp
 *
 *  Created on: Jan 21, 2010
 *      Author: lars
 */

#include "opencl/kernel.h"
#include <CL/cl.h>
#include <iostream>

using namespace GRAVID;

#define MAX_WORKGROUP_SIZE 256

Kernel::Kernel(const cl_program& program, const char* kernelName,
				const size_t globalWidth, const size_t globalHeight){

	// initialize member
	this->kernel = NULL;
	this->global[0] = globalWidth; this->global[1] = globalHeight;

	// extract the kernel from the program
	this->kernel = clCreateKernel(program, kernelName,&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't extract the specified kernel from the loaded source");

	// calculate workgroup size
	this->guessWorkgroupSize();
}

Kernel::~Kernel(){
	if(NULL != this->kernel)
		clReleaseKernel(this->kernel);
}

void Kernel::guessWorkgroupSize(){
	// holds the currently determined maximum workgroup size
	unsigned short size;
	for(int i=0;i<2;i++){
		size = MAX_WORKGROUP_SIZE;
		for(char j=1;j<=9;j++){
			if(0 == this->global[i] % size){
				this->local[i] = size;
				break;
			}
			// division by 2
			size >>= 1;
		}
	}
	while(this->local[0] * this->local[1] > MAX_WORKGROUP_SIZE){
		if(this->local[0] > this->local[1])
			this->local[0] >>= 1;
		else
			this->local[1] >>= 1;
	}
}

void Kernel::setWorkgroupDim(unsigned short width, unsigned short height){
	this->local[0] = width;
	this->local[1] = height;
}

void Kernel::errorHappened(const char* error){
	this->errorMsg = error;
	throw std::logic_error(this->errorMsg);
}

unsigned char Kernel::getNBKernelArgs(){
	cl_uint nb_args;
	this->errorCode = clGetKernelInfo(this->kernel,CL_KERNEL_NUM_ARGS,sizeof(cl_uint),(void*)nb_args,NULL);
	if(CL_INVALID_KERNEL == this->errorCode)
		this->errorHappened("this kernel is invalid");
	else if(CL_SUCCESS != this->errorCode)
		this->errorHappened("undefined error occured while retrieving number of kernel arguments");
	return (unsigned char)nb_args;
}
