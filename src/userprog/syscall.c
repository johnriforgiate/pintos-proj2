#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stddef.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "devices/shutdown.h"
#include "pagedir.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);
static void validate_frame (struct intr_frame *, int);
struct thread_file {
  struct file *file;
  int fd;
  struct list_elem elem;
};
// For debugging
char *syscall_names[] =  
  {
    /* Projects 2 and later. */
    "SYS_HALT",                   /* Halt the operating system. */
    "SYS_EXIT",                   /* Terminate this process. */
    "SYS_EXEC",                   /* Start another process. */
    "SYS_WAIT",                   /* Wait for a child process to die. */
    "SYS_CREATE",                 /* Create a file. */
    "SYS_REMOVE",                 /* Delete a file. */
    "SYS_OPEN",                   /* Open a file. */
    "SYS_FILESIZE",               /* Obtain a file's size. */
    "SYS_READ",                   /* Read from a file. */
    "SYS_WRITE",                  /* Write to a file. */
    "SYS_SEEK",                   /* Change position in a file. */
    "SYS_TELL",                   /* Report current position in a file. */
    "SYS_CLOSE",                  /* Close a file. */

    /* Project 3 and optionally project 4. */
    "SYS_MMAP",                   /* Map a file into memory. */
    "SYS_MUNMAP",                 /* Remove a memory mapping. */

    /* Project 4 only. */
    "SYS_CHDIR",                  /* Change the current directory. */
    "SYS_MKDIR",                  /* Create a directory. */
    "SYS_READDIR",                /* Reads a directory entry. */
    "SYS_ISDIR",                  /* Tests if a fd represents a directory. */
    "SYS_INUMBER"                 /* Returns the inode number for a fd. */
  };
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/*
Using this guide as a reference https://static1.squarespace.com/static/5b18aa0955b02c1de94e4412/t/5b85fad2f950b7b16b7a2ed6/1535507195196/Pintos+Guide
We read that the syscall stack for esp looks like this:
|-----------------------------| f->esp  
           SYS_CODE          
|-----------------------------|  
  arg[0] ptr -OR- int_value  
|-----------------------------|  
  arg[1] ptr -OR- int_value  
|-----------------------------|  
  arg[2] ptr -OR- int_value  
|-----------------------------|
*/
static void
syscall_handler (struct intr_frame *f) 
{
  //if (f->esp is a bad pointer){exit(-1);}
  //cast f->esp into an int*, then dereference it for the SYS_CODE
  validate_pointer((const void*) f->esp);
  switch(*(int*)f->esp)
  {
	  case SYS_HALT: // working
	    halt();
        break;
      case SYS_EXIT: // working
	    // Validate each used pointer
		validate_frame(f,1); 
		// *((int*)f->esp + 1);
		// status is an int which contains the definition of an open file
		// First we cast to an int to increment the esp by 4 bytes and then dereference
		exit(*((int*)f->esp + 1));
        break;
	  case SYS_EXEC: // TODO
		validate_frame(f,1);
		break;
		//f->eax = exec
	  case SYS_CREATE:
	    validate_frame(f, 2);
	    f->eax = create((char*)*((int*)f->esp + 1), *((unsigned*)f->esp + 2));
		break;
	  case SYS_OPEN:
	    validate_frame(f,1);
		f->eax = open((char*)*((int*)f->esp + 1));
		break;
	  case SYS_FILESIZE:
	    validate_frame(f,1);
		f->eax = filesize(*((int*)f->esp + 1));
		break;
      case SYS_READ:
	  // Validate each used pointer
		validate_frame(f, 3);
		
	    f->eax = read(*((int*)f->esp + 1), (void*)(*((int*)f->esp + 2)), *((unsigned*)f->esp + 3));
	    break;
	  case SYS_WRITE: // Minimal Implementation
		// Validate each used pointer
		validate_frame(f, 3);
		
		// *((int*)f->esp + 1);
		// fd is an int which contains the definition of an open file
		// First we cast to an int to increment the esp by 4 bytes and then dereference
		
		
		// (void*)(*((int*)f->esp + 2));
		// buffer is what is to be written to the file.
		// First cast to an int to increment by 8 and dereference to get the pointer to the function which we cast as a void pointer
        
		
		// *((unsigned*)f->esp + 3);
		// size is a pointer to an unsigned int that contains the length of the buffer to be written to the file
		// First increment 12 bytes and dereference it. 
		
		// Store the write return value in the eax register as a return.
		f->eax = write(*((int*)f->esp + 1), (void*)(*((int*)f->esp + 2)), *((unsigned*)f->esp + 3));
		break;
	  case SYS_CLOSE:
	    validate_frame(f,1);
	    close(*((int*)f->esp + 1));
		break;
	  default:
        printf (" unimplemented system call: ");
		printf (syscall_names[*(int*)f->esp]);
        exit(-1);
  }
}

