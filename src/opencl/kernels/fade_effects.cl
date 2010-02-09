const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;

/**
* testing kernel
*/

__kernel void testFade(__read_only image3d_t src_img, __write_only image2d_t dst_img, __global float percentage ){
  __private size_t x = get_global_id(0);
  __private size_t y = get_global_id(1);

  int4 act_pix;
  act_pix = read_imageui( src_img, sampler, (int4)(x,y,0,0) );

  write_imageui( dst_img, (int2)(x,y), act_pix);
}