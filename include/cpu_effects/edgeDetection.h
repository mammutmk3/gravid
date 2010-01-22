/* 
 * File:   sobel-cpu.h
 * Author: mk3
 *
 * Created on 15. Januar 2010, 17:41
 */
#include "types.h"

#include <stdio.h>
#include <iostream>
#include <cstdlib>

#include <vector>

#ifndef _SOBEL_CPU_H
#define	_SOBEL_CPU_H



#endif	/* _SOBEL_CPU_H */

namespace GRAVID {

    class EdgeDetection {
    public:
        EdgeDetection() {
            
        }
        
        void sobelOperator(RGBA* pixarray, RGBA* output_pic, int width, int height);
    };
    
}



