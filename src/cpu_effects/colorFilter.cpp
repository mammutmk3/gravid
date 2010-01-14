/*
 * grayFilter.cpp
 *
 *  Created on: Jan 13, 2010
 *      Author: lars
 */

#include "colorFilter.h"
#include "types.h"

#include <algorithm>

using namespace GRAVID;

void ColorFilter::applyFilter(const RGB* srcImage,
								const unsigned short width,
								const unsigned short height,
								RGB* dstImage,
								enum FilterType fType){
	const RGB *srcPixel;
	RGB *dstPixel;
	for(unsigned short y = 0; y < height; y++){
		for(unsigned short x = 0; x < width;x++){
			srcPixel = &(srcImage[y*width+x]);
			dstPixel = &(dstImage[y*width+x]);
			switch(fType){
				case GRAY_FILTER: this->applyGray(*srcPixel,*dstPixel);break;
				case SEPIA_FILTER: this->applySepia(*srcPixel,*dstPixel);break;
			}
		}
	}
}

void ColorFilter::applyGray(const RGB& srcPixel, RGB& dstPixel){
	unsigned char gray;
	gray = (unsigned char)(0.299 * srcPixel.r + 0.587 * srcPixel.g + 0.114 * srcPixel.b);
	dstPixel.r = dstPixel.g = dstPixel.b = gray;
}

void ColorFilter::applySepia(const RGB& srcPixel, RGB& dstPixel){
	const unsigned char sepiaIntensity = 30;
	applyGray(srcPixel,dstPixel);
	dstPixel.r = (unsigned char)std::min(255, dstPixel.r+2*sepiaIntensity);
	dstPixel.g = (unsigned char)std::min(255, dstPixel.g+sepiaIntensity);
}
