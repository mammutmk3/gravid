/*
 * VideoWriter.cpp
 *
 *  Created on: Jan 9, 2010
 *      Author: lars
 */

#include "codec/videoWriter.h"

#include <iostream>
#include <stdexcept>

using namespace GRAVID;
using std::logic_error;

#define STREAM_PIX_FMT PIX_FMT_YUV420P // default pix_fmt for the output stream
#define MUX_PRELOAD 0.5
#define MUX_MAX_DELAY 0.7

#define INT64_C

// adapt this to RGBA

VideoWriter::VideoWriter(const char* filename, const VideoInfo vInfo) throw(std::logic_error){
	// register all available codecs in mpeg
	av_register_all();

	// set the filename
	this->filename = filename;
	// set the conversion infos
	this->vInfo = vInfo;

	 // detect the needed codec from the filename extension
	 // - if a codec can't be deduced: MPEG is the default
    this->fmt = guess_format(NULL, this->filename, NULL);
    if (NULL == fmt) {
        fmt = guess_format("mpeg", NULL, NULL);
        if (NULL == fmt)
        	this->errorHappened("couldn't find a suitable output format");
    }

    // allocate the output media context
    oc = avformat_alloc_context();
    if(NULL == this->oc)
    	this->errorHappened("not enough memory for allocating the outputn media context");
    this->oc->oformat = this->fmt; // set the output format that has been determined
    // safely set the filename
    snprintf(this->oc->filename, sizeof(this->oc->filename), "%s", this->filename);

    // add the video stream using the default format codec and initialize the codec
    this->video_st = NULL;
    if (this->fmt->video_codec != CODEC_ID_NONE) {
        this->video_st = this->add_video_stream(this->fmt->video_codec);
    }

    // set the output parameters (must be done even if no parameters)
    if (av_set_parameters(this->oc, NULL) < 0)
    	this->errorHappened("Invalid output format parameters");

    this->oc->preload = (int)(MUX_PRELOAD*AV_TIME_BASE);
    this->oc->max_delay = (int)(MUX_MAX_DELAY*AV_TIME_BASE);

    // now that all the parameters are set, we can open the audio and
    // video codecs and allocate the necessary encode buffers
    if (NULL != this->video_st)
        this->openVideo();

    // open the output file, if needed
    if (!(this->fmt->flags & AVFMT_NOFILE)) {
        if (url_fopen(&(this->oc->pb), this->filename, URL_WRONLY) < 0)
        	this->errorHappened("couldn't open output file");
    }

    // write the stream header, if any
	av_write_header(this->oc);

	// set the current video frame pointer to the first frame to encode
	this->frame_count = 0;
}

VideoWriter::~VideoWriter(){
    // free the video context
    if (NULL != this->video_st){
    	avcodec_close(this->video_st->codec);
    	if(picture){
    		av_free(this->picture->data[0]);
    		av_free(this->picture);
    	}
		if (this->tmp_picture) {
			av_free(this->tmp_picture->data[0]);
			av_free(this->tmp_picture);
		}
		if(this->video_outbuf)
			av_free(this->video_outbuf);
    }

    // free the streams
    for(unsigned int i = 0; i < this->oc->nb_streams; i++) {
    	if(this->oc->streams[i]){
    		av_freep(&this->oc->streams[i]->codec);
        	av_freep(&this->oc->streams[i]);
    	}
    }

    if (!(this->fmt->flags & AVFMT_NOFILE) && this->oc->pb != NULL) {
        // close the output file
        url_fclose(this->oc->pb);
    }

    // free the stream
    if(NULL != this->oc)
    	av_free(this->oc);
}

void VideoWriter::errorHappened(const char* error) throw(std::logic_error){
	this->errorMsg = error;
	throw logic_error(this->errorMsg);
}

