#include "virtioblk.h"
#include "alloc.h"
#include "builtins.h"
#include "stdint.h"
#include "stdio.h"

// todo: move to proper module
#define assert(cond, msg)                                                      \
  if (!(cond))                                                                 \
  panic(msg)

struct virtio_virtq *blk_req_vq;
struct virtio_blk_req *blk_req;
paddr_t blk_req_paddr;
unsigned blk_capacity;

void virtio_blk_init(void) {
  // check device requirements
  assert(virtio_reg_read32(VIRTIO_REG_MAGIC) == VIRTIO_DEVICE_MAGIC,
         "virtio: invalid magic value");
  assert(virtio_reg_read32(VIRTIO_REG_VERSION) == VIRTIO_DEVICE_VERSION,
         "virtio: invalid version");
  assert(virtio_reg_read32(VIRTIO_REG_DEVICE_ID) == VIRTIO_DEVICE_BLK,
         "virtio: invalid device id");

  // general device init
  vurtio_reg_write32(VIRTIO_REG_DEVICE_STATUS, 0);
  virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_ACK);
  virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_DRIVER);
  virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_FEAT_OK);

  assert(virtio_reg_read32(VIRTIO_REG_DEVICE_STATUS) & VIRTIO_STATUS_FEAT_OK,
         "virtio: device is unusable");

  // device-specific init, block device in our case
  blk_req_vq = virtq_init(0);

  // finish device init
  virtio_reg_write32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_DRIVER_OK);

  // todo: why + 0?
  blk_capacity = virtio_reg_read32(VIRTIO_REG_DEVICE_CONFIG + 0) * SECTOR_SIZE;
  printf("virtio-blk: capasity is %d bytes\n", blk_capacity);

  blk_req_paddr =
      balloc_pages(align_up(sizeof(*blk_req), PAGE_SIZE) / PAGE_SIZE);

  blk_req = (struct virtio_blk_req *)blk_req_paddr;
}
