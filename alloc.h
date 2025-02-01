#pragma once

#include "stdint.h"

#define PAGE_SIZE 4096 // 4kb

void *balloc_pages(size_t n);
