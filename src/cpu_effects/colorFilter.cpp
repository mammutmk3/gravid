/*
 * grayFilter.cpp
 *
 *  Created on: Jan 13, 2010
 *      Author: lars
 */

#include "cpu_effects/colorFilter.h"
#include "types.h"

#include <algorithm>

using namespace GRAVID;

void ColorFilter::applyFilter(const RGBA* srcImage,
								const unsigned short width,
								const unsigned short height,
								RGBA* dstImage,
								enum FilterType fType){
	const RGBA *srcPixel;
	RGBA *dstPixel;
	for(unsigned short y = 0; y < height; y++){
		for(unsigned short x = 0; x < width;x++){
			srcPixel = &(srcImage[y*width+x]);
			dstPixel = &(dstImage[y*width+x]);
			switch(fType){
				case C_GRAY_FILTER: this->applyGray(*srcPixel,*dstPixel);break;
				case C_SEPIA_FILTER: this->applySepia(*srcPixel,*dstPixel);break;
			}
		}
	}
}

void ColorFilter::applyGray(const RGBA& srcPixel, RGBA& dstPixel){
	unsigned char gray;
	gray = (unsigned char)(0.299 * srcPixel.r + 0.587 * srcPixel.g + 0.114 * srcPixel.b);
	dstPixel.r = dstPixel.g = dstPixel.b = gray;
}

void ColorFilter::applySepia(const RGBA& srcPixel, RGBA& dstPixel){
	const unsigned char sepiaIntensity = 30;
	applyGray(srcPixel,dstPixel);
	dstPixel.r = (unsigned char)std::min(255, dstPixel.r+2*sepiaIntensity);
	dstPixel.g = (unsigned char)std::min(255, dstPixel.g+sepiaIntensity);
}
