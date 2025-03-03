#include <pthread.h>
#include <time.h>
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

#define TIMEO	30

sigset_t set_of_signals;

int symlink(const char *target, const char *linkpath);

static char * const mount_sys_commnad[] = {"sysfs","/sys", "sysfs"};
static char * const mount_proc_commnad[] = {"proc","/proc", "proc"};
static char * const mount_dev_commnad[] = {"dev","/dev", "devtmpfs"};
static char * const mount_pts_commnad[] = {"devpts","/dev/pts", "devpts"};

static char * const mount_efivars_commnad[] = {"efivarfs","/sys/firmware/efi/efivars", "efivarfs"};

static char * const mount_boot_commnad[] = {"/dev/nvme0n1p1","/boot", "vfat"};

static char * const mount_disk_commnad[] = {"/dev/nvme0n1p5","/root/disk", "ext4"};

static char * const mount_shm_commnad[] = {"tmpfs","/dev/shm", "tmpfs"};

static char * const mount_run_commnad[] = {"tmpfs","/run", "tmpfs"};

static char * const mingetty1[] = {"/bin/mingetty", "--autologin=root","tty1",NULL};

static char * const mingetty2[] = {"/bin/mingetty", "--autologin=root","tty2",NULL};


static char * const udev_script[] = {"/udev.sh",NULL};


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

static char * const ip_addr_lo_command[] = {"/sbin/ip","addr", "add" ,
  "127.0.0.1/8", "label", "lo", 
  "dev","lo", NULL};

static char* const ip_lo_up[] = {"/sbin/ip","link", 
  "set", "lo", "up",  NULL};

void wait_signal_for_close(int pid){
    
	  waitpid(pid, NULL, WNOHANG);
    pthread_exit(0);
    
}

void setup_loopback() {
  int pid;
  int signal;
  if ((pid = fork())) {
    while (1) {
      sigwait(&set_of_signals, &signal);
      if (signal == SIGCHLD) {
        waitpid(pid, NULL, WNOHANG);

        if (fork() == 0) {
          sigprocmask(SIG_UNBLOCK, &set_of_signals, NULL);
          setsid();
          execvp(ip_lo_up[0], ip_lo_up);
        }

        break;
      }
    }
  } else {
    sigprocmask(SIG_UNBLOCK, &set_of_signals, NULL);
    setsid();
    execvp(ip_addr_lo_command[0], ip_addr_lo_command);
  }
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

static void launch_program(char* const command[]){
  int result;

  if(fork() == 0){
		sigprocmask(SIG_UNBLOCK, &set_of_signals, NULL);
		setsid();
		result = execvp(command[0], command);
    if(result == -1){
      printf("Can't execvp %s\n",command[0]);
    }
		perror("execvp");
  }
}

static void launch_mingetty(const char* mingetty_exec,char* const arguments[]){
  int result;

  if(fork() == 0){
		sigprocmask(SIG_UNBLOCK, &set_of_signals, NULL);
		setsid();
		result = execvp(mingetty_exec, arguments);
    if(result == -1){
      printf("Can't execvp %s\n",mingetty1[0]);
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
  
  FILE* boot_time = fopen("boot_time","w");
  if(!boot_time){
    printf("Can't create boot file\n");
  }
  
  clock_t init_time = clock();

  if(getpid() != 1){
    printf("Need to be PID 1\n");
    _exit(1);
  }

  chdir("/");

  sigfillset(&set_of_signals);
  sigprocmask(SIG_BLOCK, &set_of_signals, NULL);
  

  pthread_t mount_thread;
  pthread_t mount_dev_thread;

  struct MountCommand mount_run_struct = {.arguments = mount_run_commnad, 
                                          .mode = 0}; 
  pthread_create(&mount_thread, NULL , mount_threaded, &mount_run_struct);

  
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
  
  struct MountCommand mount_efi_struct = {.arguments = mount_efivars_commnad, 
                                          .mode = 0}; 

  pthread_create(&mount_thread, NULL , mount_threaded, &mount_efi_struct);

  struct MountCommand mount_boot_struct = {.arguments = mount_boot_commnad, 
                                          .mode = 0}; 

  pthread_create(&mount_thread, NULL , mount_threaded, &mount_boot_struct);

  struct MountCommand mount_disk_struct = {.arguments = mount_disk_commnad, 
                                          .mode = 0}; 

  pthread_create(&mount_thread, NULL , mount_threaded, &mount_disk_struct);


  //launch_program(udev_script);

  symlink("/proc/self/fd/0","/dev/stdin");
  symlink("/proc/self/fd/1","/dev/stdout");
  symlink("/proc/self/fd/2","/dev/stderr");
  symlink("/proc/self/fd","/dev/fd");


 
  mkdir("/dev/pts", S_IRWXU | S_IRWXG | S_IRWXO);
  mkdir("/dev/shm", S_IRWXU | S_IRWXG | S_IRWXO);
  

  struct MountCommand mount_pts_struct = {.arguments = mount_pts_commnad, 
                                          .mode = 0}; 
  pthread_create(&mount_thread, NULL , mount_threaded, &mount_pts_struct);

  struct MountCommand mount_shm_struct = {.arguments = mount_shm_commnad, 
                                         .mode = MS_NOSUID | MS_NODEV}; 
  
  pthread_create(&mount_thread, NULL , mount_threaded, &mount_shm_struct);



  //wifi
  pthread_t ip_add_thread;
  pthread_create(&mount_thread, NULL , execute_thread_command, ip_set_up_command) ;
  pthread_create(&mount_thread, NULL , execute_thread_command, wpa_command) ;
  
  set_ip(NULL);

  setup_loopback();

  swapon("/dev/nvme0n1p4", SWAP_FLAG_DISCARD);

  launch_mingetty(mingetty1[0],mingetty1);
  launch_mingetty(mingetty2[0],mingetty2);

  init_time = clock() - init_time;
  double time_taken = ((double)init_time)/CLOCKS_PER_SEC;

  fprintf(boot_time,"init config time = %f seconds\n",time_taken);

  fclose(boot_time);
  
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
