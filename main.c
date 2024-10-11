#include <complex.h>
#include <pthread.h>
#define _XOPEN_SOURCE 200809L
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/swap.h>

#include <string.h>
#define NULL 0
#define TIMEO	30

sigset_t set_of_signals;

int symlink(const char *target, const char *linkpath);

static char * const ls_command[] = {"/bin/ls",NULL};


static char * const mount_sys_commnad[] = {"sysfs","/sys", "sysfs"};
static char * const mount_proc_commnad[] = {"proc","/proc", "proc"};
static char * const mount_dev_commnad[] = {"dev","/dev", "devtmpfs"};
static char * const mount_pts_commnad[] = {"devpts","/dev/pts", "devpts"};

static char * const mount_boot_commnad[] = {"/dev/nvme0n1p1","/boot", "vfat"};

static char * const mount_shm_commnad[] = {"tmpfs","/dev/shm", "tmpfs"};

static char * const mount_shm_command_mount[] = {"/bin/mount", "tmpfs","/dev/shm",
  "-t", "tmpfs","-o", "nosuid,nodev", NULL};


static char * const agetty_command[] = {"/sbin/agetty","--noclear", "--autologin", "root",
  "tty1", "9600",NULL};

static char * const pts_command[] = {"/bin/mount","-n", "-t" , "devpts", 
  "-o", "gid=5,mode=0620", "devpts", "/dev/pts", NULL};

//wifi
#define DEV "wlan0"

static char * const ip_set_up_command[] = {"/sbin/ip","link", "set" ,DEV, 
  "up", NULL};
static char * const wpa_command[] = {"/sbin/wpa_supplicant","-B", "-c" , "/wifi", 
  "-i", DEV, NULL};
static char * const ip_addr_command[] = {"/sbin/ip","addr", "add" , "192.168.0.23/24", 
  "dev",DEV, NULL};
static char * const ip_route_command[] = {"/sbin/ip","route", "add","default","via",
  "192.168.0.1", "src", "192.168.0.23","dev" , DEV, NULL};

void wait_signal_for_close(int pid){
    
	  waitpid(pid, NULL, WNOHANG);
    pthread_exit(0);
    
}

void* execute_thread_command(void*command_line){
  char* const* command = (char* const *)(command_line);
  int pid; 
  if((pid = fork())){
    wait_signal_for_close(pid);
  }else{
		setsid();
		execvp(*command,command_line);
  } 
  return NULL;
}

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
  }
}


struct MountCommand{
  char * const *arguments;
  unsigned long int mode; 
};

void* mount_threaded(void*command_line){
  struct MountCommand* mount_command = (struct MountCommand*)command_line;

  char* const* command = (char* const *)(mount_command->arguments);

  mount(command[0], command[1], command[2],
      mount_command->mode,NULL);

  return NULL;
}



void* set_ip(void*){

  int signal;
  int pid; 
  if((pid = fork())){
    
    while(1){
      sigwait(&set_of_signals,&signal);
      if(signal == SIGCHLD){
	      waitpid(pid, NULL, WNOHANG);
        pthread_t thread;
        pthread_create(&thread, NULL , execute_thread_command, ip_route_command) ;

        break;
      }
    }
  }else{
		sigprocmask(SIG_UNBLOCK, &set_of_signals, NULL);
		setsid();
		execvp(ip_addr_command[0],ip_addr_command);
  }  
}


static void signal_reap(void)
{
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
	alarm(TIMEO);
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
  pthread_t mount_dev_thread;

  
  struct MountCommand mount_dev_struct; 
  mount_dev_struct.arguments = mount_dev_commnad;
  mount_dev_struct.mode = MS_NOSUID;

  pthread_create(&mount_dev_thread, NULL , mount_threaded, &mount_dev_struct);
  
  struct MountCommand mount_proc_struct = {.arguments = mount_proc_commnad, 
                                          .mode = MS_NOSUID | MS_NOEXEC | MS_NODEV}; 
  
  pthread_create(&mount_thread, NULL , mount_threaded, &mount_proc_struct);
  
  struct MountCommand mount_sys_struct = {.arguments = mount_sys_commnad, 
                                          .mode = MS_NOSUID | MS_NOEXEC | MS_NODEV}; 

  pthread_create(&mount_thread, NULL , mount_threaded, &mount_sys_struct);


  pthread_join(mount_dev_thread,NULL);
  

  struct MountCommand mount_boot_struct = {.arguments = mount_boot_commnad, 
                                          .mode = 0}; 

  pthread_create(&mount_thread, NULL , mount_threaded, &mount_boot_struct);


  symlink("/proc/self/fd/0","/dev/stdin");
  symlink("/proc/self/fd/1","/dev/stdout");
  symlink("/proc/self/fd/2","/dev/stderr");
  symlink("/proc/self/fd","/dev/fd");


 
  mkdir("/dev/pts", S_IRWXU | S_IRWXG | S_IRWXO);
  mkdir("/dev/shm", S_IRWXU | S_IRWXG | S_IRWXO);
  

  struct MountCommand mount_pts_struct = {.arguments = mount_pts_commnad, 
                                          .mode = 0}; 
  //need chmod 0666 /dev/pts/ptmx
  pthread_create(&mount_thread, NULL , mount_threaded, &mount_pts_struct);

  struct MountCommand mount_shm_struct = {.arguments = mount_shm_commnad, 
                                         .mode = MS_NOSUID | MS_NODEV}; 
  
  pthread_create(&mount_thread, NULL , mount_threaded, &mount_shm_struct);

 // pthread_create(&mount_thread, NULL , execute_thread_command, mount_shm_command_mount) ;


  //wifi
  pthread_t ip_add_thread;
  pthread_create(&mount_thread, NULL , execute_thread_command, ip_set_up_command) ;
  pthread_create(&mount_thread, NULL , execute_thread_command, wpa_command) ;
  
  set_ip(NULL);

  swapon("/dev/nvme0n1p4", SWAP_FLAG_DISCARD);

  launch_agetty();
  
  int signal;
  while(1){
    alarm(30) ;
    sigwait(&set_of_signals,&signal);
    if(signal == SIGCHLD || signal == SIGALRM){
      signal_reap();
    }
    
  }

  return 0;
}
