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
#include "visual/writePPM.h"

#include <iostream>
#include <cstdlib>
#include <stdexcept>

#include <pthread.h>

using namespace GRAVID;

int main(int argc, char** argv){


		// create the decoder for a video file
		VideoReader reader1("videos/fadeVid2.mpg");
		VideoInfo vidInf = reader1.getVideoInfo();
		
		VideoReader reader2("videos/fadeVid1.mpg");
		VideoInfo vidInf2 = reader2.getVideoInfo();

		RGBA* frameVid1 = (RGBA*)malloc( vidInf.height * vidInf.height * sizeof( RGBA ) );
		RGBA* frameVid2 = (RGBA*)malloc( vidInf2.height * vidInf2.height * sizeof( RGBA ) );
		RGBA* output = (RGBA*)malloc( vidInf2.height * vidInf2.height * sizeof( RGBA ) );

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

		FadingEffects fadingEff;

		while ( reader1.hasNextFrame() ) {
			reader1.changeFrameBuffer( frameVid1 );
			reader1.decodeNextFrame();
			
			std::cout << i << std::endl;

			reader2.changeFrameBuffer( frameVid2 );
			reader2.decodeNextFrame();

			fadingEff.fadeAdditive( frameVid1, frameVid2, output, vidInf2.width, vidInf2.height, ((float)i/(float)vidInf.nb_frames) );

			// write the image to output
			sprintf(filename,"pictures/pic%i.ppm",i);

			writePPM( output ,filename, vidInf2.width, vidInf2.height);
			i++;
		}
}
