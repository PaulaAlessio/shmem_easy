/****************************************************************************
 * Copyright (c) 2017 by Paula Perez Rubio                                  *
 *                                                                          *
 * This file is part of shmem_easy.                                         *
 * You can use and/or modify this code at your own risk.                    *
 *                                                                          *
 ****************************************************************************/

/**
 * @file  client.c 
 * @author Paula Perez Rubio <ppru@gmv.es>
 * @date 17.12.2017
 * @brief Read data in shared memory from a virtual hardware device
 *   
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include "shm_struct.h"

#define R_INTERVAL 1

int main () {
 
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
 while (ShmPTR -> state != on ) {};
 printf("Data found: \n");
 while (ShmPTR -> state != off) {
     printf("Value is Period = %f, time = %f , value =  %d \n", 
     ShmPTR -> period, ShmPTR -> time, ShmPTR -> value);
     sleep (R_INTERVAL);
 }
 printf("Device is being shut down.\n");
 if (ShmPTR !=  NULL)
   shmdt((void *) ShmPTR);
 return 0; 
}
