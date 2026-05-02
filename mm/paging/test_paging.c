#include<stdio.h>
#include<string.h>
#include<sys/mman.h>
#include<stdlib.h>
#include<fcntl.h>

int main()
{
    char *buf;
    int fd;
    char buffer[20] = {0,};

    fd = open("/proc/vaddr_inspect", O_RDWR);

    printf("Before memory allocation \n");
    write(fd, "a", 1);

    //allocate using mmap anonymous pages to create seperate vma entries
    buf = mmap(NULL, 8192, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    printf("After memory allocation \n");
    write(fd, "a", 1);

    sprintf(buffer, "%u", buf);
    write(fd, buffer, strlen(buffer));

    printf("\r buf 0x%x  %s \n", buf, buffer);
    sleep(10);
     
    printf("\r before 2nd sleep\n");
    sleep(20);
    
    buf[0] = 'c';

    write(fd, "a", 1);   
    write(fd, buffer, strlen(buffer));
    close(fd);

    printf("\r before 3rd sleep\n");
    while(1)
        sleep(10);
  
    //buf[4096] = 'c';
   
    //sleep(20);

    free(buf);
    return 0;
}
