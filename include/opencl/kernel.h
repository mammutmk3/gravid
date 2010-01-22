/*
 * kernel.h
 *
 *  Created on: Jan 21, 2010
 *      Author: lars
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <CL/cl.h>
#include <string>
#include <stdexcept>

namespace GRAVID{

	class Kernel{
	private:
		// the actual kernel itself
		cl_kernel kernel;
		// determines, wether the kernel has finished, or not
		cl_event kernelFinished;

		// the error message of the exception that is thrown, whenever something unexpected happens
		std::string errorMsg;
		// the cl error code
		cl_int errorCode;

		/**
		 * takes a C-string as error message an throws a std::logic_error excetion including it
		 *
		 * @param error contains the error message that can be read from the exception
		 */
		void errorHappened(const char* error);

	public:
		Kernel(const cl_program& program, const char* kernelName);
		~Kernel();

		/**
		 * set an argument of the kernel
		 *
		 * @param kernel the kernel which's parameter is beeing set
		 * @param index the index of the argument in the list of kernel parameters. Must be within [0,getNBKernelArgs-1].
		 * @param argumentValue the memmory object that is supposed to be the kernels argument
		 */
		template <typename T>
		void setKernelArgument(const unsigned char index, const T& argumentValue){
			// set the kernel argument
			this->errorCode = clSetKernelArg(this->kernel,index,sizeof(T),(void*)&argumentValue);
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

		/**
		 * starts the kernel
		 *
		 * @param cmdQ the OpenCL command queue that the kernel is beeing enqueued in
		 * @param waitFor the OpenCL event that must finish before the kernel will start
		 * @param globalWidth the total width of all work items
		 * @param globalHeight the total height of all work items
		 * @param localWidth the width of a work group
		 * @param localHeight the height of a work group
		 *
		 * @return the OpenCL event with which one can determine if the kernel has finished yet
		 */
		cl_event start(const cl_command_queue& cmdQ, const cl_event& waitFor,
								const size_t globalWidth, const size_t globalHeight,
								const size_t localWidth, const size_t localHeight);

		/**
		 * returns the number of arguments the loaded kernel owns
		 */
		unsigned char getNBKernelArgs();
	};
}

#endif /* KERNEL_H_ */
