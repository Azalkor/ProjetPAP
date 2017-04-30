
#include "compute.h"
#include "graphics.h"
#include "debug.h"
#include "ocl.h"

#include <stdbool.h>

unsigned version = 0;

void first_touch_v1(void);
void first_touch_v2(void);

unsigned compute_v0(unsigned nb_iter);
unsigned compute_v1(unsigned nb_iter);
unsigned compute_v2(unsigned nb_iter);
unsigned compute_v3(unsigned nb_iter);
unsigned compute_v0_openmpfor(unsigned nb_iter);
unsigned compute_v1_openmpfor(unsigned nb_iter);
unsigned compute_v2_openmpfor(unsigned nb_iter);
unsigned compute_v1_openmptask(unsigned nb_iter);

void_func_t first_touch [] = {
  NULL,
  NULL,//first_touch_v1,
  NULL,//first_touch_v2,
  NULL,
  NULL,
  NULL,
  NULL,
};

int_func_t compute [] = {
  compute_v0,
  compute_v1,
  compute_v2,
  compute_v0_openmpfor,
  compute_v1_openmpfor,
  compute_v2_openmpfor,
  compute_v1_openmptask,
};  //  compute_v3,

char *version_name [] = {
  "Séquentielle de base",
  "Séquentielle tuilée",
  "Séquentielle tuilée (optimisée)",
  "Séquentielle de base avec OpenMP for",
  "Séquentielle tuilée avec OpenMP for",
  "Séquentielle tuilée (optimisée) avec OpenMP for",
  "Séquentielle tuilée avec OpenMP task",
};//  "OpenMP",
//  "OpenMP zone",
//  "OpenCL",


unsigned opencl_used [] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

///////////////////////////// Version séquentielle simple


unsigned const colorViv = 0xFFFF00FF;
unsigned const colorMort = 0x0;

int countAlive(int x, int y);
void majImg(int i, int j, int nbVoisinsVivants);
void majTab();
void initTabsTiles();
void task_v1(int i);

unsigned compute_v0 (unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
    for(unsigned i = 0; i < DIM; i++){
      for(unsigned j = 0; j < DIM; j++){
	majImg(i, j, countAlive(i, j));
      }
    }
    swap_images();
  }
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
unsigned sizeTiles = 32;
#define NBTILES  DIM / sizeTiles

unsigned compute_v1(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
    for(int i = 0; i < DIM; i+=sizeTiles){
      for(int j = 0; j < DIM; j+=sizeTiles){
	for(int x = i; x < i + sizeTiles; x++){
	  for(int y = i; y < j + sizeTiles; y++){
	    majImg(x, y, countAlive(x, y));
	  }
	}
      }
    }
    swap_images();
  }
  return 0;
}

unsigned compute_v1_openmpfor(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
#pragma omp parallel for collapse(2)
    for(int i = 0; i < DIM; i+=sizeTiles){
      for(int j = 0; j < DIM; j+=sizeTiles){
	for(int x = i; x < i + sizeTiles; x++){
	  for(int y = i; y < j + sizeTiles; y++){
	    majImg(x, y, countAlive(x, y));
	  }
	}
      }
    }
    swap_images();
  }
  return 0;
}

unsigned compute_v1_openmptask(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
#pragma omp parallel
#pragma omp single
    for(int i = 0; i < DIM; i+=sizeTiles){
#pragma omp task
      task_v1(i);
    }
    swap_images();
  }
  return 0;
}

void task_v1(int i){
  for(int j = 0; j < DIM; j+=sizeTiles){
    for(int x = i; x < i + sizeTiles; x++){
      for(int y = i; y < j + sizeTiles; y++){
	majImg(i, j, countAlive(i, j));
      }
    }
  }
}


///////////////////////////// Version OpenMP optimisée

void first_touch_v2 ()
{

}


