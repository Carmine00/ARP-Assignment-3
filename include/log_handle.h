#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
// library created to write on the log file of the processes


void file_logE(char *my_log,char *msg){ // function to write on the log file if an error occurred
  
  FILE *fp; // file pointer to the log

  // variables to handle time
  char dt[20];
  struct tm tm;
  time_t current_time;

  current_time = time(NULL);
  tm = *localtime(&current_time);
  // creation of the timestamp
  strftime(dt,sizeof(dt), "%d/%m/%Y %H:%M:%S", &tm);

  // opening of the log file and in case of error exit -5
  fp = fopen(my_log,"a");
  if (fp == NULL)
    exit(-5);
  else{
      /*
      print in the log file a message (ex: function where the program failed) set by the caller of the function,
      the specific error and its code and the timestamp
      */
      fprintf(fp,"%s:%s (%d), %s\n", msg,strerror(errno), errno, dt);
      fclose(fp);
      exit(errno);
  }
}

void file_logS(char *my_log, int val){ // function to write on the log file if a termination signal
  
  FILE *fp; // file pointer to the log

  // variables to handle time
  char dt[20];
  struct tm tm;
  time_t current_time;

  current_time = time(NULL);
  tm = *localtime(&current_time);
  strftime(dt,sizeof(dt), "%d/%m/%Y %H:%M:%S", &tm);

  fp = fopen(my_log,"a");
  if (fp == NULL)
    exit(-5);
  else{
      fprintf(fp,"Received signal %d, %s\n", val, dt); 
      fclose(fp);
      exit(0); 
  }
}

void file_logG(char *my_log, char *msg){ // function to write on the log file to state what the process is doing
  
  FILE *fp; // file pointer to the log

  // variables to handle time
  char dt[20];
  struct tm tm;
  time_t current_time;

  current_time = time(NULL);
  tm = *localtime(&current_time);
  strftime(dt,sizeof(dt), "%d/%m/%Y %H:%M:%S", &tm);

  fp = fopen(my_log,"a");
  if (fp == NULL)
    exit(-5);
  else{
      fprintf(fp,"%s, %s\n", msg, dt);
      fclose(fp);
  }
}
