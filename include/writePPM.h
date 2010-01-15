/*
 * writePPM.h
 *
 *  Created on: Jan 7, 2010
 *      Author: lars
 */

#ifndef WRITEPPM_H_
#define WRITEPPM_H_

#include "types.h"

namespace GRAVID{

	/**
	 * takes an image as 24-bit RGB values per pixel
	 * and write it as a PPM file with the specified name and dimensions to the drive
	 *
	 * @param image the array of width*height
	 * @param filename path and filename, either relative to execution path or absolute, where the PPM file is written to
	 * @param width the width of the PPM image
	 * @param height the height of the PPM image
	 *
	 */
	void writePPM(const RGB* image, const char* filename, unsigned short width, unsigned short height);
}

#endif /* WRITEPPM_H_ */
