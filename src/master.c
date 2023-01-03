#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "./../include/log_handle.h"
#define LEN 10
#define my_log "./logs/master_log.txt"

void sig_handler(int signo){

   if(signo == SIGINT){ // termination signal for the master

            // Getting the pid of child processes
            char line1[LEN], line2[LEN];
            // pid1 for processA, pid2 for processB
            long pid1 = 0, pid2 = 0;
            // pointers to the processes
            FILE *cmd1, *cmd2;
            cmd1 = popen("pidof -s processA", "r");
            cmd2 = popen("pidof -s processB", "r");
            // retrieve the pid numbers
            fgets(line1, LEN, cmd1);
            fgets(line2, LEN, cmd2);

            pid1 = strtoul(line1, NULL, 10);
            pid2 = strtoul(line2, NULL, 10);

            // Send signal to processA
            kill(pid1, SIGTERM);
            file_logG(my_log,"sent SIGTERM to processA");
            // Send signal to processB
            kill(pid2, SIGTERM);
            file_logG(my_log,"sent SIGTERM to processB");

            file_logS(my_log,signo);
        } 
}


int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}

int main() {

  file_logG(my_log,"Program started...");

  // setup to receive SIGINT
  signal(SIGINT, sig_handler);

  char * arg_list_A[] = { "/usr/bin/konsole", "-e", "./bin/processA", NULL };
  char * arg_list_B[] = { "/usr/bin/konsole", "-e", "./bin/processB", NULL };

  pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_A);
  pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

  int status;
  waitpid(pid_procA, &status, 0);
  waitpid(pid_procB, &status, 0);
  
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

