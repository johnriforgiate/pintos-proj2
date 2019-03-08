#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
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
  switch(*(int*)f->esp)
  {
	  case SYS_HALT:
	  {
	    printf ("unimplemented SYS_HALT call!\n");
        thread_exit ();
        break;
	  }
      case SYS_EXIT:
	  {
	    printf ("unimplemented SYS_EXIT call!\n");
        thread_exit ();
        break;
	  }
	  case SYS_WRITE:
	  {
		//printf ("unimplemented SYS_WRITE call!\n");
        //thread_exit ();

		// fd is an int which contains the definition of an open file
		// First we cast to an int to increment the esp by 4 bytes and then dereference
		int fd = *((int*)f->esp + 1);
		
		// buffer is what is to be written to the file.
		// First cast to an int to increment by 8 and dereference to get the pointer to the function which we cast as a void pointer
        void *buffer = (void*)(*((int*)f->esp + 2));
		
		// size is a pointer to an unsigned int that contains the length of the buffer to be written to the file
		// First increment 12 bytes and dereference it. 
        unsigned size = *((unsigned*)f->esp + 3);
		
        f->eax = write(fd, buffer, size);
		break;
	   }
	  default:
      {	  
        printf (" unimplemented system call: ");
		printf (syscall_names[*(int*)f->esp]);
        thread_exit ();
      }
  }
}


int
write(int fd, const void* buffer, unsigned size)
{
    if (fd == STDOUT_FILENO)
    {
      putbuf(buffer, size);
      return size;
    }
	printf ("unimplemented SYS_WRITE not to stdout call!\n");
	thread_exit();
}