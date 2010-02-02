/*
 * kernelExecutor.cpp
 *
 *  Created on: Jan 23, 2010
 *      Author: lars
 */

#include "opencl/kernelExecutor.h"
#include "opencl/kernel.h"

#include <stdexcept>
#include <iostream>

using namespace GRAVID;

#define MAX_WORKGROUP_SIZE 256

KernelExecutor::KernelExecutor(cl_command_queue cmdQ,
								cl_program program,
								const Image_Effect* imgEffects,
								const unsigned char nb_img_effects,
								VideoInfo &vidInf){

	// initialize members
	this->lastKernel = NULL;
	this->cmdQ = cmdQ;

	// create kernels dynamically
	Kernel *kernel;
	for(int i=0;i< nb_img_effects;i++){
		switch(imgEffects[i]){
		case GRAY_FILTER: kernel = new Kernel(program, "grayFilter", vidInf.width, vidInf.height); break;
		case SEPIA_FILTER: kernel = new Kernel(program, "sepiaFilter", vidInf.width, vidInf.height); break;
		case EDGE_DETECTION: kernel = new Kernel(program, "grayFilter", vidInf.width, vidInf.height); break;
		case GAUSS_BLUR: kernel = new Kernel(program, "grayFilter", vidInf.width, vidInf.height); break;
		}
		// add the kernel to the vector of kernel pointers
		this->kernels.push_back(kernel);
	}
	// TODO replace the grayFilters above with the real names
}

KernelExecutor::~KernelExecutor(){
	if(NULL != this->lastKernel)
		clReleaseEvent(this->lastKernel);

	// delete all allocated kernels
	for(size_t i=0;i<this->kernels.size();i++)
		delete this->kernels[i];
}

void KernelExecutor::errorHappened(const char* error){
	this->errorMsg = error;
	throw std::logic_error(this->errorMsg);
}

void KernelExecutor::executeAll(MemoryManager &memMan){
	for(size_t i=0;i<this->kernels.size();i++){
		Kernel *k = this->kernels[i];
		// all even launches start from the original memory
		if(0 == i%2){
			k->setKernelArgument(0,memMan.getImage_Original());
			// if this kernel isn't the last in line, the result goes to a temporary image
			if(i != this->kernels.size()-1)
				k->setKernelArgument(1,memMan.getImage_Tempory());
			else{
				k->setKernelArgument(1,memMan.getImage_Output());
			}
		}
		else{
			k->setKernelArgument(1,memMan.getImage_Tempory());
			if(i != this->kernels.size()-1)
				k->setKernelArgument(1,memMan.getImage_Original());
			else
				k->setKernelArgument(1,memMan.getImage_Output());
		}

		// start the kernel as soon as the copy process is finished
		this->errorCode = clEnqueueNDRangeKernel(this->cmdQ, k->getNativeKernel(),
													2, NULL, k->getGlobalDim(), k->getLocalDim(),
													0, NULL, &this->lastKernel);

		if(CL_SUCCESS != this->errorCode)
			this->errorHappened("kernel couldn't be enqueued");
	}
}

cl_event KernelExecutor::getLastKernelEvent(){
	return this->lastKernel;
}
