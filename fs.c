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

    assert(strcmp(header->magic, TAR_MAGIC) == 0,
           "fs: invalid tar header magic=%s, expected=ustar", header->magic);
    // assert(strcmp(header->version, TAR_VERSION) == 0,
    //        "fs: invalid tar header version=%s, expected=00",
    //        header->version);
    assert(header->type == TAR_TYPE_FILE,
           "fs: non-file types of blocks are not supported");

    struct file *file = &files[i];
    unsigned file_size = oct2dec(header->size, sizeof(header->size));
    file->in_use = true;
    strcpy(file->name, header->name);
    memcpy(file->data, header->data, file_size);
    file->size = file_size;

    printf("fs: loaded file=%s\n", file->name);

    // tar blocks are padded to a multiple of 512 bytes after data section
    // thus we need to align up to that 512-byte padding to find the next block
    off += align_up(sizeof(struct file_header) + file_size, SECTOR_SIZE);
  }
  printf("fs: init completed\n");
}

void fs_flush(void) {
  // 1. Create tar blocks in disk from files
  // 2. Write contents of disk byte-to-byte to virtio-blk dev
  memset(disk, 0, DISK_MAX_SIZE);
  unsigned off = 0;
  for (unsigned i = 0; i < FILES_MAX_NUM; i++) {
    struct file *f = &files[i];

    if (!f->in_use)
      continue;

    // create tar header
    struct file_header *header = (struct file_header *)&disk[off];
    memcpy(header->name, f->name, sizeof(f->name));
    strcpy(header->mode, "000644");
    strcpy(header->magic, TAR_MAGIC);
    strcpy(header->version, TAR_VERSION);
    header->type = TAR_TYPE_FILE;
    // dec2oct(f->size, header->size, sizeof(header->size));
    int filesz = f->size;
    for (int i = sizeof(header->size); i > 0; i--) {
      header->size[i - 1] = (filesz % 8) + '0';
      filesz /= 8;
    }

    int checksum = ' ' * sizeof(header->checksum);
    for (unsigned k = 0; k < sizeof(struct file_header); k++) {
      checksum += disk[off + k];
    }
    // dec2oct(checksum, header->checksum, sizeof(header->checksum));
    for (int i = 5; i >= 0; i--) {
      header->checksum[i] = (checksum % 8) + '0';
      checksum /= 8;
    }

    memcpy(header->data, f->data, f->size);

    off += align_up(sizeof(struct file_header) + f->size, SECTOR_SIZE);
  }

  for (unsigned sector = 0; sector < sizeof(disk) / SECTOR_SIZE; sector++) {
    // copy sector by sector
    write_disk(&disk[sector * SECTOR_SIZE], sector);
  }
}

struct file *fs_lookup(const char *filename) {
  assert(filename != NULL, "fs: attempt to access null pointer");

  for (int i = 0; i < FILES_MAX_NUM; i++) {
    struct file *f = &files[i];
    if (f->in_use && strcmp(f->name, filename) == 0) {
      return f;
    }
  }
  return NULL;
}
