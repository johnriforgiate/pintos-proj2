#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void halt(void);
void exit(int status);

int write(int, const void*, unsigned);


void validate_pointer(const void *);

#endif /* userprog/syscall.h */
