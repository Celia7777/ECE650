#ifndef POTATO
#define POTATO

#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <string.h>

class Potato {
 public:
  int hops_num;
  int path_idx;
  int path[512];
  Potato() : hops_num(0), path_idx(0) { 
      memset(path, 0, sizeof(path)); 
      }

  void printPath(){
      for(int i = 0; i < path_idx - 1; i++){
          printf("%d,", path[i]);
      }
      //print the last one
      printf("%d\n", path[path_idx - 1]);
  }  
};

#endif