AVStream* VideoWriter::add_video_stream(enum CodecID codec_id){
    AVCodecContext *c;
    AVStream *st;

	// allocate a new stream
    st = av_new_stream(this->oc, 0);
    if (NULL == st)
    	this->errorHappened("couldn't allocate new video stream");

    c = st->codec;
    c->codec_id = (CodecID)codec_id;
    c->codec_type = CODEC_TYPE_VIDEO;

    // put sample parameters
    c->bit_rate = 10000000;
    // resolution must be a multiple of two
    c->width = this->vInfo.width;
    c->height = this->vInfo.height;

    // set the frame rate
    c->time_base.num = this->vInfo.frame_rate.den;
    c->time_base.den = this->vInfo.frame_rate.num;
    c->gop_size = 12; // emit one intra frame every twelve frames at most
    c->pix_fmt = STREAM_PIX_FMT;

    // add b-frames if codec allows it
    if (c->codec_id == CODEC_ID_MPEG2VIDEO)
        c->max_b_frames = 2;

    // some hack to avoid makroblocks
    if (c->codec_id == CODEC_ID_MPEG1VIDEO)
        c->mb_decision=2;

    // some formats want stream headers to be separate
    if(this->oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

void VideoWriter::openVideo(){
    AVCodec *codec;
    AVCodecContext *c;

    c = this->video_st->codec;

    // find the video encoder
    codec = avcodec_find_encoder(c->codec_id);
    if (NULL == codec)
    	this->errorHappened("codec not found");

    // open the codec
    if (avcodec_open(c, codec) < 0)
    	this->errorHappened("could not open codec");

    this->video_outbuf = NULL;
    if (!(this->oc->oformat->flags & AVFMT_RAWPICTURE)) {
        // allocate output buffer - 6.5 MB in size
        this->video_outbuf_size = 6500000;
        this->video_outbuf = (uint8_t*)av_malloc(this->video_outbuf_size);
    }

    // allocate the encoded raw picture
    this->picture = alloc_picture(c->pix_fmt);
    if (NULL == this->picture)
    	this->errorHappened("could not allocate picture");

    /* because the input-image format is not YUV420P, a temporary YUV420P
       picture is needed too. It is then converted to the required
       output format */
    tmp_picture = NULL;
	tmp_picture = alloc_picture(PIX_FMT_BGR32);
	if (NULL == tmp_picture)
		this->errorHappened("couldn't allocate temporary picture");

}

AVFrame* VideoWriter::alloc_picture(enum PixelFormat pix_fmt){
    AVFrame *picture;
    uint8_t *picture_buf;
    int size;

    picture = avcodec_alloc_frame();
    if (NULL == picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, this->vInfo.width, this->vInfo.height);
    picture_buf = (uint8_t*)av_malloc(size);
    if (NULL == picture_buf) {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf,
                   pix_fmt, this->vInfo.width, this->vInfo.height);
    return picture;
}

void VideoWriter::writeMultiMedFrame(RGBA* frame){

    if(this->video_pts < this->vInfo.duration) {
        if (NULL != this->video_st)
            this->video_pts = (double)video_st->pts.val * this->video_st->time_base.num / this->video_st->time_base.den;

        this->write_video_frame(frame);
    }
}

void VideoWriter::write_video_frame(RGBA* frame) throw (std::logic_error){
    int out_size, ret;
    AVCodecContext *c;
    static struct SwsContext *img_convert_ctx;

	c = this->video_st->codec;

	// always true for us, as we convert RGB to YUV
	if (PIX_FMT_BGR32 != STREAM_PIX_FMT) {
		if (img_convert_ctx == NULL) {
			img_convert_ctx = sws_getContext(c->width, c->height,
											 PIX_FMT_BGR32,
											 c->width, c->height,
											 STREAM_PIX_FMT,
											 SWS_BICUBIC, NULL, NULL, NULL);
			if (img_convert_ctx == NULL)
				this->errorHappened("Cannot initialize the conversion context");
		}
		// set the picture data to the rgb byte field
		this->tmp_picture->data[0] = (uint8_t*)frame;
		// convert the image
		sws_scale(img_convert_ctx, tmp_picture->data, tmp_picture->linesize,
				  0, c->height, picture->data, picture->linesize);
	}

	// encode the image
	out_size = avcodec_encode_video(c, this->video_outbuf, this->video_outbuf_size, this->picture);
	// if zero size, it means the image was buffered
	if (out_size > 0) {
		AVPacket pkt;
		av_init_packet(&pkt);

		if (c->coded_frame->pts != AV_NOPTS_VALUE)
			pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, this->video_st->time_base);
		if(c->coded_frame->key_frame)
			pkt.flags |= PKT_FLAG_KEY;
		pkt.stream_index= this->video_st->index;
		pkt.data= video_outbuf;
		pkt.size= out_size;

		// write the compressed frame in the media file
		ret = av_interleaved_write_frame(oc, &pkt);
	} else {
		ret = 0;
	}

    if (ret != 0)
    	this->errorHappened("Error while writing video frame");
}

void VideoWriter::finalizeVideo(){
	// write the end of the video file
	if(0 != av_write_trailer(this->oc))
		this->errorHappened("couldn't write video trailer");
}
