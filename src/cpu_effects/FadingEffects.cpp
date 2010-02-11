/*
 * File:   fading-cpu.h
 * Author: mk3 / Lars Kühne
 *
 */
#include "cpu_effects/FadingEffects.h"
#include "types.h"

#include <stdlib.h>
#include <math.h>

using namespace GRAVID;

void FadingEffects::fadeAdditive(RGBA* inpic0, RGBA* inpic1, RGBA* output_pic, int width, int height, float act_percent) {

	// calculate opacity
	int opacity = (int)255*act_percent;
// 	std::cout << opacity << std::endl;
	
    for (int i=0; i<height; i++) {
        for (int n=0; n<width; n++) {

		//Adresse berechnen
		int address = (n+width*i);

		RGBA act_pix1 = inpic0[address];
		RGBA act_pix2 = inpic1[address];
		unsigned char red = (unsigned char)( ( ( opacity * act_pix2.r ) + (( 255 - opacity ) * act_pix1.r ) )/ 255 );
		unsigned char green  = (unsigned char)( ( ( opacity * act_pix2.g ) + (( 255 - opacity ) * act_pix1.g ) ) / 255 );
		unsigned char blue = (unsigned char)( ( ( opacity * act_pix2.b ) + (( 255 - opacity ) * act_pix1.b ) ) / 255 );

		RGBA tmp;
		tmp.r = red;
		tmp.g = green;
		tmp.b = blue;
		output_pic[address] = tmp;
        }
    }
}


void FadingEffects::fadeBlind(RGBA* inpic0, RGBA* inpic1, RGBA* output_pic, int width, int height, float act_percent) {
	
    for (int h=0; h<height; h++) {
        for (int w=0; w<width; w++) {

		//Adresse berechnen
		int address = (w+width*h);

		/* calculate equidistant distance*/
		/* how much blinds */
		int blinds = 7;
		int distance = width / blinds;
		
		/* entfernung berechnen wie weit das erste video noch gehen soll */
		/*if n zwischen abstand und grenze der jalousie, dann nimm video 1, else video2 */
		
		/* percentage of the actual pixel in a blind */
		float pc_act_pix = (float)(w % distance) / distance;		

		RGBA tmp;
		
		if ( pc_act_pix < act_percent ) {
			tmp = inpic0[address];
		} else {
			tmp = inpic1[address];
		}
		output_pic[address] = tmp;
        }
    }
}

void FadingEffects::circle(RGBA* in_img_1, RGBA* in_img_2, RGBA* out_img, size_t width, size_t height, float act_percent){
	// calculate the center
	unsigned int center[2] = {width / 2, height / 2};

	// radius of the viewable circle
	unsigned int radius_view = sqrt((center[0]*center[0] + center[1]*center[1])) * act_percent;

	// radius of the current pixel
	unsigned int radius_own, radius_width, radius_height;

	for(unsigned int x=0;x<width;x++){
		for(unsigned int y=0;y<width;y++){
			radius_width = abs(x-center[0]);
			radius_height = abs(y-center[1]);
			radius_own = sqrt(radius_width*radius_width+radius_height*radius_height);
			out_img[y*width+x] = (radius_own <= radius_view)?in_img_2[y*width+x]:in_img_1[y*width+x];
		}
	}
}

void FadingEffects::teeth(RGBA* in_img_1, RGBA* in_img_2, RGBA* out_img, size_t width, size_t height, float act_percent){
	// calculate the center
	unsigned int pos_in_peak;
	float height_val;

	const unsigned int peak_width = 64;
	const size_t height_half = height / 2 ;

	RGBA* src_img;

	for(unsigned int x=0;x<width;x++){
		for(unsigned int y=0;y<width;y++){
			pos_in_peak = (y >= height_half)?x % peak_width :(x+peak_width/2) % peak_width;
			height_val = (height * act_percent) + abs(pos_in_peak - peak_width / 2) * 2;

			if(y <= height_half)
				src_img = (y<=height_val)?in_img_1:in_img_2;
			else
				src_img = (y >= height_val)?in_img_1:in_img_2;

			out_img[y*width+x] = src_img[y*width+x];
		}
	}
}
