const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
							CLK_ADDRESS_NONE |
							CLK_FILTER_NEAREST;

/* The gray filter */
__kernel void grayFilter(__read_only image2d_t srcImage, __write_only image2d_t dstImage){
	
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);
	int2 dimension = get_image_dim(srcImage);
	
	__private uint4 srcPixel = read_imageui(srcImage, sampler, (int2)(x,y));
	__private uchar4 dstPixel;
	
	__private uchar gray = (uchar)(0.299 * srcPixel.x + 0.587 * srcPixel.y + 0.114 * srcPixel.z);
	dstPixel.x = dstPixel.y = dstPixel.z = gray;
	
	write_imageui(dstImage, (int2)(x,y), (int4)(dstPixel.x,dstPixel.y,dstPixel.z,dstPixel.w));
}

/* the sepia filter */
__kernel void sepiaFilter(__read_only image2d_t srcImage, __write_only image2d_t dstImage){
	
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);
	int2 dimension = get_image_dim(srcImage);
	
	__private uint4 srcPixel = read_imageui(srcImage, sampler, (int2)(x,y));
	__private uchar4 dstPixel;
	
	__private const sepiaFactor = 20;
	
	__private uchar gray = (uchar)(0.299 * srcPixel.x + 0.587 * srcPixel.y + 0.114 * srcPixel.z);
	dstPixel.x = min(gray + 2* sepiaFactor, 255);
	dstPixel.y = min(gray + sepiaFactor,255);
	dstPixel.z = gray;
	
	write_imageui(dstImage, (int2)(x,y), (int4)(dstPixel.x,dstPixel.y,dstPixel.z,dstPixel.w));
	/*dstImage[y*dimension.x+x] = dstPixel;*/
}


/* sobel edge-detection */

__kernel void sobelFilter(__read_only image2d_t src_img, __write_only image2d_t dst_img){

	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);

        int sum_x=0;
        int sum_y=0;


        /*convolution in x-dir, with unrolled loops*/

	/* read the pixel and his neighbourhood and convert it to gray */
	int4 act_pix = read_imageui( src_img, sampler, (int2)(x-1,y-1) );
	uchar p00 = (uchar)( (act_pix.x + act_pix.y + act_pix.z)/3 );
	
	act_pix = read_imageui( src_img, sampler, (int2)(x-1,y) );
	uchar p01 = (uchar)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x-1,y+1) );
	uchar p02 = (uchar)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x,y-1) );
	uchar p10 = (uchar)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x,y+1) );
	uchar p12 = (uchar)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x+1,y-1) );
	uchar p20 = (uchar)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x+1,y) );
	uchar p21 = (uchar)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x+1,y+1) );
	uchar p22 = (uchar)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	/* hold the middle of the kernel, so we dont need to fetch the value again (cached?) */
	act_pix = read_imageui( src_img, sampler, (int2)(x,y) );
	uchar p11 = (uchar)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	/* Horizontal */	
	sum_x  = ( (p22*-1) + (p20*1) + (p12*-2) + (p10*2) + (p02*-1) + (p00*1) );
	
	/* Vertical */	
	sum_y  = ((p22*1) + (p21*2) + (p20*1) + (p02*-1) + (p01*-2) + (p00*-1));

	int2 dim = get_image_dim(src_img);
	uchar4 tmp;

	if ( abs(sum_x) + abs(sum_y) > 255 ) {
		tmp.x = act_pix.x;
		tmp.y = act_pix.y;
		tmp.z = act_pix.z;
	} else {
		tmp.x = tmp.y = tmp.z = 0;
	}
	write_imageui( dst_img, (int2)(x,y), (int4)( tmp.x, tmp.y, tmp.z, tmp.w ) );
}
