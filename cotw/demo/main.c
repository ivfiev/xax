#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

float getter(int tid) {
  return tid * 0.67;
}

void *thread(void *arg) {
  int tid = (long)arg;
  for (;;) {
    printf("%d -> %f\n", tid, getter(tid));
    sleep(1);
  }
}

int main(int argc, char **argv) {
  const size_t count = 5;
  pthread_t threads[count];
  pid_t tids[16];
  for (int i = 0; i < count; i++) {
    pthread_create(threads + i, NULL, thread, (void *)(long)i);
  }
  for (int i = 0; i < count; i++) {
    pthread_join(threads[i], NULL);
  }
  return 0;
}