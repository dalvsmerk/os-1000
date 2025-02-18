#pragma once

#include "virtioblk.h"
#include "builtins.h"
#include "stdbool.h"

#define FILES_MAX_NUM 2
#define DISK_MAX_SIZE align_up(sizeof(struct file) * FILES_MAX_NUM, SECTOR_SIZE)

// TAR header
struct file_header {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char type;
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char padding[12];
  char data[];
} __attribute__((packed));

struct file {
  bool in_use;
  char name[100];
  char data[1024];
  size_t size;
};

void fs_init(void);
