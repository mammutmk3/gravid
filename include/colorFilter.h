/*
 * greyFilter.h
 *
 *  Created on: Jan 13, 2010
 *      Author: lars
 */

#ifndef GREYFILTER_H_
#define GREYFILTER_H_

#include "types.h"

enum FilterType{
	GRAY_FILTER,
	SEPIA_FILTER
};

namespace GRAVID{

	class ColorFilter{
	private:

		/**
		 * applies the Sepia color-effect to a single Pixel
		 *
		 * @param srcPixel pixel of the original image
		 * @param dstPixel pixel of the filtered image
		 */
		void applySepia(const RGB& srcPixel, RGB& dstPixel);

		/**
		 * applies the humanoid gray-color effect to a single pixel
		 *
		 * @param srcPixel pixel of the original image
		 * @param dstPixel pixel of the filtered image
		 */
		void applyGray(const RGB& srcPixel, RGB& dstPixel);

	public:

		ColorFilter(){}

		/**
		 * applies an image filter to an RGB image
		 *
		 * @param image the 24-bit RGB image that is supposed to be converted
		 * @param width the width of the images in pixels
		 * @param height the height of the images in pixels
		 * @param dstImage 24-bit RGB image where the converted image is written to
		 */
		void applyFilter(const RGB* srcImage,
							const unsigned short width,
							const unsigned short height,
							RGB* dstImage,
							enum FilterType fType);

	};
}


#endif /* GREYFILTER_H_ */
