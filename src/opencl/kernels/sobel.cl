const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |	CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

__kernel void sobel(__read_only image2d_t src_img, __global uchar4* dst_img){

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


	/* values for the convolution-kernels 
	gx[0][0] = -1;  gx[0][1] = 0;   gx[0][2] = 1;
	gx[1][0] = -2;  gx[1][1] = 0;   gx[1][2] = 2;
	gx[2][0] = -1;  gx[2][1] = 0;   gx[2][2] = 1;

	gy[0][0] = 1;   gy[0][1] = 2;   gy[0][2] = 1;
	gy[1][0] = 0;   gy[1][1] = 0;   gy[1][2] = 0;
	gy[2][0] = -1;  gy[2][1] = -2;  gy[2][2] = -1;
	*/

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
		dst_img[y * dim.x + x] = tmp;
	} else {
		tmp.x = tmp.y = tmp.z = 0;
		dst_img[y * dim.x + x] = tmp;
	}
	/*	write_imageui( dst_img, (int2)(x,y), act_pix );*/
}
