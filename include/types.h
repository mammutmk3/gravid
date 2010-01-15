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
	 * this struct represents the color information for 1 pixel in an 24-bit RGB image
	 */
	struct RGB{
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	/**
	 * this struct transfers necessary information from the VideoReader to the VideoWriter
	 * the last uses this information to set up its video stream
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
		// a pointer to the videos audio stream
		AVStream* pAudioStream;
		// the bit-rate for the stream
		int bit_rate;
	};
}

#endif /* TYPES_H_ */
