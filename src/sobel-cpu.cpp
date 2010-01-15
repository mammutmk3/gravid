/*
 * File:   sobel-cpu.h
 * Author: mk3
 *
 * Created on 15. Januar 2010, 17:41
 */

#include "sobel-cpu.h"


void sobel_cpu(rgb* pixarray, rgb* &output_pic, int width, int height) {
    /* convert colour-picture into grayscale */
    /* malloc have to be done from the host */
    unsigned char* gray = (unsigned char*) malloc (sizeof(unsigned char)*width*height);
    for (int i=0; i<height; i++) {
        for (int n=0; n<width; n++) {
            int address = (n+width*i);
            gray[address] = (unsigned char)(( pixarray[address].r + pixarray[address].g + pixarray[address].b) / 3);
        }
    }

    /* convert into vector, in opencl use Image2D */
    std::vector< std::vector<int> > gray_img(height);
    for (int i=0; i<height; i++) {
        gray_img[i].resize(width);
    }

    /* copy data into vector */
    for (int i=0; i<height; i++) {
        for (int n=0; n<width; n++) {
            gray_img[i][n] = gray[ (n+width*i) ];
        }
    }

    /* convolution-kernels */
    int gx[3][3];
    int gy[3][3];

    /* values for the convolution-kernels */
    gx[0][0] = -1;  gx[0][1] = 0;   gx[0][2] = 1;
    gx[1][0] = -2;  gx[1][1] = 0;   gx[1][2] = 2;
    gx[2][0] = -1;  gx[2][1] = 0;   gx[2][2] = 1;

    gy[0][0] = 1;   gy[0][1] = 2;   gy[0][2] = 1;
    gy[1][0] = 0;   gy[1][1] = 0;   gy[1][2] = 0;
    gy[2][0] = -1;  gy[2][1] = -2;  gy[2][2] = -1;

    for (int i=0; i<height; i++) {
        for (int n=0; n<width; n++) {

            int sum=0;
            int sum_c=0;
            int sum_x=0;
            int sum_y=0;

            /* convolution, optimized for linear arrays, better with Image() */

            /* image boundaries */
            if ( !( i==0 || i==(height-1) ) & !( n==0 || n==(width-1) ) ) {

                /*convolution in x-dir*/
                for ( int h=-1; h<=1; h++ ) {
                    for ( int k=-1; k<=1; k++  ) {
                        sum_x = sum_x + (gx[h+1][k+1])*(gray_img[i-h][n-k]);
                    }
                }

                /*convolution in y-dir*/
                for ( int h=-1; h<=1; h++ ) {
                    for ( int k=-1; k<=1; k++  ) {
                        sum_y = sum_y + (gy[h+1][k+1])*(gray_img[i-h][n-k]);
                    }
                }
            }

            if ( abs(sum_x) + abs(sum_y) > 255) {
                output_pic[(n+width*i)].r = pixarray[ (n+width*i) ].r;
                output_pic[(n+width*i)].g = pixarray[ (n+width*i) ].g;
                output_pic[(n+width*i)].b = pixarray[ (n+width*i) ].b;
            } else {
                output_pic[(n+width*i)].r = 0;
                output_pic[(n+width*i)].g = 0;
                output_pic[(n+width*i)].b = 0;
            }

            /* 720x576 */
        }
    }
}


