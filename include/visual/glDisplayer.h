/*
 * glOutput.h
 *
 *  Created on: Jan 22, 2010
 *      Author: lars
 */

#ifndef GLOUTPUT_H_
#define GLOUTPUT_H_

#include "opencl/kernelExecutor.h"
#include "opencl/memoryManager.h"
#include "codec/videoReader.h"

#include "types.h"

namespace GRAVID{

	void glDisplay(int argc, char** argv,
					const unsigned short windowWidth, const unsigned short windowHeight,
					RGBA* displayImage);

	void setToolChain(KernelExecutor &kExec, VideoReader &reader, MemoryManager &memMan);
}

#endif /* GLOUTPUT_H_ */
