/*
 * kernelExecutor.h
 *
 *  Created on: Jan 23, 2010
 *      Author: lars
 */

#ifndef KERNELEXECUTOR_H_
#define KERNELEXECUTOR_H_

#include "types.h"
#include "opencl/kernel.h"
#include "opencl/memoryManager.h"

#include <vector>
#include <string>
#include <CL/cl.h>

namespace GRAVID{

	class KernelExecutor{
	private:
		std::vector<Kernel*> kernels;
		cl_command_queue cmdQ;

		// event that determines whether the last enqueued kernel has finifhed it's computations
		cl_event lastKernel;

		std::string errorMsg;
		cl_int errorCode;
		/**
		 * takes a C-string as error message an throws a std::logic_error excetion including it
		 *
		 * @param error contains the error message that can be read from the exception
		 */
		void errorHappened(const char* error);

	public:

		KernelExecutor(cl_command_queue cmdQ,
						cl_program program,
						const Image_Effect* imgEffects,
						const unsigned char nb_img_effects,
						VideoInfo &vidInf);
		~KernelExecutor();

		/**
		 * returns the number of registered kernels
		 *
		 * @return the number of registered kernels
		 */
		unsigned char getKernelCount(){return this->kernels.size();}

		/**
		 * executes all kernels enqueued asynchronously
		 *
		 * @param memMan the streaming pipeline manager that controls the images for the kernels
		 */
		void executeAll(MemoryManager &memMan);

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
