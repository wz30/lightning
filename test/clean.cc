#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.h"

void clear(LightningClient &client, int num_tests, int object_size) {
  char *a = new char[object_size];
  for (int i = 0; i < object_size; i++) {
    a[i] = 'a';
  }

  // for (uint64_t i = 0; i < num_tests; i++) {
  //   uint8_t *ptr;
  //   int status = client.Create(i, &ptr, object_size);
  //   memcpy(ptr, a, object_size);
  //   status = client.Seal(i);
  // }

  char *out;
  size_t size;

  for (uint64_t i = 0; i < num_tests; i++) {
    int status = client.Get(i, (uint8_t **)&out, &size);
    if(status==0) {
      client.Delete(i);
    }
  }

  delete[] a;
}

int main(int argc, char **argv) {
  LightningClient client("/tmp/lightning", "password");

  int nums = atoi(argv[1]);
  int obj_size = 1024
  
  clear(client, nums, obj_size);

  return 0;
}