static void
validate_frame (struct intr_frame *f, int num_args)
{
	for(int i = 0; i < num_args+1; i++)
	  validate_pointer((void*)(((int*)f->esp + i)));
}

void
halt(void)
{
	shutdown_power_off();
}

void 
exit(int status)
{
  // TODO: signal parent
  printf ("%s: exit(%d)\n", thread_current()->name, status);
  if(is_thread(thread_current()->parent))
    sema_up(&thread_current()->parent->process_wait_sema);
  thread_exit();
}

void
validate_pointer(const void *vaddr)
{
	  if(vaddr < (void*) 0x08048000 || is_kernel_vaddr(vaddr) || pagedir_get_page(thread_current()->pagedir, vaddr) == NULL)
	    exit(-1);
}

int
filesize(int fd)
{
  int size = -1;
  struct thread *cur = thread_current();
  struct list_elem *e;
  sema_down(&filesys_sema);
  
  for (e = list_begin(&cur->file_list); e != list_end(&cur->file_list); e = list_next(e))
    if (fd == list_entry(e, struct thread_file, elem)->fd) 
	  {
        size = file_length(list_entry(e, struct thread_file, elem)->file);
        sema_up(&filesys_sema);
        return size;
      }
  sema_up(&filesys_sema);
  return size;
}

int
read(int fd, const void* buffer, unsigned size)
{
  validate_pointer(buffer);
  int bytes_read = -1;
  struct thread *cur = thread_current();
  struct list_elem *e;
  sema_down(&filesys_sema);
  
  for (e = list_begin(&cur->file_list); e != list_end(&cur->file_list); e = list_next(e))
    if (fd == list_entry(e, struct thread_file, elem)->fd) 
	  {
        bytes_read = file_read(list_entry(e, struct thread_file, elem)->file, (void*)buffer, size);
        sema_up(&filesys_sema);
        return bytes_read;
      }
  sema_up(&filesys_sema);
  return bytes_read;
}

int
write(int fd, const void* buffer, unsigned size)
{
  validate_pointer(buffer);
  if (fd == STDOUT_FILENO)
  {
	if(size > 100)
      size = 100;
    putbuf(buffer, size);
    return size;
  }
  struct thread *cur = thread_current();
  struct list_elem *e;
  sema_down(&filesys_sema);
  
  for (e = list_begin(&cur->file_list); e != list_end(&cur->file_list); e = list_next(e))
    if (fd == list_entry(e, struct thread_file, elem)->fd) 
	  {
        size = file_write(list_entry(e, struct thread_file, elem)->file, (void*)buffer, size);
        sema_up(&filesys_sema);
        return size;
      }
  sema_up(&filesys_sema);
  return size;
}

bool
create(const char *file, unsigned initial_size)
{
  bool success = false;
  // Check for NULL pointer
  validate_pointer(file);
  
  // Check for a file that is too long/short
  if(strlen(file) > 14 || strlen(file) == 0) return false;
  
  sema_down(&filesys_sema);
  success = filesys_create(file, initial_size);
  sema_up(&filesys_sema);
  return success;
}

int
open(const char *file)
{
  validate_pointer(file);
  if(strlen(file) == 0) return -1;
  struct file* fp;
  struct thread * cur = thread_current();
  
  sema_down(&filesys_sema);
  fp = filesys_open(file);
  
  if(NULL == fp) return -1;
  
  // Check if file is already open by process
  struct list_elem *e;
  for (e = list_begin(&cur->file_list); e != list_end(&cur->file_list); e = list_next(e))
          if (fp == list_entry(e, struct thread_file, elem)->file) 
		  {
			  sema_up(&filesys_sema);
			  return -1;
		  }
  
  // Allocate a new file slot in the file_list
  struct thread_file *tf = malloc(sizeof(struct thread_file));
  tf->file = fp;
  tf->fd = cur->fd;
  cur->fd++;
  list_push_back(&cur->file_list, &tf->elem);

  sema_up(&filesys_sema);
  return tf->fd;
}

void
close(int fd)
{
  sema_down(&filesys_sema);
  struct thread *cur = thread_current();
  struct list_elem *e;
  for (e = list_begin(&cur->file_list); e != list_end(&cur->file_list); e = list_next(e))
          if (fd == list_entry(e, struct thread_file, elem)->fd) 
		  {
			  file_close(list_entry(e, struct thread_file, elem)->file);
			  list_remove(e);
			  free(list_entry(e, struct thread_file, elem));
			  sema_up(&filesys_sema);
			  return;
		  }
  sema_up(&filesys_sema);
  exit(-1); // fails to find file
}