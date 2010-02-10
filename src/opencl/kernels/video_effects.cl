const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;
 
/*__kernel void echoeffect(__read_only image3d_t src_img, __global uchar4* dst_img){*/
__kernel void ghosteffect(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global int gLoopCnt ){
 
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);
	/* max random aberration of the overlayed frames */
	int randomness = 30;
	/* opacity of the overlayed frames, 102 = 40%*/
	int opacity = 102;
	/* how much background-frames */
	int frames_cnt = get_image_dim( src_img ).z;
	/* wheres is the first frame */
	__local int start_address;
 

	/* fetch to shared memory with barrier() */
	if ( get_local_id( 0 ) && get_local_id(1) == 0 ) {
		int loopCnt = gLoopCnt;
		/* appoint the starting memory index */
		start_address = loopCnt % frames_cnt;
		/*randomness  = gRandomness;
		opacity = gOpacity;*/
	}
	/* synchronize threads */
	barrier(CLK_LOCAL_MEM_FENCE);

	/* front frame */
	int4 act_pix = read_imageui( src_img, sampler, (int4)(x,y,start_address,0) );
	/* output frame */
	int4 out_pix = read_imageui( src_img, sampler, (int4)(x,y,start_address,0) );

	/* temporary frames */
	int4 act_pix2;
	/* (random) translation of the background frames */
	__local int vert_translation;
	__local int hor_translation;

	/* overlaying, depending on how much frames there are in the past */
	for (int n = 1; n<(frames_cnt); n++) {
		/* get background frames in the right order */
		int i = start_address - n;
		if ( i<0)
			i = frames_cnt + i;

		/* pseudo-random generator ;-) */
		if ( get_local_id( 0 ) && get_local_id(1) == 0 ) {
			act_pix2 = read_imageui( src_img, sampler, (int4)(i,i,i,0) );
			/* pseudo-random perturbation of the single frames*/
			int random = act_pix2.x % (randomness * 2);
			vert_translation = (randomness - random);
			random = act_pix2.y % (randomness * 2);
			hor_translation = ( randomness - random);
		}
		barrier(CLK_LOCAL_MEM_FENCE);

		act_pix2 = read_imageui( src_img, sampler, (int4)(vert_translation+x,hor_translation+y,i,0) );
		
		/* transparency overlay */
/*		out_pix.y = ( ( ( opacity * act_pix2.x ) + (( 255 - opacity ) * out_pix.x) ) / 255 );
		out_pix.y = ( ( ( opacity * act_pix2.y ) + (( 255 - opacity ) * out_pix.y) ) / 255 );
		out_pix.z = ( ( ( opacity * act_pix2.z ) + (( 255 - opacity ) * out_pix.z) ) / 255 );*/
		out_pix.x = ( ( mul24( opacity, act_pix2.x ) +  mul24(( 255 - opacity ), out_pix.x) ) / 255 );
		out_pix.y = ( ( mul24( opacity, act_pix2.y ) +  mul24(( 255 - opacity ), out_pix.y) ) / 255 );
		out_pix.z = ( ( mul24( opacity, act_pix2.z ) +  mul24(( 255 - opacity ), out_pix.z) ) / 255 );
	}
	/* once again, the first frame, to make sure, it is visible enough */
	out_pix.x = ( ( mul24( 75, act_pix.x ) + mul24(( 255 - 75), out_pix.x) ) / 255 );
	out_pix.y = ( ( mul24( 75, act_pix.y ) + mul24(( 255 - 75), out_pix.y) ) / 255 );
	out_pix.z = ( ( mul24( 75, act_pix.z ) + mul24(( 255 - 75), out_pix.z) ) / 255 );

	write_imageui( dst_img, (int2)(x,y), (int4)out_pix );
}





__kernel void echoblureffect(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global int gLoopCnt ){
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);
	/* max random aberration of the overlayed frames */
	int randomness = 20;
	/* opacity of the overlayed frames, 102 = 40%*/
	int opacity = 102;
	/* how much background-frames */
	int frames_cnt = get_image_dim( src_img ).z;
	/* wheres is the first frame */
	__local int start_address;
 
	/* fetch to shared memory with barrier() */
	if ( get_local_id( 0 ) & get_local_id(1) == 0 ) {
		int loopCnt = gLoopCnt;
		/* appoint the starting memory index */
		start_address = loopCnt % frames_cnt;
		/*randomness  = gRandomness;
		opacity = gOpacity;*/
	}
	/* synchronize threads */
	barrier(CLK_LOCAL_MEM_FENCE);

	/* front frame */
	int4 act_pix = read_imageui( src_img, sampler, (int4)(x,y,start_address,0) );
	/* output frame */
	int4 out_pix = read_imageui( src_img, sampler, (int4)(x,y,start_address,0) );
	/* temporary frames */
	int4 act_pix2;
	/* overlaying, depending on how much frames there are in the past */
	for (int n = 1; n<(frames_cnt); n++) {
		/* get background frames in the right order */
		int i = start_address - n;
		if ( i<0)
			i = frames_cnt + i;

		/* pseudo-random generator ;-) */
		/* (random) translation of the background frames */
		int vert_translation;
		int hor_translation;
		/* pseudo-random bluring of the single frames*/
		int random = out_pix.x % (randomness * 2);
		vert_translation = (randomness - random)+x;
		random = out_pix.y % (randomness * 2);
		hor_translation = ( randomness - random)+y;

		act_pix2 = read_imageui( src_img, sampler, (int4)(vert_translation,hor_translation,i,0) );
		
		/* transparency overlay */
		out_pix.x = ( ( mul24( opacity, act_pix2.x ) +  mul24(( 255 - opacity ), out_pix.x) ) / 255 );
		out_pix.y = ( ( mul24( opacity, act_pix2.y ) +  mul24(( 255 - opacity ), out_pix.y) ) / 255 );
		out_pix.z = ( ( mul24( opacity, act_pix2.z ) +  mul24(( 255 - opacity ), out_pix.z) ) / 255 );
	}
	/* once again, the first frame, to make sure, it is visible enough */
	out_pix.x = ( ( mul24( 75, act_pix.x ) + mul24(( 255 - 75), out_pix.x) ) / 255 );
	out_pix.y = ( ( mul24( 75, act_pix.y ) + mul24(( 255 - 75), out_pix.y) ) / 255 );
	out_pix.z = ( ( mul24( 75, act_pix.z ) + mul24(( 255 - 75), out_pix.z) ) / 255 );

	write_imageui( dst_img, (int2)(x,y), (int4)out_pix );
}






/* a test-kernel for 3d-images */
__kernel void test3DImage(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global int gLoopCnt ){
	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);
	
    __local int start_index;
 
    if ( 0 == get_local_id( 0 ) && 0 == get_local_id(1)){
    	__private int loop_cnt = gLoopCnt;
    	start_index = loop_cnt % get_image_dim(src_img).z;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
 
	int4 act_pix;
	act_pix = read_imageui( src_img, sampler, (int4)(x,y,start_index,0) );
 
	write_imageui( dst_img, (int2)(x,y), act_pix);
}