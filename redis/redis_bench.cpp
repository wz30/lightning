#include <iostream>
#include <sw/redis++/redis++.h>
#include <cstring>
#include <algorithm>
#include <bits/stdc++.h>
#include <chrono>
#include <unistd.h>
using namespace sw::redis;
using namespace std;

void test(int object_size) {
  Redis redis = Redis("tcp://127.0.0.1:6379");
  char *a = new char[object_size];
  for (int i = 0; i < object_size; i++) {
    a[i] = 'a';
  }

  uint64_t num_tests = 100;

  std::cout << object_size << ", ";

  auto start = std::chrono::high_resolution_clock::now();

  for (uint64_t i = 0; i < num_tests; i++) {
    string key = "key" + std::to_string(i);
    // string val = "val" + std::to_string(i);
    int status = redis.set(key, std::string(a));
  }

  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> duration = end - start;

  std::cout << duration.count() / num_tests << ", ";


  start = std::chrono::high_resolution_clock::now();
  for (uint64_t i = 0; i < num_tests; i++) {
    string key = "key" + std::to_string(i);
    auto val = redis.get(key);
  }

  end = std::chrono::high_resolution_clock::now();

  duration = end - start;

  std::cout << duration.count() / num_tests << ", ";

  start = std::chrono::high_resolution_clock::now();
  for (uint64_t i = 0; i < num_tests; i++) {
    string key = "key" + std::to_string(i);
    redis.del(key);
  }

  end = std::chrono::high_resolution_clock::now();

  duration = end - start;

  std::cout << duration.count() / num_tests << std::endl;

  delete[] a; 
}

int main(int argc, char *argv[]) {
  srand(getpid());

  int test_runs = 100;
  for (int i = 0; i < test_runs; i++) {
    int object_size = 16;
    while (object_size <= 1024 * 1024) {
      test(object_size);
      object_size *= 2;
    }
  }

  return 0;
}

