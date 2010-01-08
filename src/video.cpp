/*
 * video.cpp
 *
 *  Created on: Jan 7, 2010
 *      Author: lars
 */

#include "video.h"
#include "exceptions.h"

#include <string>
extern "C"{
	#include <libswscale/swscale.h>
}

#include <iostream>

using namespace GRAVID;

Video::Video(const char* filename) throw (FileNotFound, std::logic_error){

	// initialize the members
	this->pFormatCtx = NULL;
	this->pCodecCtx = NULL;
	this->pFrame = NULL;
	this->pRGBFrame = NULL;
	this->buffer = NULL;
	this->videoStreamIndex = -1;
	this->nextPos = 0;
	this->endOfFile = false;

	// open the video file
	if(av_open_input_file(&(this->pFormatCtx), filename, NULL, 0, NULL)!=0){
		std::string error = "File \"";
		error.append(filename);
		error.append("\" not found");
		throw FileNotFound(error);
	}

	// retrieve stream information
	if(av_find_stream_info(this->pFormatCtx)<0){
		std::string error = "couldn't find stream information";
		throw std::logic_error(error);
	}

	// Find the first video stream
	for(unsigned char i=0; i<this->pFormatCtx->nb_streams; i++){
	  if(this->pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
	    this->videoStreamIndex=i;
	    break;
	  }
	}
	if(-1 == this->videoStreamIndex){
		std::string error = "could't find any video stream";
		throw std::logic_error(error);
	}
	else{
		this->pCodecCtx=this->pFormatCtx->streams[this->videoStreamIndex]->codec;
		// set width and height as well
		this->width = this->pCodecCtx->width;
		this->height = this->pCodecCtx->height;
	}

	// get the actual codec that is used by the found video stream
	AVCodec *pCodec = NULL;
	pCodec = avcodec_find_decoder(this->pCodecCtx->codec_id);
	if(NULL == pCodec){
		std::string error = "unsupported codec";
		throw std::logic_error(error);
	}
	if(avcodec_open(this->pCodecCtx, pCodec) < 0){
		std::string error = "couldn't open codec";
		throw std::logic_error(error);
	}

	// allocate space for the original video frame
	this->pFrame = avcodec_alloc_frame();
	// and some for the RGBA variant of the frame above
	this->pRGBFrame = avcodec_alloc_frame();
	if(NULL == this->pFrame || NULL == this->pRGBFrame){
		std::string error = "couldn't allocate frames";
		throw std::logic_error(error);
	}

	// determine the required buffer size for an RGB image
	int numBytes = avpicture_get_size(PIX_FMT_RGB24,this->pCodecCtx->width, this->pCodecCtx->height);
	// allocate the buffer
	buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	// associate the buffer to the RGB image
	avpicture_fill((AVPicture *)this->pRGBFrame, this->buffer, PIX_FMT_RGB24, this->pCodecCtx->width, this->pCodecCtx->height);
}

Video::~Video(){
	// free the raw data buffer
	if(NULL != this->buffer)
		av_free(this->buffer);

	// free the frames
	if(NULL != this->pFrame)
		av_free(this->pFrame);
	if(NULL != this->pRGBFrame )
		av_free(this->pRGBFrame);

	// close the codec-context
	if(NULL != this->pCodecCtx)
		avcodec_close(this->pCodecCtx);

	// close the loaded video file
	if(NULL != this->pFormatCtx)
		av_close_input_file(this->pFormatCtx);
}

rgb* Video::getNextFrame()  throw(std::logic_error){

	// return NULL if the end of the video has been reached
	if(!this->hasNextFrame())
		return NULL;

	// read the video file packet for packet;
	// initialized with zero as to state, that no frame has yet been decompressed
	int frameFinished = 0;
	AVPacket packet;
	int endOfFileIndicator;
	// only loop until the next frame or the end of the video stream has been reached
	while(0 == (endOfFileIndicator = av_read_frame(this->pFormatCtx, &packet)) && frameFinished == 0){
		// check if the stream packet belongs to the video stream of the file
		if(packet.stream_index == this->videoStreamIndex){
			// decode the frame
			avcodec_decode_video(this->pCodecCtx, this->pFrame, &frameFinished, packet.data, packet.size);
		}
	}
	// set the member if the end of the stream has been reached
	if(0 > endOfFileIndicator)
		this->endOfFile = true;

	// prepare the conversion
	SwsContext *pSwsCtx;
	pSwsCtx = sws_getContext(this->pCodecCtx->width,
								this->pCodecCtx->height,
								this->pCodecCtx->pix_fmt,
								this->pCodecCtx->width,
								this->pCodecCtx->height,
								PIX_FMT_RGB24,
								SWS_BILINEAR,
								NULL, NULL, NULL);
	if(NULL == pSwsCtx){
		std::string error = "couldn't initialize the conversion context";
		throw std::logic_error(error);
	}

	// convert the frame from YUV to RGB
	sws_scale(pSwsCtx,
				this->pFrame->data,
				this->pFrame->linesize,
				0,
				this->pCodecCtx->height,
				this->pRGBFrame->data,
				this->pRGBFrame->linesize);
	// free the requested packet
	av_free_packet(&packet);
	// return the frame as an RGB representation
	return (rgb* )this->pRGBFrame->data[0];
}
