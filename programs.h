
#include <unistd.h>
#include <pthread.h>
#include <time.h>
//#define _XOPEN_SOURCE 200809L
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/swap.h>

#include <sys/reboot.h>

#include <string.h>

#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>

#define	LINUX_REBOOT_CMD_RESTART	0x01234567

int symlink(const char *target, const char *linkpath);
//wifi
#define DEV "wlan0"

static char * const mount_sys_commnad[] = {"sysfs","/sys", "sysfs"};
static char * const mount_proc_commnad[] = {"proc","/proc", "proc"};
static char * const mount_dev_commnad[] = {"dev","/dev", "devtmpfs"};
static char * const mount_pts_commnad[] = {"devpts","/dev/pts", "devpts"};

static char * const mount_efivars_commnad[] = {"efivarfs","/sys/firmware/efi/efivars", "efivarfs"};

static char * const mount_boot_commnad[] = {"/dev/nvme0n1p1","/boot", "vfat"};

static char * const mount_disk_commnad[] = {"/dev/nvme0n1p5","/root/disk", "ext4"};
static char * const mount_disk2_commnad[] = {"/dev/sda1","/disk2", "ext4"};

static char * const mount_shm_commnad[] = {"tmpfs","/dev/shm", "tmpfs"};

static char * const mount_run_commnad[] = {"tmpfs","/run", "tmpfs"};

static char * const mingetty1[] = {"/bin/pgetty", "--autologin=root","tty1",NULL};

static char * const mingetty2[] = {"/bin/pgetty", "--autologin=root","tty2",NULL};

static char * const pulseaudio[] = {"/bin/pulseaudio",NULL};


static char * const udev_script[] = {"/udev.sh",NULL};



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

