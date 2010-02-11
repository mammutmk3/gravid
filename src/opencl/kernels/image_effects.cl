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
	
	__private uchar gray = (uchar)(mad(0.299f,(float)srcPixel.x,mad(0.587f,(float)srcPixel.y,0.114f * (float)srcPixel.z)));
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
	
	__private uchar gray = (uchar)(mad(0.299f,(float)srcPixel.x,mad(0.587f,(float)srcPixel.y,0.114f * (float)srcPixel.z)));
	/* sepia factor is 20 here */
	dstPixel.x = min(gray + 40, 255);
	dstPixel.y = min(gray + 20,255);
	dstPixel.z = gray;
	
	write_imageui(dstImage, (int2)(x,y), (int4)(dstPixel.x,dstPixel.y,dstPixel.z,dstPixel.w));
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
	int p00 = (int)( (act_pix.x + act_pix.y + act_pix.z)/3 );
	
	act_pix = read_imageui( src_img, sampler, (int2)(x-1,y) );
	int p01 = (int)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x-1,y+1) );
	int p02 = (int)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x,y-1) );
	int p10 = (int)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x,y+1) );
	int p12 = (int)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x+1,y-1) );
	int p20 = (int)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x+1,y) );
	int p21 = (int)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	act_pix = read_imageui( src_img, sampler, (int2)(x+1,y+1) );
	int p22 = (int)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	/* hold the middle of the kernel, so we dont need to fetch the value again */
	act_pix = read_imageui( src_img, sampler, (int2)(x,y) );
	int p11 = (int)( (act_pix.x + act_pix.y + act_pix.z)/3 );

	/* Horizontal */
	/* sum_x  = ( (p22*-1) + (p20*1) + (p12*-2) + (p10*2) + (p02*-1) + (p00*1) ); */
	sum_x  = ( mad24(p22,-1,p20) + mul24(p12,-2) + mul24(p10,2) + mad24(p02,-1,p00) );
	
	/* Vertical */
	/* sum_y  = ((p22*1) + (p21*2) + (p20*1) + (p02*-1) + (p01*-2) + (p00*-1));*/
	sum_y  = ( mad24(p21,2,p22) + mad24(p02,-1,p20) + mul24(p01,-2) + mul24(p00,-1) );

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

__kernel void gaussFilter3(__read_only image2d_t srcImage, __write_only image2d_t dst_img){
	
	uint x = get_global_id(0);
	uint y = get_global_id(1);

	int2 idim = get_image_dim(srcImage);

	__private uint4  srcPixel;
	
	__private float ret_r = 0.0f;
	__private float ret_g = 0.0f;
	__private float ret_b = 0.0f;

	/* pos: (-1,-1) */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-1,y-1));
	ret_r 		+= 	((float)srcPixel.x * 0.0625f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0625f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0625f);

	/* pos: (0,-1) */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x,y-1));
	ret_r 		+= 	((float)srcPixel.x * 0.125f); 
	ret_g 		+= 	((float)srcPixel.y * 0.125f); 
	ret_b 		+= 	((float)srcPixel.z * 0.125f);

	/* pos: (1,-1) */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+1,y-1));
	ret_r 		+= 	((float)srcPixel.x * 0.0625f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0625f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0625f);

	/* pos: (-1,0) */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-1,y));
	ret_r 		+= 	((float)srcPixel.x * 0.125f); 
	ret_g 		+= 	((float)srcPixel.y * 0.125f); 
	ret_b 		+= 	((float)srcPixel.z * 0.125f);

	/* pos: (0,0) */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x,y));
	ret_r 		+= 	((float)srcPixel.x * 0.25f); 
	ret_g 		+= 	((float)srcPixel.y * 0.25f); 
	ret_b 		+= 	((float)srcPixel.z * 0.25f);

	/* pos: (1,0) */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+1,y));
	ret_r 		+= 	((float)srcPixel.x * 0.125f); 
	ret_g 		+= 	((float)srcPixel.y * 0.125f); 
	ret_b 		+= 	((float)srcPixel.z * 0.125f);

	/* pos: (-1,1) */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-1,y+1));
	ret_r 		+= 	((float)srcPixel.x * 0.0625f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0625f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0625f);

	/* pos: (0,1) */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x,y+1));
	ret_r 		+= 	((float)srcPixel.x * 0.125f); 
	ret_g 		+= 	((float)srcPixel.y * 0.125f); 
	ret_b 		+= 	((float)srcPixel.z * 0.125f);

	/* pos: (1,1) */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+1,y+1));
	ret_r 		+= 	((float)srcPixel.x * 0.0625f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0625f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0625f);
	
	write_imageui( dst_img, (int2)(x,y), (int4)( ret_r, ret_g, ret_b, 0 ) );
};


