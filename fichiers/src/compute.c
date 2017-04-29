
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

unsigned version = 0;

void first_touch_v1 (void);
void first_touch_v2 (void);

unsigned compute_v0 (unsigned nb_iter);
unsigned compute_v1 (unsigned nb_iter);
unsigned compute_v2 (unsigned nb_iter);
unsigned compute_v3 (unsigned nb_iter);

void_func_t first_touch [] = {
  NULL,
  first_touch_v1,
  first_touch_v2,
  NULL,
};

int_func_t compute [] = {
  compute_v0,
  compute_v1,
  compute_v2,
  compute_v3,
};

char *version_name [] = {
  "Séquentielle",
  "OpenMP",
  "OpenMP zone",
  "OpenCL",
};

unsigned opencl_used [] = {
  0,
  0,
  0,
  1,
};

///////////////////////////// Version séquentielle simple


unsigned const colorViv = 0xFFFF00FF;
unsigned const colorMort = 0x0;

int countAlive(int x, int y);
int majImg(int i, int j, int nbVoisinsVivants);

unsigned compute_v0 (unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
    for(unsigned i = 0; i < DIM; i++){
      for(unsigned j = 0; j < DIM; j++){
	majImg(i, j, countAlive(i, j));
      }
    }
  }
  swap_images();
  return 0;
}


///////////////////////////// Version OpenMP de base

void first_touch_v1 ()
{
  int i,j ;

#pragma omp parallel for
  for(i=0; i<DIM ; i++) {
    for(j=0; j < DIM ; j += 512)
      next_img (i, j) = cur_img (i, j) = 0 ;
  }
}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
const unsigned sizeTile = 32;
unsigned compute_v1(unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {
    for(int i = 0; i < DIM; i+=sizeTile){
      for(int j = 0; j < DIM; j+=sizeTile){
	for(int x = i; x < i + sizeTile; x++){
	  for(int y = i; y < j + sizeTile; y++){
	    majImg(x, y, countAlive(x, y));
	  }
	}
      }
    }
  }
  swap_images();
  return 0;
}



///////////////////////////// Version OpenMP optimisée

void first_touch_v2 ()
{

}


int** tabTiles;
// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v2(unsigned nb_iter)
{
  initTabTiles();
  for(int i = 0; i < DIM; i+=sizeTile){
    for(int j = 0; j < DIM; j+=sizeTile){
      if(tileWillChanged(i, j)){
	for(int x = i; x < i + sizeTile; x++){
	  for(int y = i; y < j + sizeTile; y++){
	    majImg(x, y, countAlive(x, y));
	  }
	}
      }
    }
  }
  return 0; // on ne s'arrête jamais
}




///////////////////////////// Version OpenCL

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v3 (unsigned nb_iter)
{
  return ocl_compute (nb_iter);
}

int tileWillChanged(int i, int j){
  for(int x = i; x < i + sizeTile; x++){
    for(int y = j; y < j + sizeTile; y++){
      if(
    }
  }
}

void initTabTiles(){
  tabTiles = malloc(sizeof(int*) * DIM);
  for(int i = 0; i < DIM; i++){
    tabTiles[i] = malloc(sizeof(int) * DIM);
    for(int j = 0; j < DIM; j++){
      tabTiles[i][j] = 1;
    }
  }
}

int countAlive(int x, int y){
  int cpt = 0;
  for(int i = x-1; i <= x+1; i++){
    for(int j = y-1; j <= y+1; j++){
      if(i >= 0 && j>= 0){
	if(cur_img(i, j) == colorViv && (i!=x || j!=y)){
	  cpt++;
	}
      }
    }
  }
  return cpt;
}

//retourne 0 si l'image n'évoluera pas, 1 sinon
int majImg(int i, int j, int nbVoisinsVivants){
  int ret = 0;
  if(cur_img(i, j) == colorViv){
    if(nbVoisinsVivants == 2 || nbVoisinsVivants == 3){
      ret = 1;
      /* if(cur_img */
      next_img(i, j) = colorViv;
    }
    else{
      ret = 1;
      next_img(i, j) = colorMort;
    }
  }
  else{
    if(nbVoisinsVivants == 3){
      ret = 1;
      next_img(i, j) = colorViv;
    }
    else{
      ret = 1;
      next_img(i, j) = colorMort;
    }
  }
  return ret;
}
