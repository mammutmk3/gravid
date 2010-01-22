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

cl_event Kernel::start(const cl_command_queue& cmdQ, const cl_event& waitFor,
				const size_t globalWidth, const size_t globalHeight,
				const size_t localWidth, const size_t localHeight){

	const size_t glDim[2] = {globalWidth, globalHeight};
	const size_t loDim[2] = {localWidth, localHeight};
	// start the kernel as soon as the copy process is finished
	this->errorCode = clEnqueueNDRangeKernel(cmdQ, this->kernel,
												2, NULL, glDim, loDim,
												1, &waitFor, &this->kernelFinished);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("kernel couldn't be enqueued");

	return this->kernelFinished;
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
