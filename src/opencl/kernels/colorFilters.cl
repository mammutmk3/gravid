const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
							CLK_ADDRESS_NONE |
							CLK_FILTER_NEAREST;

/* The gray filter */
__kernel void grayFilter(__read_only image2d_t srcImage, __global uchar4* dstImage){
	
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);
	int2 dimension = get_image_dim(srcImage);
	
	__private uint4 srcPixel = read_imageui(srcImage, sampler, (int2)(x,y));
	__private uchar4 dstPixel;
	
	__private uchar gray = (uchar)(0.299 * srcPixel.x + 0.587 * srcPixel.y + 0.114 * srcPixel.z);
	dstPixel.x = dstPixel.y = dstPixel.z = gray;
	
	dstImage[y*dimension.x+x] = dstPixel;
}

/* the sepia filter */
__kernel void sepiaFilter(__read_only image2d_t srcImage, __global uchar4* dstImage){
	
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
	
	dstImage[y*dimension.x+x] = dstPixel;
}