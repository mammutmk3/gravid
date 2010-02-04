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
//		tmp.r = act_pix1.r;
//		tmp.g = act_pix1.g;
//		tmp.b = act_pix1.b;
		output_pic[address] = tmp;
        }
    }
}
