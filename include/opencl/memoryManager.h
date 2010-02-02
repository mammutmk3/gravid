/*
 * memoryManager.h
 *
 *  Created on: Jan 20, 2010
 *      Author: lars
 */

#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include "types.h"
#include "opencl/doubleBuffer.h"

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
		/*cl_mem srcImage2D_host;
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
		size_t region[3];*/
		DoubleBuffer *pHost_in;
		DoubleBuffer *pDevice_in;
		DoubleBuffer *pHost_out;
		DoubleBuffer *pDevice_out;
		cl_mem tmpImage;

		// events to keep track of asynchronous copies
		cl_event copyToDevice_event;
		cl_event copyFromDevice_event;

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
		 * retuns the address for the Decoder to write the next frame into
		 */
		RGBA* getFrame_forDecoder(){return this->pHost_in->getBackBuffer_Pointer();}
		//RGBA* getInputFrame(){return this->srcImage_mapped;}

		/**
		 * returns the image object that contains the original image in the first place
		 */
		cl_mem getImage_Original(){return this->pDevice_in->getFrontBuffer();}

		//RGBA* getOutputFrame(){return this->dstImage_mapped;}

		/**
		 * returns the image object that can be used for intermediate results
		 * in the case of multiple effects beeing applied for one frame
		 */
		cl_mem getImage_Tempory(){return this->tmpImage;}

		/**
		 * returns the image object representing the place where the last image effect in a sequence of kernels
		 * finally writes the output image to. It's beeing used as a kernel parameter by the kernel executor
		 */
		cl_mem getImage_Output(){return this->pDevice_out->getBackBuffer();}

		/**
		 * returns the pointer to the processed frame that is ready for output by OpenGL or encoding by the encoder
		 */
		RGBA* getFrame_forEncoder(){return this->pHost_out->getFrontBuffer_Pointer();}


		/**
		 * THE DECODER MUST HAVE FINISHED IT'S WORK; OTHERWISE THE RESULT IS UNDEFINED
		 * copies a single image from the input image buffer to the device
		 *
		 * @param kernelFinished event that indicates whether e preceeding kernel launch has finished yet
		 */
		void copyToDevice(cl_event kernelFinished);

		/**
		 * THE ENCODER MUST HAVE FINISHED IT'S WORK; OTHERWISE THE RESULT IS UNDEFINED
		 * copies a single image from the output image buffer of the device to the device
		 *
		 * @param kernelFinished event that indicates whether e preceeding kernel launch has finished yet
		 */
		void copyFromDevice(cl_event kernelFinished);

		cl_event getCopyToDevice_event(){return this->copyToDevice_event;}
		cl_event getCopyFromDevice_event(){return this->copyFromDevice_event;}

		//cl_mem getInImage_dev(){return this->srcImage2D_dev;}

		//cl_mem getOutImage_dev(){return this->dstImage2D_dev;}

		/**
		 * synchronizes the input frame on the host with the input image on the device in an
		 * asynchronous copy.
		 *
		 * @return this event can be used to determine when the copy process is finished
		 */
		//cl_event updateInputFrame();

		/**
		 * retrieves the latest frame from the device in an asynchronous copy.
		 *
		 * @param waitingFor if specified, the copy process is deferred until this event is finished.
		 * 			Ignored if NULL
		 *
		 * @return this event can be used to determine when the copy process is finished
		 */
		//cl_event updateOutputFrame(const cl_event &waitingFor);
	};

}

#endif /* MEMORYMANAGER_H_ */
