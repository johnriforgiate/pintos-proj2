#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  verify_pointer(f->esp);
  int syscallNumber = (int)f->esp;
  // TODO: dereference 
  printf ("system call!\n");
  thread_exit ();
}

void
verify_pointer(void *vaddr)
{
  if(NULL == vaddr||is_kernel_vaddr(vaddr))// TODO: Check if address is outside user defined memory
    thread_exit ();
}