
/****************************************************************************
 * Copyright (c) 2017 by Paula Perez Rubio                                  *
 *                                                                          *
 * This file is part of shmem_easy.                                         *
 * You can use and/or modify this code at your own risk.                    *
 *                                                                          *
 ****************************************************************************/

/**
 * @file  device.c 
 * @author Paula Perez Rubio <ppru@gmv.es>
 * @date 17.12.2017
 * @brief Virtual hardware device
 *   
 * Configures a virtual hardware that generates a wave function
 * and stores the data in shared memory. Capable of starting, stopping
 * reconfiguring and shutting down the hardware 
 * */
 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "shm_struct.h"

#define CMD_LEN 20 

void usage();
void sighandler(); 
Twtype get_wave_type( char *signal);
void print_state(Wave *ShmPTR);
void config_device(Wave *ShmPTR);

/**
 * @brief configure/start/stop/shutdown a hardware device
 * */
int  main(int argc, char *argv[]) {
  int    ShmID; 
  key_t  ShmKEY; 
  Wave   *ShmPTR; 
  Twtype  in_wtype; 
  double  in_period; 
  signal(SIGINT, sighandler);
  if (argc != 3) {
    usage(); 
    exit(EXIT_FAILURE);
  }
  /* Read input arguments */
  in_period = atof(argv[2]);
  if ((in_wtype = get_wave_type(argv[1])) == none) {
     fprintf(stderr, "First arg hast to be one of: {SIN, TRI, QUA}\n");
     usage();
     exit(EXIT_FAILURE);
  }
  ShmKEY = ftok(".",'x');
  if (ShmKEY == -1  ) {
    fprintf(stderr, "ftok (server) encountered an error. Exiting program\n");
    exit(EXIT_FAILURE);
  }

  ShmID = shmget(ShmKEY, sizeof(Wave),  IPC_CREAT | 0666);
  if (ShmID < 0) {
    fprintf(stderr, "shmget (server) encountered an error. Exiting program\n");
    fprintf(stderr, "Error message %s \n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  ShmPTR = (Wave *) shmat( ShmID, NULL, 0);
  if ( (long int) ShmPTR == -1) {
    fprintf(stderr, "shmmat (server) encountered an error. Exiting program\n");
    exit(EXIT_FAILURE);
  }

  printf("Starting device, configuring...\n");
  ShmPTR -> state = initializing;
  ShmPTR -> wtype = in_wtype;
  ShmPTR -> value = 0.0;
  ShmPTR -> period = in_period; 
  ShmPTR -> time = 0.0;
  ShmPTR -> state = on;
  print_state(ShmPTR); 
  system("./run_device &");
  printf("Device started . Type: {exit, start, off, config, stop} to ");
  printf("act on the device\n");
  printf("HW_WAVE> ");
  char *command = calloc(CMD_LEN, 1); 
  strncpy(command, "start\n", CMD_LEN);
  fgets(command, CMD_LEN, stdin);
  while (ShmPTR -> state != off )  {
      /* Accepting commands */ 
      if (!strncmp(command, "start\n", CMD_LEN)) {
         ShmPTR -> state = on; 
         printf("Restarting device\n");
         printf("HW_WAVE> ");
         fgets(command, CMD_LEN, stdin);
      } else if (!strncmp(command, "stop\n", CMD_LEN)) {
         ShmPTR -> state = in_pause;
         printf("Stopping device\n");
         memset(command, 0, CMD_LEN);
         printf("HW_WAVE> ");
         fgets(command, CMD_LEN, stdin);
      } else if (!strncmp(command, "config\n", CMD_LEN)) {
         ShmPTR -> state = config;
         config_device(ShmPTR);
         printf("The device will restart running... \n"); 
         ShmPTR -> state = on; 
         memset(command, 0, CMD_LEN);
         printf("HW_WAVE> ");
         fgets(command, CMD_LEN, stdin);
      } else if (!strncmp(command, "exit\n", CMD_LEN)) {
         printf("Shutting device down:\n");
         printf("  Detaching the device ...\n");
         ShmPTR -> state = off; 
         printf("  Letting other processes detach ... \n");
         usleep(100000);
      } else if (!strncmp(command,"\n",CMD_LEN)) {
         memset(command, 0, CMD_LEN);
         printf("\rHW_WAVE> ");
         fgets(command, CMD_LEN, stdin);
      } else {
         printf("Unrecognized command:\n");
         printf("Command options: {exit, start, off, config, stop}\n");
         memset(command, 0, CMD_LEN);
         printf("HW_WAVE> ");
         fgets(command, CMD_LEN, stdin);
      } 
  }
  shmdt(ShmPTR);
  shmctl(ShmID, IPC_RMID, NULL);
  printf("Device is off, exiting program \n");
  return 0; 
}


/*----------------------------------------------------------------------------*/
/*                                FUNCTIONS                                   */
/*----------------------------------------------------------------------------*/

/**
 * @brief describes the usage 
 * @description ./device <Type> <Period>
 * */ 
void usage() {
  char message[] = "Usage: ./device <Type> <Period> \n"
                   "   Type (string): SIN (sinusoidal), TRI (triangular),\n" 
                   "                  QUA (quadratic).\n" 
                   "   Period (double): period of the signal.\n";

  fprintf(stderr, "%s \n", message);
}

/**
 * @brief Avoids using ctrl-c to exit the program
 * */
void sighandler() {
  fprintf(stdout ,"Type exit to exit!\nHW_WAVE> ");
  fflush(stdout); 
}

/**
 * @brief obtains enum value for wave type 
 * */
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

/**
 * @brief print state of the device.
 * @param[in] pointer to shared memory structure.
 * */
void print_state(Wave *ShmPTR) {
  printf("\t\t DEVICE STATE \n");
  printf(" -----------------------------------------\n");
  printf(" * Type of wave: ");
  switch (ShmPTR -> wtype){
    case sine:
      printf("sinusoidal\n");
      break;
    case tri:
      printf("triangular\n");
      break;
    case qua:
      printf("quadratic\n");
      break;
    default:
      printf("undefined\n"); 
      break;
  }
  printf(" * Period: %f s\n", ShmPTR -> period);
  printf(" * Starting at time: %f \n", ShmPTR -> time);
  printf(" ----------------------------------------\n");
}

/** 
 * @brief manual device configuration.
 * @param[in/out] pointer to shared memory structure. 
 * @description  A dialog is opened. The user is asked to introduce the 
 *               device parameters.
 *
 * */
void config_device(Wave *ShmPTR) {
  char *input_v = calloc(CMD_LEN,1);
  printf("Reconfiguring the device. Please introduce the values:\n");
  while (1) {
    printf(" - Wave signal {SIN, TRI, QUA}: ");
    fgets(input_v,CMD_LEN, stdin);
    strtok(input_v, "\n");
    if ((ShmPTR -> wtype = get_wave_type(input_v)) == none) {
      printf("   ERROR: Wave signal parameter not recognized.\n"); 
    } else {
      break;
    }
  }
  while (1) {
    printf(" - Do you want to reset the time?  yes/no: ");
    fgets(input_v,CMD_LEN, stdin);
    if (!strncmp(input_v,"yes\n",CMD_LEN)) {
      ShmPTR -> time = 0.0;
      break; 
    } else if (!strncmp(input_v, "no\n", CMD_LEN)) {
      break;     
    } else {
      printf("  ERROR: Please introduce values: {yes, no}\n");
    }  
  }
  printf(" - Period (in sec): ");
  fgets(input_v,CMD_LEN, stdin);
  strtok(input_v,"\n");
  ShmPTR -> period = atof(input_v);
  printf("Configuration finished.\n");
  print_state(ShmPTR);
}


