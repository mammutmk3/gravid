/*
 * videoReader.h
 *
 *  Created on: Jan 7, 2010
 *      Author: lars
 */

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

#include "exceptions.h"
#include "types.h"

namespace GRAVID{
	/**
	 * This class represents a video that is being loaded from hard disk and provides methods to read the single frames and other characteristic data
	 */
	class VideoReader{
	private:
		// the filename to the corresponding video file
		char* filename;
		AVFormatContext *pFormatCtx;
		// contains all the information about the found video stream
		AVCodecContext *pCodecCtx;
		// the stream index of the video stream within the video file
		unsigned char videoStreamIndex;
		// container for the actual frame in it's original format
		AVFrame *pFrame;
		// container for the same frame holding it in RGBA format
		AVFrame *pRGBFrame;
		// container for the raw frame data
		uint8_t *buffer;
		// height and width of the video
		unsigned short width, height;
		// the position of the next frame that will be decoded
		unsigned int nextPos;
		// denotes, if there is a next frame in the stream, or not
		bool endOfFile;

	public:
		/**
		 * initializes the video object
		 *
		 * @param filename the path relative to the current execution directory or an absolute path to the video file of interest
		 */
		VideoReader(const char* filename) throw(FileNotFound, std::logic_error);

		~VideoReader();

		/**
		 * Returns the next frame in the video stream
		 *
		 * @return field of rgb values of size getWidht()*getHeight(); the field ist stored in consecutive rows. NULL if the end of the stream has been reached
		 * */
		RGB* getNextFrame() throw(std::logic_error);

		/**
		 * Get the videos width
		 */
		unsigned short getWidth(){return this->width;}

		/**
		 * Get the videos height
		 */
		unsigned short getHeight(){return this->height;}

		/**
		 * reports if there is still an undecoded frame left in the stream
		 *
		 * @return true if there is a next frame to decode, false if the end of the stream has been reached
		 */
		bool hasNextFrame(){return !this->endOfFile;}

		/**
		 * returns some basic information like bit-rate and frame-rate needed for video output in VideoWriter class
		 */
		VideoInfo getVideoInfo();
	};
}
