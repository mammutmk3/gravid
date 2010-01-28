/*
 * kernelExecutor.h
 *
 *  Created on: Jan 23, 2010
 *      Author: lars
 */

#ifndef KERNELEXECUTOR_H_
#define KERNELEXECUTOR_H_

#include "opencl/kernel.h"

#include <vector>
#include <string>
#include <CL/cl.h>

namespace GRAVID{

	class KernelExecutor{
	private:
		std::vector<Kernel> kernels;
		cl_command_queue cmdQ;
		size_t *globalDim;
		size_t *localDim;
		cl_event lastKernel;

		std::string errorMsg;
		cl_int errorCode;
		/**
		 * takes a C-string as error message an throws a std::logic_error excetion including it
		 *
		 * @param error contains the error message that can be read from the exception
		 */
		void errorHappened(const char* error);

		/**
		 * find maximum possible work group size and set it to localDim
		 */
		void setWorkgroupSize();

	public:

		KernelExecutor(cl_command_queue cmdQ, const unsigned short globalWidth, const unsigned short globalHeight);
		~KernelExecutor();

		/**
		 * returns the number of registered kernels
		 *
		 * @return the number of registered kernels
		 */
		unsigned char getKernelCount(){return this->kernels.size();}

		/**
		 * registers another Kernel for execution
		 *
		 * @param kernel the kernel to execute
		 */
		void addKernel(Kernel &kernel);

		/**
		 * executes all kernels enqueued asynchronously
		 */
		void executeAll(const cl_event &waitFor);

		/**
		 * provides an event to track if alle launched kernels have finished work,
		 * as kernels among themselves don't get executed out of order
		 *
		 * @return the event created by enqueuing the last kernel
		 */
		cl_event getLastKernelEvent();
	};
}

#endif /* KERNELEXECUTOR_H_ */
