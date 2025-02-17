#include "virtioblk.h"
#include "alloc.h"
#include "assert.h"
#include "builtins.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct virtq *blk_virtq;
struct virtio_blk_req *blk_req;
paddr_t blk_req_paddr;
unsigned blk_capacity;

uint32_t virtio_reg_read32(unsigned offset) {
  return *((volatile uint32_t *)(VIRTIO_BLK_PADDR + offset));
}

uint64_t virtio_reg_read64(unsigned offset) {
  return *((volatile uint64_t *)(VIRTIO_BLK_PADDR + offset));
}

void virtio_reg_write32(unsigned offset, uint32_t value) {
  *((volatile uint32_t *)(VIRTIO_BLK_PADDR + offset)) = value;
}

void virtio_reg_fetch_and_or32(unsigned offset, uint32_t value) {
  virtio_reg_write32(offset, virtio_reg_read32(offset) | value);
}

struct virtq *virtq_init(unsigned index) {
  virtio_reg_write32(VIRTIO_REG_QUEUE_SEL, index);

  assert(virtio_reg_read32(VIRTIO_REG_QUEUE_PFN) == 0,
         "virtio: virtq is already used");
  assert(virtio_reg_read32(VIRTIO_REG_QUEUE_NUM_MAX) != 0,
         "virtio: virtq is not available");

  paddr_t virtq_paddr = (paddr_t)balloc_pages(
      align_up(sizeof(struct virtq), PAGE_SIZE) / PAGE_SIZE);

  struct virtq *vq = (struct virtq *)virtq_paddr;
  vq->queue_index = index;
  vq->used_index = (volatile uint16_t *)&vq->used_ring.index; // is it just 0?

  virtio_reg_write32(VIRTIO_REG_QUEUE_NUM, VIRTQ_ENTRY_NUM);
  virtio_reg_write32(VIRTIO_REG_QUEUE_ALIGN, 0);
  virtio_reg_write32(VIRTIO_REG_QUEUE_PFN, virtq_paddr);

  return vq;
}

void virtio_blk_init(void) {
  // check device requirements
  assert(virtio_reg_read32(VIRTIO_REG_MAGIC) == VIRTIO_DEVICE_MAGIC,
         "virtio: invalid magic value");
  assert(virtio_reg_read32(VIRTIO_REG_VERSION) == VIRTIO_DEVICE_VERSION,
         "virtio: invalid version");
  assert(virtio_reg_read32(VIRTIO_REG_DEVICE_ID) == VIRTIO_DEVICE_BLK,
         "virtio: invalid device id");

  // general device init
  virtio_reg_write32(VIRTIO_REG_DEVICE_STATUS, 0);
  virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_ACK);
  virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_DRIVER);
  // note: not supported by legacy interface
  virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_FEAT_OK);

  assert(virtio_reg_read32(VIRTIO_REG_DEVICE_STATUS) & VIRTIO_STATUS_FEAT_OK,
         "virtio: device is unusable");

  // device-specific init, block device in our case
  blk_virtq = virtq_init(0);
  printf("virtio: block device virtq initialized\n");

  // finish device init
  virtio_reg_write32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_DRIVER_OK);

  // todo: why + 0?
  blk_capacity = virtio_reg_read64(VIRTIO_REG_DEVICE_CONFIG + 0) * SECTOR_SIZE;
  printf("virtio-blk: capacity is %d bytes\n", blk_capacity);

  blk_req_paddr =
      (paddr_t)balloc_pages(align_up(sizeof(*blk_req), PAGE_SIZE) / PAGE_SIZE);

  blk_req = (struct virtio_blk_req *)blk_req_paddr;

  printf("virtio: block device initialized\n");
}

void virtq_kick(struct virtq *vq, int desc_index) {
  assert(vq != NULL, "vq must be a valid pointer");
  // write index of new request descriptor to ring buffer
  vq->avail_ring.ring[vq->avail_ring.index % VIRTQ_ENTRY_NUM] = desc_index;
  vq->avail_ring.index++;
  __sync_synchronize();
  // in reality, there will be only one queue per device in our implementation
  virtio_reg_write32(VIRTIO_REG_QUEUE_NOTIFY, vq->queue_index);
  vq->last_used_index++;
}

int virtq_busy(struct virtq *vq) {
  assert(vq != NULL, "vq must be a valid pointer");
  return vq->last_used_index != *vq->used_index;
  // return vq->last_used_index != vq->used_ring.index;
}

// NOTE: this does not support sending multiple requests for simplicity
// before sending next request, we need to wait for previous to be processed
// that's why always writing to the head of descriptors
void read_write_disk(void *buf, unsigned sector, int is_write) {
  if (sector >= blk_capacity / SECTOR_SIZE) {
    printf("virtio: attempt to read/write sector=%d over capacity=%d\n", sector,
           blk_capacity);
    return;
  }

  blk_req->sector = sector;
  blk_req->type = is_write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN;

  if (is_write)
    memcpy(blk_req->data, buf, SECTOR_SIZE);

  // construct descriptors for a single block device request
  struct virtq *vq = blk_virtq;

  // descriptor for metadata
  vq->descs[0].addr = blk_req_paddr;
  vq->descs[0].len = sizeof(uint32_t) * 2 + sizeof(uint64_t);
  vq->descs[0].flags = VIRTQ_DESC_F_NEXT;
  vq->descs[0].next = 1;

  // descriptor for data
  vq->descs[1].addr = blk_req_paddr + offsetof(struct virtio_blk_req, data);
  vq->descs[1].len = SECTOR_SIZE;
  vq->descs[1].flags = VIRTQ_DESC_F_NEXT | (is_write ? 0 : VIRTQ_DESC_F_WRITE);
  vq->descs[1].next = 2;

  // descriptor for status
  vq->descs[2].addr = blk_req_paddr + offsetof(struct virtio_blk_req, status);
  vq->descs[2].len = sizeof(uint8_t);
  vq->descs[2].flags = VIRTQ_DESC_F_WRITE;

  // notify device about new request that's in the head of descriptors (0-index)
  virtq_kick(vq, 0);

  __sync_synchronize();

  // polling - wait for request to be processed before allowing another request
  // to be sent while
  while (virtq_busy(vq))
    ;

  if (blk_req->status != 0) {
    printf("virtio: failed to read/write sector=%d status=%d\n", sector,
           blk_req->status);
    return;
  }

  if (!is_write)
    memcpy(buf, blk_req->data, SECTOR_SIZE);
}
