/*
 * threads.h
 *
 *  Created on: Feb 1, 2010
 *      Author: lars
 */

#ifndef THREADS_H_
#define THREADS_H_

#include "codec/videoReader.h"
#include "codec/videoWriter.h"
#include "opencl/kernelExecutor.h"
#include "opencl/memoryManager.h"

namespace GRAVID{

	void initThreadVariables(VideoReader *pReader_local, VideoWriter *pWriter_local,
								MemoryManager *pMemMan_local, KernelExecutor *pKExec_local,
								unsigned short width, unsigned short height);

	/**
	 * thread function, that handles the input side of the pipeline
	 *
	 * @param RGBA_Frame the address of the memory where to write the frame into
	 */
	void *decode_Frame(void* RGBA_Frame);

	/**
	 * thread function, that handles the output side of the pipeline
	 *
	 * @param RGBA_Frame the address of the memory from where to read the frame
	 */
	void *encode_Frame(void* RGBA_Frame);

	/**
	 * thread function, that handles the output side of the pipeline
	 *
	 * @param RGBA_Frame the address of the memory from where to read the frame
	 */
	void* display_Frame(void* RGBA_Frame);
}

#endif /* THREADS_H_ */
