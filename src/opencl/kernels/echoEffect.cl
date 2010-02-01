const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |	CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

/*__kernel void echoeffect(__read_only image3d_t src_img, __global uchar4* dst_img){*/
__kernel void echoeffect(__read_only image3d_t src_img, __global uchar4* dst_img, __global int gRandomness, __global int gOpacity){

	__private size_t x = get_global_id(0);
	__private size_t y = get_global_id(1);
	__local int randomness;
	__local int opacity;

	/* opacity of the overlayed frames, 102 = 40%*/
	/* fetch to shared memory with barrier() */
	if ( get_local_id( 0 ) == 0 ) {
		randomness  = gRandomness;
		opacity = gOpacity;
	}

	/* random generator */
	/* fetch randomness from shared memory */
	


	int4 act_pix;
	act_pix = read_imageui( src_img, sampler, (int4)(x,y,5,0) );
	uchar red = act_pix.x;
	uchar yellow = act_pix.y;
	uchar blue = act_pix.z;

	/* overlaying with "manual loop unrolling" */
	/* overlay second frame */
	for (int i = 0; i<=4; i++) {

		/* funny washing effect
		int random = red % (randomness * 2);
		random = ( randomness - random );
		int vert_translation = random;
		int hor_translation = random;
		*/

		/* random perturbatrion of the single frames*/
		int4 start_pix = read_imageui( src_img, sampler, (int4)(0,0,i,0) );
		int random = start_pix.x % (randomness * 2);
		random = ( randomness - random );
		int vert_translation = random;
		int hor_translation = random;


		act_pix = read_imageui( src_img, sampler, (int4)(x + vert_translation,y + hor_translation,i,0) );
		red = (uchar)( ( ( opacity * act_pix.x ) + (( 255 - opacity ) * red ) )/ 255 );
		yellow = (uchar)( ( ( opacity * act_pix.y ) + (( 255 - opacity ) * yellow ) ) / 255 );
		blue = (uchar)( ( ( opacity * act_pix.z ) + (( 255 - opacity ) * blue ) ) / 255 );
	}

	/* make sure, that the output-frame is in the right position */
/*	act_pix = read_imageui( src_img, sampler, (int4)(x, y, 5, 0) );
	red = (uchar)( ( ( opacity * act_pix.x ) + (( 255 - opacity ) * red ) )/ 255 );
	yellow = (uchar)( ( ( opacity * act_pix.y ) + (( 255 - opacity ) * yellow ) ) / 255 );
	blue = (uchar)( ( ( opacity * act_pix.z ) + (( 255 - opacity ) * blue ) ) / 255 );
*/
	uchar4 tmp;
	tmp.x = red;
	tmp.y = yellow;
	tmp.z = blue;

	int4 dim = get_image_dim(src_img);
	dst_img[y * dim.x + x] = tmp;
}
