/*
 * File:   fading-cpu.h
 * Author: mk3
 *
 */
#include "cpu_effects/FadingEffects.h"
#include "types.h"
#include <iostream>

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
