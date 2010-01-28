/*
 * videoReader.h
 *
 *  Created on: Jan 7, 2010
 *      Author: lars
 */
#pragma once

extern "C" {
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

#include "types.h"
#include <string>

namespace GRAVID{
	/**
	 * This class represents a video that is being loaded from hard disk and provides methods to read the single frames and other characteristic data
	 */
	class VideoReader{
	private:
		// the filename to the corresponding video file
		AVFormatContext *pFormatCtx;
		// contains all the information about the found video stream
		AVCodecContext *pCodecCtx;
		// the stream index of the video stream within the video file
		unsigned char videoStreamIndex;
		// container for the actual frame in it's original format
		AVFrame *pFrame;
		// container for the same frame holding it in RGBA format
		AVFrame *pRGBAFrame;
		// the readers own container for the raw frame output
		uint8_t *pLocalBuffer;
		// points to the memory buffer that is currently beeing used for writing to images
		uint8_t *pCurrentBuffer;
		// contains the undecoded videoframe data
		AVPacket packet;
		// structure containing information for colorspace conversion
		SwsContext *pSwsCtx;

		// height and width of the video
		unsigned short width, height;
		// denotes, if there is a next frame in the stream, or not
		bool endOfFile;
		// containing an error message in case an error has happened
		std::string errorMsg;

		/**
		 * throws a std::logic_error exception containing the error message given
		 *
		 * @param error contains the error message that is packed with the thrown exception
		 */
		void errorHappened(const char* error);

		void associateCurrentBuffer();

	public:
		/**
		 * initializes the video object
		 *
		 * @param filename the path relative to the current execution directory or an absolute path to the video file
		 */
		VideoReader(const char* filename);

		~VideoReader();

		/**
		 * writes the next frame of the video stream into the current frame buffer
		 * */
		void decodeNextFrame();

		/**
		 * returns a pointer to the current framebuffer
		 * contains the last decoded image if no framebuffer changes have been made
		 */
		void* getCurrentFrameBuffer(){return (void*)this->pCurrentBuffer;}

		/**
		 * returns the videos width
		 *
		 * @return the width of a single video frame in pixels
		 */
		unsigned short getWidth(){return this->width;}

		/**
		 * returns the videos height
		 *
		 * @return the height of a single video fram ein pixels
		 */
		unsigned short getHeight(){return this->height;}

		/**
		 * reports if there is still an undecoded frame left in the stream
		 *
		 * @return true if there is a next frame to decode, false if the end of the stream has been reached
		 */
		bool hasNextFrame(){return !this->endOfFile;}

		/**
		 * tells the decoder to use its own internal frame buffer memory space
		 * this is already done by default
		 */
		void useOwnFrameBuffer();

		/**
		 * tells the decoder to use the given frameBuffer instead of the previous one
		 *
		 * @param pFrameBuffer points to
		 */
		void changeFrameBuffer(void* pFrameBuffer);

		/**
		 * returns some basic information about the loaded video
		 */
		VideoInfo getVideoInfo();
	};
}
