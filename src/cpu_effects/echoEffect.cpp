/*
 * File:   echo-cpu.h
 * Author: mk3
 *
 * Created on 15. Januar 2010, 17:41
 */
#include "cpu_effects/echoEffect.h"
#include "types.h"

using namespace GRAVID;


void EchoEffect::renderEcho( std::vector<RGBA*> frames, RGBA* &output_pic_param, int width, int height, int loopCnt, int frames_cnt) {
	/* max random aberration of the overlayed frames */
	int randomness = 30;
	/* opacity of the overlayed frames, 102 = 40%*/
	int opacity = 102;
	int start_address = loopCnt;
	//std::cout << "loops: "<< loopCnt<< " frames" << frames_cnt << std::endl;
	start_address = loopCnt % frames_cnt;
	//std::cout << "= startaddress: "<< start_address  << std::endl;
	
	RGBA* start_pix = frames[start_address];
	RGBA* output_pic = frames[start_address];
		
	for (int n = 1; n<(frames_cnt); n++) {
		/* get background frames in the right order */
		int i = (start_address-n);

		if ( i<0)
			i = frames_cnt + i;
		//std::cout << i << " = start: " << start_address << " - n: " << n << std::endl;		
		
		RGBA* act_pix = frames[i];
		
		int random =  output_pic[i].r % (randomness * 2);
		int vert_translation = (randomness - random);
		random = output_pic[i].g % (randomness * 2);
		int hor_translation = ( randomness - random);
		
		//std::cout << "vert: " << vert_translation << std::endl;
		//std::cout << "hor: " << hor_translation << std::endl;

		for (int h=0; h<height; h++) {
			for (int w=0; w<width; w++) {

				//Adresse berechnen
				int address = (w+width*h);
				
				int perturbation = address;
				int hor_address = w + ( hor_translation );
				if ( hor_address < 0 )
					perturbation = address + hor_translation + abs(hor_address);
				if ( hor_address > width )
					perturbation = address + hor_translation + ( w - hor_translation );

				int vert_address = h + (vert_translation);
				if ( vert_address < 0 )
					perturbation = perturbation - ( h * width);
				if ( vert_address > height )
					perturbation = (width * height - (perturbation%width));
				
				RGBA act_pix_val = act_pix[address];
				RGBA start_pix_val =  start_pix[address];

				unsigned char r = ( ( ( opacity * act_pix_val.r ) + (( 255 - opacity ) * output_pic[perturbation].r) ) / 255 );
				unsigned char g = ( ( ( opacity * act_pix_val.g ) + (( 255 - opacity ) * output_pic[perturbation].g) ) / 255 );
				unsigned char b = ( ( ( opacity * act_pix_val.b ) + (( 255 - opacity ) * output_pic[perturbation].b) ) / 255 );
				output_pic[address].r = r;
				output_pic[address].g = g;
				output_pic[address].b = b;
			}
		}
		/* and overlaying once again, for better visibility */
		for (int h=0; h<height; h++) {
			for (int w=0; w<width; w++) {
				int address = (w+width*h);
				output_pic[address].r = ( ( ( 75 * start_pix[address].r ) + (( 255 - 75 ) * output_pic[address].r) ) / 255 );
				output_pic[address].g = ( ( ( 75 * start_pix[address].g ) + (( 255 - 75 ) * output_pic[address].g) ) / 255 );
				output_pic[address].b = ( ( ( 75 * start_pix[address].b ) + (( 255 - 75 ) * output_pic[address].b) ) / 255 );
			}
		}
		output_pic_param = output_pic;
	}
}