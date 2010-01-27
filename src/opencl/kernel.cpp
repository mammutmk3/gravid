/*
 * kernel.cpp
 *
 *  Created on: Jan 21, 2010
 *      Author: lars
 */

#include "opencl/kernel.h"
#include <CL/cl.h>

using namespace GRAVID;

Kernel::Kernel(const cl_program& program, const char* kernelName){

	// initialize member
	this->kernel = NULL;

	// extract the kernel from the program
	this->kernel = clCreateKernel(program, kernelName,&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't extract the specified kernel from the loaded source");
}

Kernel::~Kernel(){
	if(NULL != this->kernel)
		clReleaseKernel(this->kernel);
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
