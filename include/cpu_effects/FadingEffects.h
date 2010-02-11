/*
 * greyFilter.h
 *
 *  Created on: Feb 11, 2010
 *      Author: lars / markus
 */

#ifndef FADINGEFFECTS_H_
#define FADINGEFFECTS_H_

#include "types.h"

namespace GRAVID {

    class FadingEffects {
    public:
        FadingEffects() {
            
        }
        
        void fadeAdditive( RGBA* inpic0, RGBA* inpic1,  RGBA* output_pic, int width, int height, float act_percent);
		void fadeBlind(RGBA* inpic0, RGBA* inpic1, RGBA* output_pic, int width, int height, float act_percent);

		void circle(RGBA* in_img_1, RGBA* in_img_2, RGBA* out_img, size_t width, size_t height, float act_percent);
		void teeth(RGBA* in_img_1, RGBA* in_img_2, RGBA* out_img, size_t width, size_t height, float act_percent);

    };
    
}

#endif /* FADINGEFFECTS_H_ */
