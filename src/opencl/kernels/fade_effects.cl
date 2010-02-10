const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;

/**
* testing kernel
*/

__kernel void testFade(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global float percentage ){
  __private size_t x = get_global_id(0);
  __private size_t y = get_global_id(1);

  int4 act_pix;
  int bla = (percentage * 96);
  act_pix = read_imageui( src_img, sampler, (int4)(x,y,bla%2,0) );

  write_imageui( dst_img, (int2)(x,y), act_pix);
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
