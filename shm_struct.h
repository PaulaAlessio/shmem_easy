#ifndef SHM_STRUCT_H
#define SHM_STRUCT_H


#include <stdint.h>
#include <sys/types.h>


typedef enum _state {
  off = 0,
  on, 
  initializing,
  in_pause,
  closing,
  config 
} Tstate; 

typedef enum _wtype {
  sine, 
  tri, 
  qua,
  none
} Twtype;

typedef struct _wave {
  Tstate state;
  Twtype wtype; 
  int value;
  double time;
  double period;
} Wave;

#endif
