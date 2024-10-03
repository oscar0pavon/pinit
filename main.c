#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

sigset_t set_of_signals;

static char * const mount_command[] = {"/bin/mount","-t", "tmpfs", "run", "/run",NULL};
static char * const mount_proc_command[] = {"/bin/mount","-v",
  "-o", "nosuid,noexec,nodev", "-t", "proc", "proc", "/proc",NULL};
static char * const mount_sys_command[] = {"/bin/mount","-v",
  "-o", "nosuid,noexec,nodev", "-t", "sysfs", "sys", "/sys",NULL};
static char * const mount_dev_command[] = {"/bin/mount","-v",
  "-o", "mode=0775,nosuid", "-t", "devtmpfs", "dev", "/dev",NULL};


static char * const agetty_command[] = {"/sbin/agetty","--noclear", "--autologin", "root",
  "tty1", "9600",NULL};

static void mount();

static void launch_agetty(){
  int result;
  if(fork() == 0){

		sigprocmask(SIG_UNBLOCK, &set_of_signals, NULL);
		setsid();
		result = execvp(agetty_command[0], agetty_command);
    if(result == -1){
      printf("Can't execvp\n");
    }
		perror("execvp");
    _exit(1);
  }  
}

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

void* mount_proc(void*){
   
  if(fork() == 0){
		setsid();
		execvp(mount_proc_command[0],mount_proc_command );
  }  

  return NULL;
}

void* mount_sys(void*){
   
  if(fork() == 0){
		setsid();
		execvp(mount_sys_command[0],mount_sys_command );
  }  

  return NULL;
}

void* mount_dev(void*){
  if(fork() == 0){
		setsid();
		execvp(mount_dev_command[0],mount_dev_command );
  }  
  return NULL;
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
  
  pthread_t mount_thread;

  pthread_create(&mount_thread, NULL , mount_proc, NULL) ;

  pthread_create(&mount_thread, NULL , mount_sys, NULL) ;

  pthread_create(&mount_thread, NULL , mount_dev, NULL) ;
  //mount();

  launch_agetty();
  //printf("executed mount\n");
  int signal;
  while(1){
    alarm(30) ;
    sigwait(&set_of_signals,&signal);
  }

  return 0;
}
