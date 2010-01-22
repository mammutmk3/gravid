/*
 * OpenCLProgram.cpp
 *
 *  Created on: Jan 13, 2010
 *      Author: lars
 */

#include "opencl/openCLProgram.h"
#include "types.h"

#include <CL/cl.h>
#include <stdexcept>
#include <string>
#include <fstream>

using namespace GRAVID;

OpenCLProgram::OpenCLProgram(const char* filename){
	// initialize members
	this->clCtx = NULL;
	this->cmdQ = NULL;
	this->clProgram = NULL;

	// retrieve platform ids
	this->pPlatformIDs = NULL;
	clGetPlatformIDs(NULL,NULL,&(this->nb_platforms));
	// stop, if there is no OpenCL platform
	if(0 == this->nb_platforms)
		this->errorHappened("no OpenCL capable platforms available");
	// some memory is allocated that has to be freed in the destructor
	this->pPlatformIDs = new cl_platform_id[this->nb_platforms];
	this->errorCode = clGetPlatformIDs(this->nb_platforms,this->pPlatformIDs,NULL);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't get the platform ids");

	// obtain the list of GPUs for the first platform
	this->pDeviceIDs = NULL;
	clGetDeviceIDs(this->pPlatformIDs[0],CL_DEVICE_TYPE_GPU, NULL, NULL, &this->nb_devices);
	// stop, if there is no device available
	if(0 == this->nb_devices)
			this->errorHappened("no device available for the first platform");
	this->pDeviceIDs = new cl_device_id[this->nb_devices];
	this->errorCode = clGetDeviceIDs(this->pPlatformIDs[0],CL_DEVICE_TYPE_GPU, this->nb_devices, this->pDeviceIDs, NULL);
	if(CL_SUCCESS != this->errorCode)
			this->errorHappened("couldn't get the device ids");

	// create a context for the devices to use
	cl_context_properties clContProperties[3];
	clContProperties[0] = CL_CONTEXT_PLATFORM;
	clContProperties[1] = (cl_context_properties)this->pPlatformIDs[0];
	clContProperties[2] = 0;
	this->clCtx = clCreateContext(clContProperties,this->nb_devices,this->pDeviceIDs,NULL,NULL,&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't create context");

	// creating the commandQueue for the kernel
	this->cmdQ = clCreateCommandQueue(this->clCtx, this->pDeviceIDs[0], NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't create command queue");

	// load the actual program
	this->loadProgram(filename);
}

void OpenCLProgram::errorHappened(const char* error){
	this->errorMsg = error;
	throw std::logic_error(errorMsg);
}

OpenCLProgram::~OpenCLProgram(){

	if(NULL != this->cmdQ)
		clReleaseCommandQueue(this->cmdQ);

	if(NULL != this->clProgram)
		clReleaseProgram(this->clProgram);

	if(NULL != this->clCtx)
		clReleaseContext(this->clCtx);

	if(NULL != this->pDeviceIDs)
			delete(this->pDeviceIDs);

	if(NULL != this->pPlatformIDs)
		delete(this->pPlatformIDs);
}

std::string OpenCLProgram::readProgram(const char* filename){
	std::ifstream inFile;
	char buffer[256];
	std::string resultString;

	inFile.open(filename,std::ifstream::in);
	if(inFile.fail())
		this->errorHappened("kernel source-file not found");

	while(inFile.good()){
		inFile.getline(buffer,256);
		resultString.append(buffer);
	}

	inFile.close();
	return resultString;
}

void OpenCLProgram::loadProgram(const char* filename){

	// retrieve the kernel source
	std::string kernelString = this->readProgram(filename);
	const char* kernel_cstr = kernelString.c_str();
	size_t kernelLength = (size_t)kernelString.length();
	this->clProgram = clCreateProgramWithSource(this->clCtx, 1, &kernel_cstr, &kernelLength,&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't create program from source");

	// build the program
	// TODO check if there maybe a built version already exists
	this->errorCode = clBuildProgram(this->clProgram,NULL,NULL,NULL,NULL,NULL);
	// get Build Information
	size_t actualSize;
	clGetProgramBuildInfo(this->clProgram,this->pDeviceIDs[0],CL_PROGRAM_BUILD_LOG,NULL,NULL,&actualSize);
	char errorMessage[actualSize];
	clGetProgramBuildInfo(this->clProgram,this->pDeviceIDs[0],CL_PROGRAM_BUILD_LOG,actualSize,(void*)errorMessage,NULL);
	if(CL_SUCCESS != this->errorCode){
		std::string totalError = "Building process failed with the following error log:\n";
		totalError.append(errorMessage);
		this->errorHappened(totalError.c_str());
	}
}
