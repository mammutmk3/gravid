/*
 * main.cpp
 *
 *  Created on: Jan 6, 2010
 *      Author: lars
 */

#include "video.h"
#include "types.h"
#include "writePPM.h"
#include "sobel-cpu.h"

#include <iostream>
#include <cstdlib>
#include <stdio.h>


using namespace GRAVID;

int main(int argc, char** argv){
	// register all available codecs in mpeg
	av_register_all();

	/*
	 * start useful stuff here
	 */

	rgb* nextFrame;

	try{
		// create a video object, retrieve all frames and write them as PPM in a subdirectory "pictures"
		Video myVideo = Video("/home/mk3/testvid.mpg");
		char filename[50];
		for(unsigned short i=0; myVideo.hasNextFrame(); i++){
			sprintf(filename,"/home/mk3/gravid/test%i.ppm",i);
			nextFrame = myVideo.getNextFrame();
                        rgb* outputPic = nextFrame;
                        //sobel_cpu(nextFrame, outputPic, myVideo.getWidth(), myVideo.getHeight() );
			writePPM(outputPic, filename, myVideo.getWidth(), myVideo.getHeight());
		}
	}
	catch(std::logic_error e){

		// print an error message
		std::cerr << e.what() << std::endl;
		// close the program
		exit(-1);
	}

	/*
	 * end useful stuff here
	 */
}
