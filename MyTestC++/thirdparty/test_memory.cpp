#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

#define BUF_SIZE 100
// 通过共享映射的方式修改文件
void WriteWithShareMaping() {
  int fd, pagecount;
  struct stat sb;
  char *mapped;
  size_t filesize, mmapsize, pagesize, datasize;
  const char *filepath = "/Users/cocodevil/Downloads/test.txt";
  const char *data = "ttttddddd";

  datasize = strlen(data);
  printf("DataSize:%lu\n", datasize);
  if (sizeof(filepath) == 0 || datasize == 0) {
    return;
  }

  /*打开文件,不存在就创建*/
  // if ((fd = open(filepath, O_RDWR | O_CREAT)) < 0) {
  //   perror("open");
  // }

  /*open file:create or truncate;set file size*/
  fd = open(filepath, O_RDWR | O_CREAT, 0777);
  printf("%d\n", fd);

  if ((fstat(fd, &sb)) == -1) {
    perror("fstat");
  }

  pagesize = sysconf(_SC_PAGE_SIZE);
  printf("SYS_PAGE_SIZE:%lu\n", pagesize);

  filesize = sb.st_size;
  printf("Current_size:%lu\n", filesize);
  mmapsize = filesize + datasize;

  // 计算需要的分页
  pagecount = mmapsize / pagesize + 1;
  printf("pagecount:%d\n", pagecount);
  printf("length:%lu\n", pagecount * pagesize);
  printf("setting file size to %lu\n", mmapsize);
  // 修改文件存储需要的空间
  // ftruncate(fd, mmapsize);
  lseek(fd, mmapsize - 1, SEEK_SET);
  write(fd, " ", 1);

  /**将文件映射至进程的地址空间**/
  if ((mapped = (char *)mmap(NULL, mmapsize, PROT_READ | PROT_WRITE, MAP_SHARED,
                             fd, 0)) == (void *)-1) {
    perror("mmap");
  }
  printf("%s\n", mapped);

  /**映射完成后，关闭系统文件操作**/
  close(fd);

  // 从上一个结束位置开始写入
  // memmove(mapped + filesize, data, datasize);
  memcpy(mapped + filesize, data, datasize);

  /* 将修改同步到磁盘文件*/
  if ((msync((void *)(mapped + filesize), datasize, MS_SYNC)) == -1) {
    perror("msync");
  }

  printf("%s\n", mapped);

  /* 释放存储映射区*/
  if ((munmap((void *)mapped, mmapsize)) == -1) {
    perror("mumap");
  }
  exit(0);
}

//指定一个大于该文件大小的内存映射区大小，跟踪该文件的当前大小（以确保不访问当前文件尾以远的部分），然后就让该文件的大小随着往其中每次写入数据而增长
// #define SIZE 50
// void WriteInfoWithMappingAdd() {
//   int fd, i;
//   char *ptr;
//   /*open:create or truncate;them mmap file */
//   fd = open("/Users/cocodevil/Downloads/test1.log", O_RDWR | O_CREAT |
//   O_TRUNC,
//             0777);
//   ptr = (char *)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

//   for (i = 10; i < SIZE; i += 10) {
//     printf("setting file size to %d\n", i);
//     ftruncate(fd, i);
//     printf("ptr[%d] = %d\n", i - 1, ptr[i - 1]);
//   }
//   printf("%s", ptr);
// }

int main(int argc, char *argv[]) {
  // int fd = open("/Users/cocodevil/Downloads/test.log", O_RDWR);
  // FILE* f = fdopen(fd, "rw");
  // fseek(f, 0, SEEK_END);
  // long ps = ftell(f);

  // cout << "file size:" << ps << endl;

  // // 分配的大小必须为页的整数倍
  // int* p = (int*)mmap(NULL, ps, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
  // if (p == MAP_FAILED) {
  //   perror("mmap, error");
  //   exit(1);
  // }

  // // 比如我用2个进程运行该程序，进程1 让*p=100 ，那么进程2
  // //
  // 去读取*p也会变成100，但重要的是文件并没有更新，就是说没有IO操作，文件仅仅在msync()或mumap()调用时才会被更新，因为此时2个进程映射的该文件，都指向了相同的内存区域，也就说只有内存中的数据改变了
  // int oper, data;
  // while (true) {
  //   cin >> oper;
  //   if (oper == 1) {
  //     cin >> data;
  //     *p = data;
  //   } else {
  //     cout << *p << endl;
  //   }
  // }
  // munmap(p, ps);
  // close(fd);
  WriteWithShareMaping();
  // WriteInfoWithMappingAdd();
  return 0;
}
