/*
 * openCLProgram.h
 *
 *  Created on: Jan 13, 2010
 *      Author: lars
 */

#ifndef OPENCLPROGRAM_H_
#define OPENCLPROGRAM_H_

#include <CL/cl.h>
#include <stdexcept>
#include <string>

namespace GRAVID{

	/**
	 * own wrapper class for the common OpenCL tasks
	 */
	class OpenCLProgram{
		private:
			// for opencl return codes
			cl_int errorCode;

			// the number of OpenCL capable platforms
			cl_uint nb_platforms;
			// the available platform ids
			cl_platform_id *pPlatformIDs;

			// the number of devices for the first platform
			cl_uint nb_devices;
			// the ids of the devices available for the first platform
			cl_device_id *pDeviceIDs;

			// the OpenCL context
			cl_context clCtx;
			// the current cl-program
			cl_program clProgram;
			// the loaded kernel within the loaded source
			cl_kernel clKernel;
			// the command queue for that kernel
			cl_command_queue cmdQ;

			/**
			 * takes a C-string as error message an throws a std::logic_error excetion including it
			 *
			 * @param error contains the error message that can by the exception
			 */
			void errorHappened(const char* error) throw(std::logic_error);

			/**
			 * reads a file containg an OpenCL program and returning the content as a string object
			 *
			 * @param the absolute or relative path to the file including the filename itself
			 *
			 * @return the string containing the files content
			 * TODO add FileNotFound Exception support
			 */
			std::string readKernel(const char* filename);

		public:
			OpenCLProgram() throw(std::logic_error);
			~OpenCLProgram();

			/**
			 * loads an external cl-file and creates an openCL program out of that
			 *
			 * @param the absolute or relative path to the file including the filename itself
			 */
			void loadProgram(const char* filename, const char* kernelName) throw(std::logic_error);

			/**
			 * returns the number of arguments the loaded kernel owns
			 */
			unsigned char getNBKernelArgs() throw(std::logic_error);

			/**
			 * set an argument of the kernel
			 *
			 * @param index the index of the argument in the list of kernel parameters. Must be within [0,getNBKernelArgs-1].
			 * @param size the number of bytes that have to be allocated for the argument value
			 * @param argumentValue the memmory object that is supposed to be the kernels argument
			 */
			template <typename T>
			void setKernelArgument(const unsigned char index, const size_t size, T& argumentValue) throw(std::logic_error);
	};
}

#endif /* OPENCLPROGRAM_H_ */
