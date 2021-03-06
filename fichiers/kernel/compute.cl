__kernel void transpose_naif (__global unsigned *in, __global unsigned *out)
{
  int x = get_global_id (0);
  int y = get_global_id (1);

  out [x * DIM + y] = in [y * DIM + x];
}



__kernel void transpose (__global unsigned *in, __global unsigned *out)
{
  __local unsigned tile [TILEX][TILEY+1];
  int x = get_global_id (0);
  int y = get_global_id (1);
  int xloc = get_local_id (0);
  int yloc = get_local_id (1);

  tile [xloc][yloc] = in [y * DIM + x];

  barrier (CLK_LOCAL_MEM_FENCE);

  out [(x - xloc + yloc) * DIM + y - yloc + xloc] = tile [yloc][xloc];
}


#define colorViv 0xFFFF00FF
#define colorMort 0x0

__kernel void ocl_naif(__global unsigned *in, __global unsigned *out){
  int x = get_global_id(0);
  int y = get_global_id(1);

  int nbAlive = 0;
  for(int i = x-1; i <= x+1; i++){
    for(int j = y-1; j <= y+1; j++){
      if(i >= 0 && j>= 0){
	if(in[j*DIM + i] == colorViv && (i!=x || j!=y)){
	  nbAlive++;
	}
      }
    }
  }
  //si la case courante est vivante
  if(in[y*DIM + x] == colorViv){
    if(nbAlive == 2 || nbAlive == 3){
      out[y*DIM + x] = colorViv;
    }
    else{
      out[y*DIM + x] = colorMort;
    }
  }
 else{
   if(nbAlive == 3){
     out[y*DIM + x] = colorViv;
   }
   else{
     out[y*DIM + x] = colorMort;
   } 
 }
}
  
// NE PAS MODIFIER
static float4 color_scatter (unsigned c)
{
  uchar4 ci;

  ci.s0123 = (*((uchar4 *) &c)).s3210;
  return convert_float4 (ci) / (float4) 255;
}

// NE PAS MODIFIER: ce noyau est appelé lorsqu'une mise à jour de la
// texture de l'image affichée est requise
__kernel void update_texture (__global unsigned *cur, __write_only image2d_t tex)
{
  int y = get_global_id (1);
  int x = get_global_id (0);
  int2 pos = (int2)(x, y);
  unsigned c;

  c = cur [y * DIM + x];

  write_imagef (tex, pos, color_scatter (c));
}
