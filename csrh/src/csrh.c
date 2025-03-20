#include <stdio.h>
#include "args.h"
#include "proc.h"
#include "util.h"
#include "impl.h"
#include <string.h>

#define START_ADDR 0x100000
#define GLOBAL_STATE_PTR_ADDR (LIBCLIENT_BASE + 0x3aa6690 - START_ADDR)
#define LOCAL_PLAYER_CTL_PTR_ADDR (LIBCLIENT_BASE + 0x3aa1618 - START_ADDR)
#define PAWN_ARRAY_ADDR (LIBCLIENT_BASE + 0x390d138 - START_ADDR)

int MEM_FD;
uintptr_t LIBCLIENT_BASE;

struct entity {
  uintptr_t ctl;
  uintptr_t pawn;
  int is_local;
  float x, y;
  float yaw;
  int is_alive;
  char team;
};

static union word64 read_word(uintptr_t addr) {
  return read_mem_word64(MEM_FD, addr);
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

static void read_entity(uintptr_t ctl, struct entity *e) {
  uintptr_t local_ctl = read_word(LOCAL_PLAYER_CTL_PTR_ADDR).ptr64;
  uintptr_t pawn = get_pawn(ctl);
  e->is_local = ctl == local_ctl;
  e->is_alive = read_word(ctl + 0x99c).int32;
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
    read_entity(ctls[i], &e);
    if (e.is_alive && (e.team == 'T' || e.team == 'C')) {
      memcpy(&players[count++], &e, sizeof(e));
    }
  }
  return count;
}

static void print_player(struct entity *player) {
  printf("%f,%f,%f:%d,%c,%d\n", player->x, player->y, player->yaw, player->is_local, player->team, player->is_alive);
}

static void main_loop(void) {
  struct entity players[64];
  for (;;) {
    size_t count = read_players(players);
    for (int i = 0; i < count; i++) {
      print_player(&players[i]);
    }
    fflush(stdout);
    msleep(40);
  }
}

static void run(void) {
  OPEN_MEM("cs2$");
  READ_DS(1536);
  MEM_FD = fd;
  LIBCLIENT_BASE = get_base_addr(pid, "libclient");
  disable_stderr();
  main_loop();
}

__attribute__((constructor))
static void init(void) {
  args_add("csrh", run);
}
// decouple pids
// sigscan for global pointers
// show bomb/more info