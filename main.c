#include <stdio.h>
#include <signal.h>
#include <unistd.h>

sigset_t set_of_signals;

static char * const mount_command[] = {"/bin/mount","-t", "tmpfs", "run", "/run",NULL};

static void mount();

static void mount(){
  int result;
  if(fork() == 0){

		sigprocmask(SIG_UNBLOCK, &set_of_signals, NULL);
		setsid();
		result = execvp(mount_command[0], mount_command);
    if(result == -1){
      printf("Can't execvp\n");
    }
		perror("execvp");
    _exit(1);
  }  
}

int main(){
  printf("Pavon Init\n");

  if(getpid() != 1){
    printf("Need to be PID 1\n");
    _exit(1);
  }

  chdir("/");

  sigfillset(&set_of_signals);
  sigprocmask(SIG_BLOCK, &set_of_signals, NULL);

  
  mount();

  printf("executed mount\n");
  while(1){}

  return 0;
}
