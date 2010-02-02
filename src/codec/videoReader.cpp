/*
 * videoReader.cpp
 *
 *  Created on: Jan 7, 2010
 *      Author: lars
 */

#include "codec/videoReader.h"
#include "types.h"

extern "C"{
	#include <libswscale/swscale.h>
	#include <libavformat/avformat.h>
}

#include <stdexcept>

using namespace GRAVID;

VideoReader::VideoReader(const char* filename){
	// register all available codecs in mpeg
	av_register_all();

	// initialize the members
	this->pFormatCtx = NULL;
	this->pCodecCtx = NULL;
	this->pFrame = NULL;
	this->pRGBAFrame = NULL;
	this->pLocalBuffer = NULL;
	this->pSwsCtx = NULL;
	this->videoStreamIndex = -1;
	this->endOfFile = false;

	// open the video file
	if(av_open_input_file(&(this->pFormatCtx), filename, NULL, 0, NULL)!=0)
		this->errorHappened("video file not found");

	// retrieve stream information
	if(av_find_stream_info(this->pFormatCtx)<0)
		this->errorHappened("couldn't find stream information");

	// Find the first video stream
	for(unsigned char i=0; i<this->pFormatCtx->nb_streams; i++){
	  if(this->pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
	    this->videoStreamIndex=i;
	    break;
	  }
	}
	if(-1 == this->videoStreamIndex)
		this->errorHappened("the video file doesn't contain any video stream");
	else{
		this->pCodecCtx=this->pFormatCtx->streams[this->videoStreamIndex]->codec;
		// set width and height as well
		this->width = this->pCodecCtx->width;
		this->height = this->pCodecCtx->height;
	}

	// find a decoder for the codec that is used by the found video stream
	AVCodec *pCodec = NULL;
	pCodec = avcodec_find_decoder(this->pCodecCtx->codec_id);
	if(NULL == pCodec)
		this->errorHappened("this video contains an unsupported codec");
	if(avcodec_open(this->pCodecCtx, pCodec) < 0)
		this->errorHappened("codec could not be opened");

	// allocate space for the original video frame
	this->pFrame = avcodec_alloc_frame();
	// and some for the RGBA variant of the frame above
	this->pRGBAFrame = avcodec_alloc_frame();
	if(NULL == this->pFrame || NULL == this->pRGBAFrame)
		this->errorHappened("not enough memory to allocate frames");

	// determine the required buffer size for an RGBA image
	int numBytes = avpicture_get_size(PIX_FMT_BGR32,this->pCodecCtx->width, this->pCodecCtx->height);
	// allocate the buffer
	pLocalBuffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	// use the local buffer by default
	this->useOwnFrameBuffer();

	// prepare color space conversion context
	this->pSwsCtx = sws_getContext(this->pCodecCtx->width, this->pCodecCtx->height,
									this->pCodecCtx->pix_fmt, this->pCodecCtx->width,
									this->pCodecCtx->height, PIX_FMT_BGR32,
									SWS_BILINEAR, NULL, NULL, NULL);
	if(NULL == this->pSwsCtx)
		this->errorHappened("error while initializing sws-context");
}

void VideoReader::errorHappened(const char* error){
	this->errorMsg = error;
	throw std::logic_error(this->errorMsg);
}

VideoReader::~VideoReader(){
	// free the sws-context
	if(NULL != this->pSwsCtx)
		sws_freeContext(this->pSwsCtx);

	// free the packet
	av_free_packet(&this->packet);

	// free the raw data buffer
	if(NULL != this->pLocalBuffer)
		av_free(this->pLocalBuffer);

	// free the frames
	if(NULL != this->pFrame)
		av_free(this->pFrame);
	if(NULL != this->pRGBAFrame )
		av_free(this->pRGBAFrame);

	// close the codec-context
	if(NULL != this->pCodecCtx)
		avcodec_close(this->pCodecCtx);

	// close the loaded video file
	if(NULL != this->pFormatCtx)
		av_close_input_file(this->pFormatCtx);
}

void VideoReader::associateCurrentBuffer(){
	// associate the buffer to the RGBA image
	avpicture_fill((AVPicture *)this->pRGBAFrame, this->pCurrentBuffer, PIX_FMT_BGR32,
					this->pCodecCtx->width, this->pCodecCtx->height);
}

void VideoReader::useOwnFrameBuffer(){
	this->pCurrentBuffer = this->pLocalBuffer;
	this->associateCurrentBuffer();
}

void VideoReader::changeFrameBuffer(void* pFrameBuffer){
	this->pCurrentBuffer = (uint8_t*) pFrameBuffer;
	this->associateCurrentBuffer();
}

void VideoReader::decodeNextFrame(){

	// the end of the video has been reached
	if(!this->hasNextFrame())
		this->errorHappened("end of video stream reached");

	// read the video file packet for packet;
	// initialized with zero as to state, that no frame has yet been decompressed
	int frameFinished = 0;
	int endOfFileIndicator;
	// only loop until the next frame or the end of the video stream has been reached
	while(0 == (endOfFileIndicator = av_read_frame(this->pFormatCtx, &this->packet)) && frameFinished == 0){
		// check if the stream packet belongs to the video stream of the file
		if(this->packet.stream_index == this->videoStreamIndex){
			// decode the frame
			avcodec_decode_video(this->pCodecCtx, this->pFrame, &frameFinished,
									this->packet.data, this->packet.size);
		}
	}
	// set the member if the last frame has been decoded
	if(0 > endOfFileIndicator)
		this->endOfFile = true;

	// convert the frame from YUV to RGB
	sws_scale(this->pSwsCtx,
				this->pFrame->data,
				this->pFrame->linesize,
				0,
				this->pCodecCtx->height,
				this->pRGBAFrame->data,
				this->pRGBAFrame->linesize);
}

VideoInfo VideoReader::getVideoInfo(){
	// the video stream of the file
	AVStream* v_st = this->pFormatCtx->streams[this->videoStreamIndex];

	VideoInfo tmp;

	// set height, width and channel format
	tmp.width = this->width;
	tmp.height = this->height;
	tmp.pixelFMT = PIX_FMT_BGR32;

	// set the total frame number
	tmp.nb_frames = v_st->duration;
	// set AVRational framerate
	tmp.frame_rate = v_st->r_frame_rate;
	// calculate the frame rate
	float frame_rate = v_st->r_frame_rate.num/(float)v_st->r_frame_rate.den;
	// duration in seconds
	tmp.duration = tmp.nb_frames / frame_rate;

	// set the size in bytes for a single frame of this video
	tmp.byteSize = tmp.width * tmp.height * sizeof(RGBA);

	return tmp;
}
