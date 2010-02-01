/*
 * doubleBuffer.h
 *
 *  Created on: Jan 30, 2010
 *      Author: lars
 */

#ifndef DOUBLEBUFFER_H_
#define DOUBLEBUFFER_H_

#include "types.h"

#include <CL/cl.h>
#include <string>

namespace GRAVID{

	enum DB_Type{
		DB_HOST,
		DB_DEVICE
	};

	class DoubleBuffer{
	private:
		// the image objects
		cl_mem backbuffer;
		cl_mem frontbuffer;
		enum DB_Type db_type;

		// the mapped pointers to the image objects
		RGBA* pBackbuffer;
		RGBA* pFrontbuffer;

		// for error handling
		cl_int errorCode;
		std::string errorMsg;

		// references to the used OpenCL objects
		cl_context ctx;
		cl_command_queue cmdQ;
		cl_image_format imgFmt;

		// image properties
		unsigned short width, height;
		size_t origin[3], region[3];

		void errorHappened(const char* error);
	public:
		DoubleBuffer(cl_context ctx, cl_command_queue cmdQ,
						const VideoInfo vidInf,
						enum DB_Type db_type);
		~DoubleBuffer();

		/**
		 * swaps between the current front- and backbuffer
		 */
		void swap();

		cl_mem getBackBuffer(){return this->backbuffer;}
		cl_mem getFrontBuffer(){return this->frontbuffer;}

		RGBA* getFrontBuffer_Pointer(){return this->pFrontbuffer;}
		RGBA* getBackBuffer_Pointer(){return this->pBackbuffer;}

		size_t* getOrigin(){return this->origin;}
		size_t* getRegion(){return this->region;}
	};
}

#endif /* DOUBLEBUFFER_H_ */
