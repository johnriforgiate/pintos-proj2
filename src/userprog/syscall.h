#include <stdbool.h>
#include "lib/user/syscall.h"
#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void halt(void);
void exit(int status);
bool create(const char*, unsigned);
int open(const char *);
int filesize(int);
int read(int, void*, unsigned);
int write(int, const void*, unsigned);
pid_t exec(const char*);
int wait(pid_t);
void seek(int, unsigned);
unsigned tell(int);


void close(int);


void validate_pointer(const void *);


#endif /* userprog/syscall.h */
