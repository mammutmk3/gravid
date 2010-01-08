/*
 * writePPM.cpp
 *
 *  Created on: Jan 7, 2010
 *      Author: lars
 */

#include "writePPM.h"
#include "types.h"

#include <iostream>
#include <fstream>


void GRAVID::writePPM(const rgb* image, const char* filename, unsigned short width, unsigned short height){
	std::fstream file;
	file.open(filename,std::ios::out);
	// write PPM mode,dimensions and maximum-color value
	file << "P6" << std::endl << width << " " << height << std::endl << "255" << std::endl;
	// write the pixels
	for(int i=0;i<height;i++){
		for(int j=0;j<width;j++){
			file << (unsigned char)image[i*width+j].r
					<< (unsigned char)image[i*width+j].g
					<< (unsigned char)image[i*width+j].b;
		}
	}
	file.close();
}
