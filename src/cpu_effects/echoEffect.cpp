/*
 * File:   echo-cpu.h
 * Author: mk3
 *
 * Created on 15. Januar 2010, 17:41
 */
#include "cpu_effects/echoEffect.h"
#include "types.h"

using namespace GRAVID;


void EchoEffect::renderEcho(
	RGBA* inpic0, 
	RGBA* inpic1,  
	RGBA* inpic2, 
	RGBA* inpic3,
	RGBA* inpic4, 
	RGBA* output_pic, 
	int width, 
	int height) 
{

    for (int i=0; i<height; i++) {
        for (int n=0; n<width; n++) {

		//Adresse berechnen
		int address = (n+width*i);
		int opacity = 102;

		// Debug:
// 		if ( inpic1[address].r != inpic0[address].r) {
// 			printf("r0: %d, r1: %d \n", inpic1[address].r, inpic0[address].r);
// 			printf("r0: %d, r1: %d \n", inpic1[address].g, inpic0[address].g);
// 			printf("r0: %d, r1: %d \n", inpic1[address].b, inpic0[address].b);
// 		}

		// Wert fuer Rot berechnen
		unsigned char rot = (unsigned char)( ( ( opacity * inpic1[address].r ) + (( 255 - opacity ) * inpic0[address].r ) )/ 255 );
		rot = (unsigned char)((( opacity * inpic2[address].r ) + (( 255 - opacity ) * rot )) / 255);
		rot = (unsigned char)((( opacity * inpic3[address].r ) + (( 255 - opacity ) * rot )) / 255);
		rot = (unsigned char)((( opacity * inpic4[address].r ) + (( 255 - opacity ) * rot )) / 255);

		// Wert fuer Gelb berechnen
		unsigned char gelb = (unsigned char)( ( ( opacity * inpic1[address].g ) + (( 255 - opacity ) * inpic0[address].g ) ) / 255 );
		gelb = (unsigned char)((( opacity * inpic2[address].g ) + (( 255 - opacity ) * gelb )) / 255);
		gelb = (unsigned char)((( opacity * inpic3[address].g ) + (( 255 - opacity ) * gelb )) / 255);
		gelb = (unsigned char)((( opacity * inpic4[address].g ) + (( 255 - opacity ) * gelb )) / 255);

		// Wert fuer Blau berechnen
		unsigned char blau = (unsigned char)( ( ( opacity * inpic1[address].b ) + (( 255 - opacity ) * inpic0[address].b ) ) / 255 );
		blau = (unsigned char)((( opacity * inpic2[address].b ) + (( 255 - opacity ) * blau )) / 255);
		blau = (unsigned char)((( opacity * inpic3[address].b ) + (( 255 - opacity ) * blau )) / 255);
		blau = (unsigned char)((( opacity * inpic4[address].b ) + (( 255 - opacity ) * blau )) / 255);

		//zurueckschreiben(unsigned char)
		output_pic[address].r = rot; 
		output_pic[address].g = gelb;
		output_pic[address].b = blau;

/*		output_pic[address].r = (unsigned char)inpic4[address].r; 
		output_pic[address].g = (unsigned char)inpic4[address].g;
		output_pic[address].b = (unsigned char)inpic4[address].b;
*/		
	}
    }
}
