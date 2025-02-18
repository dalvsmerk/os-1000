#include "fs.h"
#include "assert.h"
#include "number.h"
#include "stdlib.h"
#include "string.h"
#include "virtioblk.h"

struct file files[FILES_MAX_NUM];
uint8_t disk[DISK_MAX_SIZE];

void fs_init(void) {
  for (unsigned sector = 0; sector < sizeof(disk) / SECTOR_SIZE; sector++) {
    read_disk(&disk[sector * SECTOR_SIZE], sector);
  }

  unsigned off = 0;

  for (int i = 0; i < FILES_MAX_NUM; i++) {
    struct file_header *header = (struct file_header *)&disk[off];

    if (header->name[0] == '\0')
      break;

    assert(strcmp(header->magic, "ustar") == 0,
           "fs: invalid tar header magic=%s, expected=ustar", header->magic);

    struct file *file = &files[i];
    unsigned file_size = oct2dec(header->size, sizeof(header->size));
    file->in_use = true;
    strcpy(file->name, header->name);
    memcpy(file->data, header->data, file_size);
    file->size = file_size;

    printf("fs: loaded file=%s\n", file->name);
    printf("fs: data=%s\n", file->data);

    off += align_up(sizeof(struct file_header) + file_size, SECTOR_SIZE);
    // todo: why not just seeking one file after another?
    // off += sizeof(struct file_header) + file_size;
  }
  printf("fs: init completed\n");
}
