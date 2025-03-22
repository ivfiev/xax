#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <string.h>

void move_mouse(int fd, int dx, int dy) {
    struct input_event ev;

    ev.type = EV_REL;
    ev.code = REL_X;
    ev.value = dx;
    write(fd, &ev, sizeof(ev));

    ev.type = EV_REL;
    ev.code = REL_Y;
    ev.value = dy;
    write(fd, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

int main() {
    int fd = open("/dev/uinput", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open /dev/uinput");
        return 1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);
    ioctl(fd, UI_SET_RELBIT, REL_Z);
    ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(fd, UI_SET_RELBIT, REL_HWHEEL);
    
    struct uinput_setup usetup = {
      .name = "bro don't ban",
      .id = {
        .bustype = BUS_VIRTUAL,
        .vendor = 6,
        .product = 6,
        .version = 6
      }
    };

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    sleep(1);

    int x, y;
    char buf[16];
    for (;;) {
      fgets(buf, sizeof(buf), stdin);
      if (sscanf(buf, "%d %d", &x, &y) != 2) {
        continue;
      }
      move_mouse(fd, x, y);
    }

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}