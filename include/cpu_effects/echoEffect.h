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
        
        void renderEcho( std::vector<RGBA*> frames, RGBA* &output_pic, int width, int height, int loopCnt, int frames_cnt);
    };
    
}



