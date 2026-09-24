
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include "gd32e10x.h"

extern uint32_t _estack, _eheap;

int uart_getchar(void);
void uart_putchar(uint8_t c);

//Dummy CRT system call implementations
int __attribute__((weak)) _fstat(int fd, struct stat *st)
{
	st->st_mode=S_IFCHR;
	return 0;
}

off_t __attribute__((weak)) _lseek(int fd, off_t pos, int whence)
{
	return 0;
}

int __attribute__((weak)) _close(int fd)
{
	return 0;
}

int __attribute__((weak)) _read(int fd, char *buf, int len)
{
	int i, c;
	if(fd==STDIN_FILENO)
	for(i=0;i<len;i++)
	{
		c=uart_getchar();
		if(c==-1) break;
		buf[i]=c&0xff;
	}
	return i;
}

int __attribute__((weak)) _write(int fd, char *buf, int len)
{
	int i;
	if(fd==STDOUT_FILENO)
	for(i=0;i<len;i++)
		uart_putchar(buf[i]);
	return len;
}

uint8_t* __attribute__((weak)) _sbrk(int inc)
{
	static uint8_t *heap=NULL;
	uint8_t *prev;
	if(!heap) heap=(uint8_t*)&_estack;
	prev=heap;
	heap+=inc;
	if(heap>=(uint8_t*)&_eheap)
	{
		heap=prev;
		errno=ENOMEM;
		return (uint8_t*)-1;
	}
	return prev;
}

int __attribute__((weak)) _isatty(int fd) {return 1;}

int __attribute__((weak)) _getpid(void) {return -1;}

void __attribute__((weak)) _kill(int pid, int sig) {return;}

void __attribute__((weak)) _exit(int status) {while(1);}
