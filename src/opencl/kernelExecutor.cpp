/*
 * kernelExecutor.cpp
 *
 *  Created on: Jan 23, 2010
 *      Author: lars
 */

#include "opencl/kernelExecutor.h"
#include <stdexcept>
#include <algorithm>

using namespace GRAVID;

#define MAX_WORKGROUP_SIZE 256

KernelExecutor::KernelExecutor(cl_command_queue cmdQ, const unsigned short globalWidth, const unsigned short globalHeight){

	// initialize members
	this->globalDim = this->localDim = NULL;
	this->lastKernel = NULL;

	this->globalDim = new size_t[2];
	this->globalDim[0] = globalWidth; this->globalDim[1] = globalHeight;
	this->localDim = new size_t[2];

	// find the maximum possible workgroup size
	this->setWorkgroupSize();

	this->cmdQ = cmdQ;
}

void KernelExecutor::setWorkgroupSize(){
	size_t a,b;
	int greaterIndex;
	if(this->globalDim[0] > this->globalDim[1]){
		greaterIndex = 0;
		a = this->globalDim[0];
		this->localDim[1] = 1;
	}
	else{
		greaterIndex = 1;
		a = this->globalDim[1];
		this->localDim[0] = 1;
	}
	b = MAX_WORKGROUP_SIZE;
	size_t tmp;
	// computing the gcd of a and b
	while(0 != b){
		tmp = a%b;
		a = b;
		b = tmp;
	}
	this->localDim[greaterIndex] = a;
}

KernelExecutor::~KernelExecutor(){
	if(NULL != this->globalDim)
		delete this->globalDim;

	if(NULL != this->localDim)
		delete this->localDim;

	if(NULL != this->lastKernel)
		clReleaseEvent(this->lastKernel);
}

void KernelExecutor::errorHappened(const char* error){
	this->errorMsg = error;
	throw std::logic_error(this->errorMsg);
}

void KernelExecutor::addKernel(Kernel &kernel){
	this->kernels.push_back(kernel);
}

void KernelExecutor::executeAll(const cl_event &waitFor){
	for(int i=0;i<this->kernels.size();i++){
		// start the kernel as soon as the copy process is finished
		this->errorCode = clEnqueueNDRangeKernel(cmdQ, this->kernels[i].getNativeKernel(),
													2, NULL, this->globalDim, this->localDim,
													1, &waitFor, &this->lastKernel);
		if(CL_SUCCESS != this->errorCode)
			this->errorHappened("kernel couldn't be enqueued");
	}
}

cl_event KernelExecutor::getLastKernelEvent(){
	return this->lastKernel;
}