__kernel void gaussFilter5(__read_only image2d_t srcImage, __write_only image2d_t dst_img){
	
	uint x = get_global_id(0);
	uint y = get_global_id(1);

	int2 idim = get_image_dim(srcImage);

	__private uint4  srcPixel;

	__private float ret_r = 0.0f;
	__private float ret_g = 0.0f;
	__private float ret_b = 0.0f;

	/* -2,-2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-2,y-2));
	ret_r	 	+= 	((float)srcPixel.x * 0.003086f); 
	ret_g 		+= 	((float)srcPixel.y * 0.003086f); 
	ret_b 		+= 	((float)srcPixel.z * 0.003086f);

	/* -0,-1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-2,y-1));
	ret_r	 	+= 	((float)srcPixel.x * 0.012346f); 
	ret_g 		+= 	((float)srcPixel.y * 0.012346f); 
	ret_b 		+= 	((float)srcPixel.z * 0.012346f);

	/* -2,0 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-2,y));
	ret_r	 	+= 	((float)srcPixel.x * 0.0247f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0247f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0247f);

	/* -2,-1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-2,y+1));
	ret_r	 	+= 	((float)srcPixel.x * 0.012346f); 
	ret_g 		+= 	((float)srcPixel.y * 0.012346f); 
	ret_b 		+= 	((float)srcPixel.z * 0.012346f);

	/* -2,+2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-2,y+2));
	ret_r	 	+= 	((float)srcPixel.x * 0.003086f); 
	ret_g 		+= 	((float)srcPixel.y * 0.003086f); 
	ret_b 		+= 	((float)srcPixel.z * 0.003086f);

	/* -1,-2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-1,y-2));
	ret_r	 	+= 	((float)srcPixel.x * 0.012346f); 
	ret_g 		+= 	((float)srcPixel.y * 0.012346f); 
	ret_b 		+= 	((float)srcPixel.z * 0.012346f);

	/* -1,-1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-1,y-1));
	ret_r	 	+= 	((float)srcPixel.x * 0.049383f); 
	ret_g 		+= 	((float)srcPixel.y * 0.049383f); 
	ret_b 		+= 	((float)srcPixel.z * 0.049383f);

	/* -1,0 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-1,y));
	ret_r	 	+= 	((float)srcPixel.x * 0.0987654f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0987654f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0987654f);

	/* -1,+1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-1,y+1));
	ret_r	 	+= 	((float)srcPixel.x * 0.049383f); 
	ret_g 		+= 	((float)srcPixel.y * 0.049383f); 
	ret_b 		+= 	((float)srcPixel.z * 0.049383f);

	/* -1,+2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x-1,y+2));
	ret_r	 	+= 	((float)srcPixel.x * 0.012346f); 
	ret_g 		+= 	((float)srcPixel.y * 0.012346f); 
	ret_b 		+= 	((float)srcPixel.z * 0.012346f);

	/* 0,-2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x,y-2));
	ret_r	 	+= 	((float)srcPixel.x * 0.0247f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0247f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0247f);

	/* 0,-1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x,y-1));
	ret_r	 	+= 	((float)srcPixel.x * 0.0987654f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0987654f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0987654f);

	/* 0,0 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x,y));
	ret_r	 	+= 	((float)srcPixel.x * 0.197531f); 
	ret_g 		+= 	((float)srcPixel.y * 0.197531f); 
	ret_b 		+= 	((float)srcPixel.z * 0.197531f);

	/* 0,+1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x,y+1));
	ret_r	 	+= 	((float)srcPixel.x * 0.0987654f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0987654f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0987654f);

	/* 0,+2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x,y+2));
	ret_r	 	+= 	((float)srcPixel.x * 0.0247f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0247f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0247f);

	/* 1,-2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+1,y-2));
	ret_r	 	+= 	((float)srcPixel.x * 0.012346f); 
	ret_g 		+= 	((float)srcPixel.y * 0.012346f); 
	ret_b 		+= 	((float)srcPixel.z * 0.012346f);

	/* 1,-1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+1,y-1));
	ret_r	 	+= 	((float)srcPixel.x * 0.049383f); 
	ret_g 		+= 	((float)srcPixel.y * 0.049383f); 
	ret_b 		+= 	((float)srcPixel.z * 0.049383f);

	/* 1,0 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+1,y));
	ret_r	 	+= 	((float)srcPixel.x * 0.0987654f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0987654f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0987654f);

	/* 1,+1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+1,y+1));
	ret_r	 	+= 	((float)srcPixel.x * 0.049383f); 
	ret_g 		+= 	((float)srcPixel.y * 0.049383f); 
	ret_b 		+= 	((float)srcPixel.z * 0.049383f);

	/* 1,+2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+1,y+2));
	ret_r	 	+= 	((float)srcPixel.x * 0.012346f); 
	ret_g 		+= 	((float)srcPixel.y * 0.012346f); 
	ret_b 		+= 	((float)srcPixel.z * 0.012346f);
	
	/* 2,-2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+2,y-2));
	ret_r	 	+= 	((float)srcPixel.x * 0.003086f); 
	ret_g 		+= 	((float)srcPixel.y * 0.003086f); 
	ret_b 		+= 	((float)srcPixel.z * 0.003086f);

	/* 0,-1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+2,y-1));
	ret_r	 	+= 	((float)srcPixel.x * 0.012346f); 
	ret_g 		+= 	((float)srcPixel.y * 0.012346f); 
	ret_b 		+= 	((float)srcPixel.z * 0.012346f);

	/* 2,0 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+2,y));
	ret_r	 	+= 	((float)srcPixel.x * 0.0247f); 
	ret_g 		+= 	((float)srcPixel.y * 0.0247f); 
	ret_b 		+= 	((float)srcPixel.z * 0.0247f);

	/* 2,-1 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+2,y+1));
	ret_r	 	+= 	((float)srcPixel.x * 0.012346f); 
	ret_g 		+= 	((float)srcPixel.y * 0.012346f); 
	ret_b 		+= 	((float)srcPixel.z * 0.012346f);

	/* 2,+2 */
	srcPixel 	= 	read_imageui(srcImage, sampler, (int2)(x+2,y+2));
	ret_r	 	+= 	((float)srcPixel.x * 0.003086f); 
	ret_g 		+= 	((float)srcPixel.y * 0.003086f); 
	ret_b 		+= 	((float)srcPixel.z * 0.003086f);

	write_imageui( dst_img, (int2)(x,y), (int4)( ret_r, ret_g, ret_b, 0 ) );
} ;