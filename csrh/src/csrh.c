#include <stdio.h>
#include <unistd.h>
#include "args.h"
#include "proc.h"
#include "util.h"
#include "impl.h"
#include <string.h>
#include <errno.h>

#define START_ADDR 0x100000
// libclient2
// #define GLOBAL_STATE_PTR_ADDR (LIBCLIENT_BASE + 0x3aa6690 - START_ADDR)
// #define LOCAL_PLAYER_CTL_PTR_ADDR (LIBCLIENT_BASE + 0x3aa1618 - START_ADDR)
// #define PAWN_ARRAY_ADDR (LIBCLIENT_BASE + 0x390d138 - START_ADDR)

static uintptr_t GLOBAL_STATE_PTR_ADDR;
static uintptr_t LOCAL_PLAYER_CTL_PTR_ADDR;
static uintptr_t PAWN_ARRAY_ADDR;

int MEM_FD;
uintptr_t LIBCLIENT_BASE;

struct entity {
  uintptr_t ctl;
  uintptr_t pawn;
  int id;
  int is_local;
  float x, y;
  float yaw;
  int is_alive;
  char team;
};

static union word64 read_word(uintptr_t addr) {
  return read_mem_word64(MEM_FD, addr);
}

void read_addr(uint8_t text[], size_t text_len, uintptr_t *addr, uint8_t sig[], size_t sig_len) {
  for (int i = 0; i < text_len; i++) {
    int j;
    for (j = 0; j < sig_len; j++) {
      if (text[i + j] != sig[j]) {
        break;
      }
    }
    if (j == sig_len) {
      union word32 w;
      uintptr_t ip = i + sig_len + sizeof(w.bytes);
      memcpy(w.bytes, text + i + sig_len, sizeof(w.bytes));
      *addr = LIBCLIENT_BASE + ip + w.int32;
      break;
    }
  }
}

void read_addrs(void) {
  const size_t text_len = 70000000;
  uint8_t global_sig[] = {0x7e, 0x18, 0x48, 0x8d, 0x05};
  uint8_t player_sig[] = {0x83, 0xff, 0xff, 0x74, 0x0b, 0x48, 0x8b, 0x05};
  uint8_t pawn_sig[] = {0x83, 0xf9, 0xff, 0x74, 0x4d, 0x48, 0x8b, 0x35};
  uint8_t *text = malloc(text_len);
  lseek(MEM_FD, (off_t)LIBCLIENT_BASE, SEEK_SET);
  read(MEM_FD, text, text_len);
  read_addr(text, text_len, &GLOBAL_STATE_PTR_ADDR, global_sig, SIZEARR(global_sig));
  read_addr(text, text_len, &LOCAL_PLAYER_CTL_PTR_ADDR, player_sig, SIZEARR(player_sig));
  read_addr(text, text_len, &PAWN_ARRAY_ADDR, pawn_sig, SIZEARR(pawn_sig));
  free(text);
}

size_t get_ctls(uintptr_t ctls[]) {
  size_t count = 0;
  uintptr_t ctl_list = read_word(read_word(GLOBAL_STATE_PTR_ADDR).ptr64 + 0x10).ptr64;
  for (int e = 1; e <= 64; e++) {
    uintptr_t ctl = read_word(ctl_list + e * 0x78).ptr64;
    if (ctl != 0) {
      ctls[count++] = ctl;
    }
  }
  return count;
}

uintptr_t get_pawn(uintptr_t ctl) {
  int handle = read_word(ctl + 0x7b4).int32;
  uintptr_t pawn_ptr = read_word(read_word(PAWN_ARRAY_ADDR).ptr64 + (handle >> 9 & 0x3f) * 8).ptr64 + (handle & 0x1ff) * 0x78;
  return read_word(pawn_ptr).ptr64;
}

static void read_entity(uintptr_t ctl, int id, struct entity *e) {
  e->id = id;
  uintptr_t local_ctl = read_word(LOCAL_PLAYER_CTL_PTR_ADDR).ptr64;
  uintptr_t pawn = get_pawn(ctl);
  e->is_local = ctl == local_ctl;
  e->is_alive = read_word(ctl + 0x9a4).int32;
  int team = read_word(ctl + 0x55b).int32;
  e->team = team == 2 ? 'T' : team == 3 ? 'C' : '_';
  uintptr_t offsets_xyz[] = {0x38, 0x70};
  uintptr_t ptr_x = hop(MEM_FD, pawn, offsets_xyz, SIZEARR(offsets_xyz));
  e->x = read_word(ptr_x).float32;
  e->y = read_word(ptr_x + 4).float32;
  uintptr_t offsets_yaw[] = {0x10, 0x0, 0x4b0, 0x720};
  uintptr_t ptr_y = hop(MEM_FD, pawn, offsets_yaw, SIZEARR(offsets_yaw));
  e->yaw = read_word(ptr_y).float32;
  e->ctl = ctl;
  e->pawn = pawn;
}

size_t read_players(struct entity *players) {
  size_t count = 0;
  struct entity e;
  uintptr_t ctls[64];
  size_t ctl_count = get_ctls(ctls);
  for (int i = 0; i < ctl_count; i++) {
    read_entity(ctls[i], i, &e);
    if (e.is_alive && (e.team == 'T' || e.team == 'C')) {
      memcpy(&players[count++], &e, sizeof(e));
    }
  }
  return count;
}

static void print_player(struct entity *player) {
  printf("%d:%f,%f,%f:%d,%c,%d", player->id, player->x, player->y, player->yaw, player->is_local, player->team, player->is_alive);
}

static void main_loop(void) {
  struct entity players[64];
  for (;;) {
    size_t count = read_players(players);
    for (int i = 0; i < count; i++) {
      print_player(&players[i]);
      putchar(i < count - 1 ? '|' : '\n');
    }
    fflush(stdout);
    msleep(25);
  }
}

static void run(void) {
  OPEN_MEM("cs2$");
  MEM_FD = fd;
  LIBCLIENT_BASE = get_base_addr(pid, "libclient");
  //disable_stderr();
  read_addrs();
  main_loop();
}

__attribute__((constructor))
static void init(void) {
  args_add("csrh", run);
}
// decouple pids
// show bomb/more info