/*
 * main.cpp
 *
 *  Created on: Jan 6, 2010
 *      Author: lars
 */

#include "videoReader.h"
#include "writePPM.h"
#include "colorFilter.h"
#include "edgeDetection.h"
#include "types.h"

extern "C"{
	#include <libavformat/avformat.h>
}

#include <iostream>
#include <cstdlib>
#include <stdexcept>


using namespace GRAVID;

int main(int argc, char** argv){
	// register all available codecs in mpeg
	av_register_all();

	/*
	 * start useful stuff here
	 */
	ColorFilter cFilt;
        EdgeDetection edgeDet;

	try{
		// width and height of the images
		unsigned short width, height;
		// source image pointer
		RGB* srcImage;

		// read a video
		VideoReader myVideo("videos/video1.mpg");
		srcImage = myVideo.getNextFrame();

		width = myVideo.getWidth();
		height = myVideo.getHeight();

		// the result picture
		RGB greyImage[width * height];
		RGB sepiaImage[width * height];
                RGB sobelImage[width * height];

		// apply the filters
		cFilt.applyFilter(srcImage,width,height, greyImage, GRAY_FILTER);
		cFilt.applyFilter(srcImage,width,height, sepiaImage, SEPIA_FILTER);
                edgeDet.sobelOperator(srcImage, sobelImage, width, height);

		// write the filtered images in PPM files
		writePPM(greyImage, "pictures/gray.ppm", width, height);
		writePPM(sepiaImage, "pictures/sepia.ppm", width, height);
                writePPM(sobelImage, "pictures/sobel.ppm", width, height);
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
