/****************************************************************************
 * Copyright (c) 2017 by Paula Perez Rubio                                  *
 *                                                                          *
 * This file is part of shmem_easy.                                         *
 * You can use and/or modify this code at your own risk.                    *
 *                                                                          *
 ****************************************************************************/

/**
 * @file  run_device.c 
 * @author Paula Perez Rubio <ppru@gmv.es>
 * @date 17.12.2017
 * @brief generates data for a virtual hardware device and stores them in 
 *        shared memory.  
 *   
 * */
 #include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "shm_struct.h"

#define BIT_RES 10
#define INTERVAL_MICRO 100000
#define INTERVAL 0.1
#define CMD_LEN 20 


typedef  int (*wavefun_ptr[])(double, double);

/**
 * @brief sinusoidal signal of period T and 
 * @param[in] t time 
 * @param[in] T period of the signal 
 * @return digital signal at time t. 
 * */
inline int sin_ws(double t, double T) {
  return ((int)((1<<(BIT_RES-1))* sin(2*M_PI*t/T)));
}

/**
 * @brief Triangular signal of period T and 
 * @param[in] t time 
 * @param[in] T period of the signal 
 * @return digital signal at time t. 
 * */
inline int tri_ws(double t, double T) {
  double  t_inT = t - (int)(t/T)*T;
  double res; 
  if (t_inT < T/4.0) {
     res =  4.0 / T * t_inT;
  } else if (t_inT < 3.0*T/4.0) {
     res =  1 - 4.0 / T * (t_inT - T/4.0); 
  } else {
     res =  -1 + 4.0 / T * (t_inT - 3.0 * T / 4.0);
  } 
  return (int)((1<<(BIT_RES -1))*res); 
}

/**
 * @brief square signal of period T and 
 * @param[in] t time 
 * @param[in] T period of the signal 
 * @return digital signal at time t. 
 * */
inline int quad_ws(double t, double T) {
  double  t_inT = t - (int)(t/T)*T;
  double res;
  if (t_inT < T/2) {
     res = 1;
  } else  {
     res = -1;
  }
  return (int)((1<<(BIT_RES -1))*res); 
}

Twtype get_wave_type( char *signal) {
  if (!strncmp(signal, "SIN", CMD_LEN)) {
     return sine;
  } else if (!strncmp(signal, "TRI", CMD_LEN)) {
     return tri;  
  } else if (!strncmp(signal, "QUA", CMD_LEN)){
     return qua; 
  } 
  return none;
}

wavefun_ptr sgtype_ptr = {&sin_ws, &tri_ws, &quad_ws};

int  main() {
 int    ShmID; 
 key_t  ShmKEY; 
 Wave   *ShmPTR; 
 ShmKEY = ftok(".",'x');
 ShmID = shmget(ShmKEY, sizeof(Wave), 0666);
 if (ShmID < 0) {
   fprintf(stderr, 
       "shmget (client) encountered an error. Exiting program\n");
   exit(EXIT_FAILURE);
 }
 ShmPTR = (Wave *) shmat( ShmID, NULL, 0);
 if ( (long int) ShmPTR == -1) {
   fprintf(stderr, "shmmat (client)  encountered an error. Exiting program\n");
   exit(EXIT_FAILURE);
 }
 while (ShmPTR -> state != off ) {
    if (ShmPTR -> state == on) {
         ShmPTR -> value = (*sgtype_ptr[ShmPTR -> wtype])(ShmPTR -> time, 
                           ShmPTR -> period);
         ShmPTR -> time += INTERVAL;
         usleep(INTERVAL_MICRO);
    }
 }
 if (ShmPTR != NULL)
    shmdt((void *) ShmPTR);
 return 0; 
}
