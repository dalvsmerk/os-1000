#pragma once

#include "stdint.h"
#include "alloc.h"

// See virtio specs
// https://docs.oasis-open.org/virtio/virtio/v1.1/csprd01/virtio-v1.1-csprd01.html#x1-910003

#define SECTOR_SIZE 512
#define VIRTQ_ENTRY_NUM 16
#define VIRTIO_DEVICE_MAGIC 0x74726976
#define VIRTIO_DEVICE_VERSION 1
#define VIRTIO_DEVICE_BLK 2
#define VIRTIO_BLK_PADDR 0x10001000

#define VIRTIO_REG_MAGIC 0x00
#define VIRTIO_REG_VERSION 0x04
#define VIRTIO_REG_DEVICE_ID 0x08

#define VIRTIO_REG_QUEUE_SEL 0x30
#define VIRTIO_REG_QUEUE_NUM_MAX 0x34
#define VIRTIO_REG_QUEUE_NUM 0x38
#define VIRTIO_REG_QUEUE_ALIGN 0x3c
#define VIRTIO_REG_QUEUE_PFN 0x40
#define VIRTIO_REG_QUEUE_READY 0x44
#define VIRTIO_REG_QUEUE_NOTIFY 0x50

#define VIRTIO_REG_DEVICE_STATUS 0x70
#define VIRTIO_REG_DEVICE_CONFIG 0x100

#define VIRTIO_STATUS_ACK 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEAT_OK 8

#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1

#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1

// virtqueue descriptor area entry
struct virtq_desc {
  uint64_t addr;
  uint32_t len;
  uint16_t flags;
  uint16_t next;
} __attribute__((packed));

// virtqueue available ring
struct virtq_avail_ring {
  uint16_t flags;
  uint16_t index;
  uint16_t ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// virtqueue used ring entry
struct virtq_used_entry {
  uint32_t id;
  uint32_t len;
} __attribute__((packed));

// virtqueue used ring
struct virtq_used_ring {
  uint16_t flags;
  uint16_t index;
  struct virtq_used_entry ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// virtqueue
struct virtio_virtq {
  struct virtq_desc descs[VIRTQ_ENTRY_NUM];
  struct virtq_avail_ring avail_ring;
  struct virtq_used_ring used_ring __attribute__((aligned(PAGE_SIZE)));
  int queue_index;
  volatile uint16_t *used_index;
  uint16_t last_used_index;
} __attribute__((packed));

// virtio-blk request
struct virtio_blk_req {
  uint32_t type;
  uint32_t reserved;
  uint64_t sector;
  uint8_t data[512];
  uint8_t status;
} __attribute__((packed));

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
