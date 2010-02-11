const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;


/**
* a circle in the center of the image get's bigger displaying the second video within it until it's completly established
*/
__kernel void circle_fade(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global float percentage ){
  __private int x = get_global_id(0);
  __private int y = get_global_id(1);

  __local float perc_local;
  __private int4 src_pixel;
  
  /* having multiple threads accessing the same value from global memory is slow */
  /* so copying to local(shared) memory makes sence even if the value is used only 1 time */
  if(0 == get_local_id(0) && 0 == get_local_id(1)){
  	 perc_local = percentage;
  }
  mem_fence(CLK_LOCAL_MEM_FENCE);  

  int4 img_dim = get_image_dim(src_img);
  __private int2 center;
  
  /* make use of a fast shift by 1 to divide by 2*/
  center.x = img_dim.x >> 1; center.y = img_dim.y >> 1;
  
  /* use the fact, that sqrt is continious, so comparing the power of 2 is just as good and faster*/
  /* use fast multiplication and addition functions */
  __private int quad_radius_viewable =  mad24(center.x,center.x,mul24(center.y,center.y));
  /* use fast native function for abs of a difference */
  __private int radius_width = abs_diff(x,center.x);
  __private int radius_height = abs_diff(y,center.y);
  __private int quad_radius_own =  mad24(radius_width,radius_width,mul24(radius_height,radius_height));
  
  /* some divergence is unevidable here */
  /* but making it just for writing an index is faster than serializing the whole copy process*/
  uchar index = (quad_radius_own <= quad_radius_viewable*perc_local)? 0: 1;
  src_pixel = read_imageui( src_img, sampler, (int4)(x,y,index,0));

  write_imageui( dst_img, (int2)(x,y), src_pixel);
}

/**
* fade in of 2 videos as one video seems to bite the other with teeth :)
*/

__kernel void teeth_fade(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global float percentage ){
  __private int2 coord;
  coord.x = get_global_id(0);
  coord.y = get_global_id(1);

  __local float perc_local;
  __private int4 src_pixel;
  
  /* having multiple threads accessing the same value from global memory is slow */
  /* so copying to local(shared) memory makes sence even if the value is used only 1 time */
  if(0 == get_local_id(0) && 0 == get_local_id(1)){
  	 perc_local = percentage;
  }
  mem_fence(CLK_LOCAL_MEM_FENCE);  

  __private int index;
  __private int img_height = get_global_size(1);
  __private int img_height_half = img_height >> 1;
  
  /* Half the peak width is defined 32 here*/
  /* mod 64 */
  __private int pos_in_peak = (coord.y >= img_height_half)?(coord.x+32)&63:coord.x & 63;
  __private float height_val = (img_height * perc_local) + (abs_diff(pos_in_peak,32) << 1);
  
  if(coord.y <= img_height_half)
    index = (coord.y <= height_val)?1:0;
  else
    index = (coord.y >= img_height-height_val)?1:0;
  
  src_pixel = read_imageui( src_img, sampler, (int4)(coord.x,coord.y,index,0));

  write_imageui( dst_img, (int2)(coord.x,coord.y), src_pixel);
}

/**
* normale additive fading effect
*/

__kernel void fadeAdditve(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global float percentage ){
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);

	__local int opacity;

	/* fetch to shared memory with barrier() */
	if ( get_local_id( 0 ) && get_local_id(1) == 0 ) {
		opacity = (int)255*percentage;
	}
	/* synchronize threads */
	barrier(CLK_LOCAL_MEM_FENCE);

	
	/* fetch from 3d-image */
	int4 act_pix1 = read_imageui( src_img, sampler, (int4)(x,y,0,0) );
	int4 act_pix2 = read_imageui( src_img, sampler, (int4)(x,y,1,0) );
	int4 out_pix;
	/* overlay both frames */
	out_pix.x = (( ( opacity * act_pix2.x ) + (( 255 - opacity ) * act_pix1.x ) )/ 255 );
	out_pix.y = (( ( opacity * act_pix2.y ) + (( 255 - opacity ) * act_pix1.y ) ) / 255 );
	out_pix.z = (( ( opacity * act_pix2.z ) + (( 255 - opacity ) * act_pix1.z ) ) / 255 );

	write_imageui( dst_img, (int2)(x,y), out_pix);
}

/**
* fading videos with overlaying blinds
*/

__kernel void fadeBlind(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global float g_percentage ){
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);

	__local int distance;
	__local float percentage;

	/* fetch to shared memory with barrier() */
	/* calculate equidistant distance*/
	/* how much blinds */
	if ( get_local_id( 0 ) && get_local_id(1) == 0 ) {
		percentage = g_percentage;
		int blinds = 7;
		distance = get_image_width( src_img ) / blinds;
	}
	/* synchronize threads */
	barrier(CLK_LOCAL_MEM_FENCE);

	
	/* percentage of the actual pixel in a blind */
	float pc_act_pix = (float)(x % distance) / distance;

	int4 out_pix;
	
	/* if the pixel is in the blind, take frame2, else frame1 */
	if ( pc_act_pix < percentage ) {
		out_pix = read_imageui( src_img, sampler, (int4)(x,y,0,0) );
	} else {
		out_pix = read_imageui( src_img, sampler, (int4)(x,y,1,0) );
	}
	write_imageui( dst_img, (int2)(x,y), out_pix);
	
}
