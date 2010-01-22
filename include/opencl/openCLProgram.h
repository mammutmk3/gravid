/*
 * openCLProgram.h
 *
 *  Created on: Jan 13, 2010
 *      Author: lars
 */

#ifndef OPENCLPROGRAM_H_
#define OPENCLPROGRAM_H_

#include <CL/cl.h>
#include <string>

#include "types.h"

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
			// the command queue
			cl_command_queue cmdQ;

			// this string get's encapsulated into an exception that is beeing thrown, whenever somtehing unexpected happens
			std::string errorMsg;

			/**
			 * takes a C-string as error message an throws a std::logic_error excetion including it
			 *
			 * @param error contains the error message that can be read from the exception
			 */
			void errorHappened(const char* error);

			/**
			 * reads a file containg an OpenCL program and returning the content as a string object
			 *
			 * @param the absolute or relative path to the file including the filename itself
			 *
			 * @return the string containing the files content
			 */
			std::string readProgram(const char* filename);

			/**
			 * loads an external cl-file and creates an openCL program out of that
			 *
			 * @param the absolute or relative path to the file including the filename itself
			 */
			void loadProgram(const char* filename);

		public:
			OpenCLProgram(const char* filename);
			~OpenCLProgram();

			/**
			 * returns the OpenCL context
			 *
			 * @return the OpenCL context
			 */
			cl_context getContext(){return this->clCtx;}

			/**
			 * returns the command queue
			 *
			 * @return the command queue
			 */
			cl_command_queue getCommandQueue(){return this->cmdQ;}

			/**
			 * returns the created cl program
			 *
			 * @return the cl program created from the specified file
			 */
			cl_program getProgram(){return this->clProgram;}
	};
}

#endif /* OPENCLPROGRAM_H_ */
