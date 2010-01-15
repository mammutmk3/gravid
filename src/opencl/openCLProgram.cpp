/*
 * OpenCLProgram.cpp
 *
 *  Created on: Jan 13, 2010
 *      Author: lars
 */
/*
#include "openCLProgram.h"

#include <CL/cl.h>
#include <stdexcept>
#include <string>
#include <fstream>

using namespace GRAVID;

OpenCLProgram::OpenCLProgram() throw(std::logic_error){

	// retrieve platform ids
	this->pPlatformIDs = NULL;
	clGetPlatformIDs(NULL,NULL,&(this->nb_platforms));
	// stop, if there is no OpenCL platform
	if(0 == this->nb_platforms)
		this->errorHappened("no OpenCL capable platforms available");
	this->pPlatformIDs = new cl_platform_id[this->nb_platforms]; // some memory is allocated that has to be freed in the destructor
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
}

void OpenCLProgram::errorHappened(const char* error) throw(std::logic_error){
	std::string errorMsg = error;
	throw std::logic_error(errorMsg);
}

OpenCLProgram::~OpenCLProgram(){

	// preserve that order, cause some objects depend on each other
	// best advide: release memory objects in reverse order that they have been allocated

	// release allocated memory objects
	// TODO actually do so, as soon as object oriented memory object management get's supported


	clReleaseCommandQueue(this->cmdQ);

	clReleaseKernel(this->clKernel);

	clReleaseProgram(this->clProgram);

	clReleaseContext(this->clCtx);

	if(NULL != this->pDeviceIDs)
			delete(this->pDeviceIDs);

	if(NULL != this->pPlatformIDs)
		delete(this->pPlatformIDs);
}

std::string OpenCLProgram::readKernel(const char* filename){
	std::ifstream inFile;
	char buffer[256];
	std::string resultString;

	inFile.open(filename,std::ifstream::in);
	while(inFile.good()){
		inFile.getline(buffer,256);
		resultString.append(buffer);
	}

	inFile.close();
	return resultString;
}

void OpenCLProgram::loadProgram(const char* filename, const char* kernelName) throw(std::logic_error){
	// retrieve the kernel source
	std::string kernelString = this->readKernel("../kernels/mandelbrot.cl");
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

	// extract the kernel within the program
	this->clKernel = clCreateKernel(this->clProgram,kernelName,&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't extract the specified kernel from the loaded source");

	// creating the commandQueue for the kernel
	this->cmdQ = clCreateCommandQueue(this->clCtx, this->pDeviceIDs[0], NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't create command queue");
}

unsigned char OpenCLProgram::getNBKernelArgs() throw(std::logic_error){
	cl_uint nb_args;
	this->errorCode = clGetKernelInfo(this->clKernel,CL_KERNEL_NUM_ARGS,sizeof(cl_uint),(void*)nb_args,NULL);
	if(CL_INVALID_KERNEL == this->errorCode)
		this->errorHappened("cannot retrieve argument number without a loaded kernel");
	else if(CL_SUCCESS != this->errorCode)
		this->errorHappened("undefined error occured while retrieving number of kernel arguments");
	return (unsigned char)nb_args;
}

template <typename T>
void OpenCLProgram::setKernelArgument(const unsigned char index, const size_t size, T& argumentValue) throw(std::logic_error){
	// set the kernel argument
	this->errorCode = clSetKernelArg(this->clKernel,0,size,(void*)&argumentValue);
	switch(this->errorCode){
		case CL_INVALID_KERNEL: this->errorHappened("couldn't set argument, because kernel hasn't been loaded yet."); break;
		case CL_INVALID_ARG_INDEX : this->errorHappened("couldn't set argument, because the specified index is invalid."); break;
		case CL_INVALID_MEM_OBJECT : this->errorHappened("couldn't set argument, because the memory object is invalid."); break;
		case CL_INVALID_ARG_SIZE : this->errorHappened("couldn't set argument, because the size of the argument value and the specified size doesnt match."); break;
		default: break;
	}
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("undefined error happened while setting the kernel's argument");
}
*/