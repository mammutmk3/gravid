/*
 * types.h
 *
 *  Created on: Jan 7, 2010
 *      Author: lars
 */

#ifndef TYPES_H_
#define TYPES_H_

extern "C" {
	#include <libavformat/avformat.h>
}

namespace GRAVID{
	/**
	 * this struct represents the color information for 1 pixel in an 32-bit RGBA image
	 */
	struct RGBA{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	};

	/**
	 * struct that contains valuable information about the video stream of a video file
	 */
	struct VideoInfo{
		// width and height of the single frames
		unsigned short width,height;
		// duration of the video in seconds
		float duration;
		// Pixel-format of the single frames, for expamle: RGB, YUV, ...
		PixelFormat pixelFMT;
		// the frame-Rate of the video stream
		AVRational frame_rate;
		// the total number of frames
		unsigned int nb_frames;
		// size in bytes of a single frame
		size_t byteSize;
	};
}

#endif /* TYPES_H_ */
