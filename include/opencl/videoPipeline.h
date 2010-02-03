/*
 * videoPipeline.h
 *
 *  Created on: Feb 3, 2010
 *      Author: lars
 */

#ifndef VIDEOPIPELINE_H_
#define VIDEOPIPELINE_H_

#include "types.h"
#include "common_gravid.h"

#include <CL/cl.h>

namespace GRAVID{

	class VideoPipeline : public Common{
	private:
		// reference to the context that is necessary to create the images
		cl_context ctx;
		// reference to the command queue that is necessary to enque image writes
		cl_command_queue cmdQ;

		// fixed parameter determining the max number of elements in the fifo buffer
		unsigned int fifo_length;
		// the index of the element within the fifo that is beeing considered as the first element
		unsigned int first_index;
		// mostly for the beginning of filling the fifo: the number of images yet stored in the fifo
		unsigned int nb_imgs_in_fifo;
		/**
		 * width and height of the images beeing processed
		 */
		size_t image_width, image_height;

		/**
		 * the memory objects of the pipeline
		 */
		cl_mem hostImage2D_in;
		// the actual fifo buffer
		cl_mem devImage3D_in;

		cl_mem devImage2D_out;
		cl_mem hostImage2D_out;

		/**
		 * the pointers that map to the memory space of the images on the host
		 */
		RGBA* hostPixelData_in;
		RGBA* hostPixelData_out;

		/*
		 * the events that can be used to keep track of the copy states
		 */
		cl_event copyToDevice;
		cl_event copyFromDevice;

	public:
		/**
		 * @param ctx the OpenCL context within the pipeline is supposed to operate
		 * @param cmdQ the OpenCL command queue that holds the copy processes
		 * @param fifo_length the max number of elements in the fifo buffer structure
		 * @param image_width the width of the images beeing processed
		 * @param image_height the height of the images beeing processed
		 */
		VideoPipeline(cl_context ctx, cl_command_queue cmdQ, const unsigned int fifo_length,
						size_t image_width, size_t image_height);
		~VideoPipeline();

		/**
		 * copies the next frame to the fifo buffer
		 *
		 * @param last_kernel_launch the OpenCL event that must finish before the copy process is started
		 * 			this one is intended to be an event of a preceeding kernel launch, that requires this image
		 *
		 * @return the OpenCL event that indicates when this copy process has finished
		 */
		cl_event copyToDevice(cl_event last_kernel_launch);

		/**
		 * copies the last result frame from the result buffer on the device to the host
		 *
		 * @param last_kernel_launch the OpenCL event that must finish before the copy process is started
		 * 			this one is intended to be an event of a preceeding kernel launch, that produced this image
		 *
		 * @return the OpenCL event that indicates when this copy process has finished
		 */
		cl_event copyFromDevice(cl_event last_kernel_launch);

		/**
		 * returns the number of elements, that the fifo can take at the maximum
		 */
		unsigned int getFifoLength(){return this->fifo_length;}

		/**
		 * returns the index of the element in the fifo, that is considered to be the first element now
		 */
		unsigned int getFirstIndex(){return this->first_index;}

		/**
		 * returns the number of images, that are currently placed within the fifo buffer
		 * this number can be lower at the beginning when the fifo is not totally filled yet
		 */
		unsigned int getNBImgsInFifo(){return this->nb_imgs_in_fifo;}
	};
}

#endif /* VIDEOPIPELINE_H_ */
