/*
 * VideoWriter.h
 *
 *  Created on: Jan 9, 2010
 *      Author: lars
 */

#ifndef VIDEOWRITER_H_
#define VIDEOWRITER_H_

extern "C" {
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

#include "types.h"

#include <stdexcept>
#include <string>

namespace GRAVID{

	/**
	 * takes a video and audio stream and encodes into a video file
	 */
	class VideoWriter{
	private:
		// members for audio output
		/*float t, tincr, tincr2;
		int16_t *samples;
		uint8_t *audio_outbuf;
		int audio_outbuf_size;
		int audio_input_frame_size;*/

		// members for video output
		AVFrame *picture, *tmp_picture;
		uint8_t *video_outbuf;
		unsigned int frame_count, video_outbuf_size;


		// general members
	    const char *filename;
	    AVOutputFormat *fmt;
	    AVFormatContext *oc;
	    AVStream /**audio_st,*/ *video_st;
	    double /*audio_pts,*/ video_pts;

	    std::string errorMsg; // used for setting exception message
	    VideoInfo vInfo; // contains information about the original video

	    /**
	     * takes a C-string as error message an throws a std::logic_error excetion including it
	     *
	     * @param error contains the error message that can by the exception
	     */
	    void errorHappened(const char* error) throw(std::logic_error);

	    /**
	     * initializes the video context
	     *
	     * @param codec_id identifies the codec to use
	     *
	     * @return the created video stream
	     */
	    AVStream *add_video_stream(enum CodecID codec_id);

		/**
		 * allocates the neccesarry buffer and initializes the encoders vor the video
		 */
		void openVideo();

		/**
		 * allocates a picture to work on in
		 */
		AVFrame *alloc_picture(enum PixelFormat pix_fmt);

		/**
		 * writes one single frame to the video stream
		 */
		void write_video_frame(RGBA* frame) throw (std::logic_error);

	public:
		/* constructor */
		VideoWriter(const char* filename, const VideoInfo vInfo) throw(std::logic_error);

		/* destructor */
		~VideoWriter();

		/**
		 * writes the wanted video
		 */
		void writeMultiMedFrame(RGBA* frame);

		/**
		 * writes the trailer of the video file and finalizes it with that action
		 */
		void finalizeVideo();

	};
}
#endif /* VIDEOWRITER_H_ */
