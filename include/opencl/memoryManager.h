/*
 * memoryManager.h
 *
 *  Created on: Jan 20, 2010
 *      Author: lars
 */

#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include "types.h"

#include <CL/cl.h>
#include <string>

namespace GRAVID{

	class MemoryManager{
	private:
		// the command queue
		cl_command_queue cmdQ;
		// specifies the image format of the frame
		cl_image_format imgFmt;
		// pinned input image on the host side
		cl_mem srcImage2D_host;
		// pinned output image on the host side
		cl_mem dstImage2D_host;
		// device input image
		cl_mem srcImage2D_dev;
		// device output image
		cl_mem dstImage2D_dev;
		// pointer to the raw pinned input host data
		RGBA* srcImage_mapped;
		// pointer to the raw pinned output host data
		RGBA* dstImage_mapped;
		// origin and region sample of the image
		size_t origin[3];
		size_t region[3];

		// events to keep track of asynchronous copies
		cl_event copyToDevice;
		cl_event copyFromDevice;

		// the error message contained in an exception, that is thrown whenever an error occurs
		std::string errorMsg;
		// contains the OpenCL error code
		cl_int errorCode;

		/**
		 * takes a C-string as error message an throws a std::logic_error excetion including it
		 *
		 * @param error contains the error message that can be read from the exception
		 */
		void errorHappened(const char* error);

	public:
		MemoryManager(cl_context clCtx, cl_command_queue cmdQ, VideoInfo vidInf);
		~MemoryManager();

		/**
		 * returns the pinned input image on the host
		 *
		 * @return the pinned input image on the host
		 */
		RGBA* getInputFrame(){return this->srcImage_mapped;}

		/**
		 * returns the pinned output image on the host
		 *
		 * @return the pinned output image on the host
		 */
		RGBA* getOutputFrame(){return this->dstImage_mapped;}

		/**
		 * returns the device's input image to be used as a kernel argument
		 *
		 * @return the device's input image
		 */
		cl_mem getInImage_dev(){return this->srcImage2D_dev;}

		/**
		 * returns the device's output image to be used as a kernel argument
		 *
		 * @return the device's output image
		 */
		cl_mem getOutImage_dev(){return this->dstImage2D_dev;}

		/**
		 * synchronizes the input frame on the host with the input image on the device in an
		 * asynchronous copy.
		 *
		 * @return this event can be used to determine when the copy process is finished
		 */
		cl_event updateInputFrame();

		/**
		 * retrieves the latest frame from the device in an asynchronous copy.
		 *
		 * @param waitingFor if specified, the copy process is deferred until this event is finished.
		 * 			Ignored if NULL
		 *
		 * @return this event can be used to determine when the copy process is finished
		 */
		cl_event updateOutputFrame(const cl_event &waitingFor);
	};

}

#endif /* MEMORYMANAGER_H_ */
