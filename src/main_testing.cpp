/*
 * main.cpp
 *
 *  Created on: Jan 6, 2010
 *      Author: mk3
 */

#include "cmdLineParser.h"
#include "threads.h"

#include "codec/videoReader.h"
#include "codec/videoWriter.h"

#include "opencl/openCLProgram.h"
#include "opencl/memoryManager.h"
#include "opencl/kernelExecutor.h"
#include "cpu_effects/FadingEffects.h"
#include "cpu_effects/edgeDetection.h"
#include "cpu_effects/echoEffect.h"
#include "cpu_effects/colorFilter.h"
#include "visual/writePPM.h"

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <sys/time.h>

#include <pthread.h>

using namespace GRAVID;

int main(int argc, char** argv){
		// create the decoder for a video file
		VideoReader reader1("videos/fadeVid1.mpg");
		VideoInfo vidInf = reader1.getVideoInfo();
		
		VideoReader reader2("videos/fadeVid2.mpg");
		VideoInfo vidInf2 = reader2.getVideoInfo();

		RGBA* frameVid1 = (RGBA*)malloc( vidInf.height * vidInf.width * sizeof( RGBA ) );
		RGBA* frameVid2 = (RGBA*)malloc( vidInf2.height * vidInf2.width * sizeof( RGBA ) );
		RGBA* output = (RGBA*)malloc( vidInf.height * vidInf.width * sizeof( RGBA ) );

		// constraints for the to videos
		if ( (vidInf.height != vidInf2.height) && ( vidInf.width != vidInf2.width) ) {
			std::cout << "videos must have the same aspect ratio" << std::endl;
			//throw logic_error("videos must have the same aspect ratio");
		}
		if ( ( vidInf.frame_rate.den / vidInf.frame_rate.num ) != ( vidInf2.frame_rate.den / vidInf2.frame_rate.num ) ) {
			//throw logic_error("videos must have the same frame rate");
			std::cout << "videos must have the same frame rate" << std::endl;
		}
		if ( vidInf.duration != vidInf2.duration) {
			//throw logic_error("video1 must be longer than the second");
			std::cout << "video1 must be longer than the second" << std::endl;
		}

		char filename[100];
		unsigned int i = 0;
		
		EdgeDetection edgeEff;
		FadingEffects fadingEff;
		EchoEffect echEff;
		ColorFilter cfilter;
		
		
		size_t pic_size = vidInf.width * vidInf.height * sizeof( RGBA);		
		std::vector<RGBA*> frames;
		
		float sum=0;
		
		// for the  echo-effect
		// srand ( time(NULL) );
		
		VideoWriter vidWriter( "videos/out1.mpg", vidInf );

		while ( reader1.hasNextFrame() ) {
			reader1.changeFrameBuffer( frameVid1 );
			reader1.decodeNextFrame();

			//only for fading
			reader2.changeFrameBuffer( frameVid2 );
			reader2.decodeNextFrame();

			timeval start;
			gettimeofday( &start, NULL);

			//fadingEff.fadeBlind( frameVid1, frameVid2, output, vidInf2.width, vidInf2.height, ((float)i/(float)vidInf.nb_frames) );
			//fadingEff.fadeAdditive( frameVid1, frameVid2, output, vidInf2.width, vidInf2.height, ((float)i/(float)vidInf.nb_frames) );
			//edgeEff.sobelOperator( frameVid1, output, vidInf.width, vidInf.height );
			//cfilter.applyFilter( frameVid1, vidInf.width, vidInf.height, output, C_SEPIA_FILTER );
			
			// für den echo-Effekt, CPU-Version 

			int frame_cnt = i+1;
			RGBA* tmp_frame = (RGBA*)malloc( pic_size );
			memcpy( tmp_frame, frameVid1, pic_size);
			
			
			frames.push_back( tmp_frame );
			if ( i > 4 ) {
				frames.erase(frames.begin());
				frame_cnt = 5;
			}
			
			//std::cout << " frame-nr: " << i << std::endl;
			
			echEff.renderEcho( frames, output, vidInf.width, vidInf.height, i, frame_cnt );
			// write the image to output
		
			timeval end;
			gettimeofday( &end, NULL);
			
			sum += ((end.tv_usec + end.tv_sec * 1000000) - (start.tv_usec + start.tv_sec * 1000000))/1000;
			
			
			sprintf(filename,"pictures/pic%i.ppm",i);
			
			vidWriter.writeMultiMedFrame((RGBA*) output);

			//writePPM( output, filename, vidInf.width, vidInf.height);
			i++;
		}
		std::cout << "avg time for video_effect: " << sum/i  << " ms"<< std::endl;
		vidWriter.finalizeVideo();
}
