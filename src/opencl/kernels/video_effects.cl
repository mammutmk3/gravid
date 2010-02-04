const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
 
/*__kernel void echoeffect(__read_only image3d_t src_img, __global uchar4* dst_img){*/
__kernel void echoeffect(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global int gLoopCnt ){
 
/* paramter Wievielter Aufruf (Frame) das ist, daraus berechnen wieviel Frames es davor überhaupt gibt */ 
/* todo: variable anzhal an vergangenheitsframes beachten */
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);
	__local int randomness = 10;
	__local int opacity = 102;
	__local int loopCnt;
	__local int frames_cnt = get_image_dim( src_img ).z;
 
	/* opacity of the overlayed frames, 102 = 40%*/
	/* fetch to shared memory with barrier() */
	if ( get_local_id( 0 ) & get_global_id(1) == 0 ) {
		loopCnt = gLoopCnt;
		/*randomness  = gRandomness;
		opacity = gOpacity;*/
	}
	/* synchronize threads */
	barrier(CLK_LOCAL_MEM_FENCE);
 
	/* appoint the starting memory address */
	int start_address = loopCnt % frames_cnt;
/*	if (start_address == 0) {
		start_address =  frames_cnt-1;
	} else {
		start_address = start_address-1;
	}*/
 
	int4 act_pix;
	int4 out_pix = read_imageui( src_img, sampler, (int4)(x,y,start_address,0) );

	/* overlaying */
	for (int n = start_address; n<= (start_address + frames_cnt); n++) {
		int i = n % (start_address + frames_cnt);

		/* pseudo-random perturbation of the single frames*/
		int4 start_pix = read_imageui( src_img, sampler, (int4)(i,i,i,0) );
		int random = start_pix.x % (randomness * 2);
		random = ( randomness - random );
		int vert_translation = random + x;
		int hor_translation = random + y;

		int4 act_pix2 = read_imageui( src_img, sampler, (int4)(x, y,i,0) );
		
		out_pix.x = ( ( ( opacity * act_pix2.x ) + (( 255 - opacity ) * out_pix.x ) ) / 255 );
		out_pix.y = ( ( ( opacity * act_pix2.y ) + (( 255 - opacity ) * out_pix.y) ) / 255 );
		out_pix.z = ( ( ( opacity * act_pix2.z ) + (( 255 - opacity ) * out_pix.z) ) / 255 );
		out_pix.w = 0;
	}

	/* tmp = act_pix; */
	write_imageui( dst_img, (int2)(x,y), (int4)out_pix );

		/* funny washing effect
		int random = red % (randomness * 2);
		random = ( randomness - random );
		int vert_translation = random;
		int hor_translation = random;
		*/

}

__kernel void test3DImage(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global int gLoopCnt ){
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);
	
    __local int loopCnt;
    __local int frames_cnt = get_image_dim( src_img ).z;
 
    if ( 0 == get_local_id( 0 ) && 0 == get_global_id(1)){
    	loopCnt = gLoopCnt;
	}
 
   barrier(CLK_LOCAL_MEM_FENCE);
 
 
  int start_index = loopCnt % frames_cnt;
 
  int4 act_pix;
  act_pix = read_imageui( src_img, sampler, (int4)(x,y,start_index,0) );
 
  write_imageui( dst_img, (int2)(x,y), act_pix);
}