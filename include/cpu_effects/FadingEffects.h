#include "types.h"

#include <stdio.h>
#include <iostream>
#include <cstdlib>

#include <vector>


namespace GRAVID {

    class FadingEffects {
    public:
        FadingEffects() {
            
        }
        
        void fadeAdditive( RGBA* inpic0, RGBA* inpic1,  RGBA* output_pic, int width, int height, float act_percent);

    };
    
}



