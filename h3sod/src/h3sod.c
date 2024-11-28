#include <stdio.h>
#include "args.h"
#include "proc.h"
#include "util.h"
#include "scan.h"
#include "impl.h"

#define global_static_ptr 0x699538

static uintptr_t get_state_ptr(int fd) {
  union word32 state_ptr;
  read_mem_bytes(fd, global_static_ptr, state_ptr.bytes, 4);
  return state_ptr.ptr;
}

static void resources(void) {
  OPEN_MEM("h3sod");
  uintptr_t global_offset = 0x20b6c;
  uintptr_t player_offset = 0x168;
  uintptr_t status_offset = 0x1f636;
  uintptr_t state_ptr = get_state_ptr(fd);
  uintptr_t resources_ptr = state_ptr + global_offset;
  union word32 resources[7];
  char status[8];
  read_mem_bytes(fd, state_ptr + status_offset, status, 8);
  for (int i = 0; i < 8; i++) {
    if (status[i] != 0) {
      continue;
    }
    read_mem_bytes(fd, resources_ptr + i * player_offset, (char *)resources, sizeof(resources));
    printf("Player: %d, Wood: %d, Mercury: %d, Ore: %d, Sulphur: %d, Crystals: %d, Gems: %d, Gold: %d\n", i,
      resources[0].int32, resources[1].int32, resources[2].int32, resources[3].int32, resources[4].int32,
      resources[5].int32, resources[6].int32);
  }
}

static void coords(void) {
  OPEN_MEM("h3sod");
  uintptr_t state_ptr = get_state_ptr(fd);
  union word32 coords = {.int32 = 0};
  union word32 colour = {.int32 = 0};
  for (int c = 0; c < 8; c++) {
    int output = 0;
    for (int h = 0;; h++) {
      uintptr_t coords_ptr = state_ptr + 0x21620 + h * 0x492;
      uintptr_t colour_ptr = coords_ptr + 0x22;
      read_mem_bytes(fd, coords_ptr, coords.bytes, 4);
      read_mem_bytes(fd, colour_ptr, colour.bytes, 1);
      if (!IN_RANGE(-1, coords.int16[0], 200) ||
          !IN_RANGE(-1, coords.int16[1], 200) ||
          !IN_RANGE(-1, colour.bytes[0], 7)) {
        break;
      }
      if (colour.int32 == c) {
        if (!output) {
          printf("Player %d: ", c);
        }
        printf("(%d,%d)  ", coords.int16[0], coords.int16[1]);
        output = 1;
      }
    }
    if (output) {
      printf("\n");
    }
  }
}

int main(void) {
  resources();
  coords();
}