int** tabTilesToMove;
int** tabTilesHaveMoved;
// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v2(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
    initTabsTiles();
    for(int i = 0; i < DIM; i+=sizeTiles){
      for(int j = 0; j < DIM; j+=sizeTiles){
	if(tabTilesToMove[i/NBTILES][j/NBTILES]){
	  tabTilesHaveMoved[i/NBTILES][j/NBTILES] = 0;
	  for(int x = i; x < i + sizeTiles; x++){
	    for(int y = i; y < j + sizeTiles; y++){
	      majImg(x, y, countAlive(x, y));
	    }
	  }
	  if(cur_img(i, j) != next_img(i, j)){
	    tabTilesHaveMoved[i/NBTILES][j/NBTILES] = 1;
	  }
	}
      }
    }
    majTab();
    swap_images();
  }
  return 0; // on ne s'arrête jamais
}

unsigned compute_v2_openmpfor(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
    initTabsTiles();
#pragma omp parallel for 
    for(int i = 0; i < DIM; i+=sizeTiles){
      for(int j = 0; j < DIM; j+=sizeTiles){
	if(tabTilesToMove[i/NBTILES][j/NBTILES]){
	  tabTilesHaveMoved[i/NBTILES][j/NBTILES] = 0;
	  for(int x = i; x < i + sizeTiles; x++){
	    for(int y = i; y < j + sizeTiles; y++){
	      majImg(x, y, countAlive(x, y));
	    }
	}
	  if(cur_img(i, j) != next_img(i, j)){
	  tabTilesHaveMoved[i/NBTILES][j/NBTILES] = 1;
	  }
	}
      }
    }
    majTab();
    swap_images();
  }
  return 0; // on ne s'arrête jamais
}


///////////////////////////// Version OpenCL

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned compute_v3 (unsigned nb_iter)
{
  return ocl_compute (nb_iter);
}

unsigned compute_v0_openmpfor(unsigned nb_iter){
  for (unsigned it = 1; it <= nb_iter; it++) {
#pragma omp for
    for(unsigned i = 0; i < DIM; i++){
      for(unsigned j = 0; j < DIM; j++){
	majImg(i, j, countAlive(i, j));
      }
    }
    swap_images();
  }
  return 0;
}

void initTabsTiles(){
  tabTilesToMove = malloc(sizeof(int*) * NBTILES);
  tabTilesHaveMoved = malloc(sizeof(int*) * NBTILES);
  for(int i = 0; i < NBTILES; i++){
    tabTilesToMove[i] = malloc(sizeof(int) * NBTILES);
    tabTilesHaveMoved[i] = malloc(sizeof(int) * NBTILES);
    for(int j = 0; j < NBTILES; j++){
      tabTilesToMove[i][j] = 1;
      tabTilesHaveMoved[i][j] = 0;
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
void majImg(int i, int j, int nbVoisinsVivants){
  if(cur_img(i, j) == colorViv){
    if(nbVoisinsVivants == 2 || nbVoisinsVivants == 3){
      next_img(i, j) = colorViv;
    }
    else{
      next_img(i, j) = colorMort;
    }
  }
  else{
    if(nbVoisinsVivants == 3){
      next_img(i, j) = colorViv;
    }
    else{
      next_img(i, j) = colorMort;
    }
  }  
}

int max(int a, int b){
  if(a > b){
    return a;
  }
  return b;
}


int min(int a, int b){
  if(a < b){
    return a;
  }
  return b;
}

void majTab()
{
  for(int x =0;x<NBTILES;x++){
    for (int y = 0; y<NBTILES;y++){
      tabTilesToMove[x][y]=0;
      for(int i = max(x-1, 0); i <= min(x+1, NBTILES-1); i++){
        for(int j = max(y-1, 0); j <= min(y+1, NBTILES-1); j++){
	  tabTilesToMove[x][y] = tabTilesHaveMoved[i][j];
        }
      }
    }
  }
}
