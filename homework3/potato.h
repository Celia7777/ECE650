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
      for(int i = 0; i < path_idx; i++){
          std::cout << path[i];
          if(i != path_idx - 1){
              std::cout<<",";
          }
          else{
              std::cout<<std::endl;
          }
      }
      //std::cout<<"path index: "<<path_idx<<std::endl;
  }  
};

#endif