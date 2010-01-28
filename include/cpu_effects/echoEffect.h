#include "types.h"

#include <stdio.h>
#include <iostream>
#include <cstdlib>

#include <vector>


namespace GRAVID {

    class EchoEffect {
    public:
        EchoEffect() {
            
        }
        
        void renderEcho( RGBA* inpic0, RGBA* inpic1,  RGBA* inpic2, RGBA* inpic3, RGBA* inpic4, RGBA* output_pic, int width, int height);
    };
    
}



