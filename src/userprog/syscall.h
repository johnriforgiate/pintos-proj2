#include <stdbool.h>
#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void halt(void);
void exit(int status);
bool create(const char*, unsigned);
int open(const char *);
int filesize(int);
int read(int, const void*, unsigned);
int write(int, const void*, unsigned);

void close(int);


void validate_pointer(const void *);


#endif /* userprog/syscall.h */
