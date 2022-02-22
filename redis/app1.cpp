#include <iostream>
#include <sw/redis++/redis++.h>
#include <cstring>
#include <algorithm>
#include <bits/stdc++.h>
#include <chrono>
using namespace sw::redis;
using namespace std;
int main(int argc, char *argv[]) {
  try {
    // Create an Redis object, which is movable but NOT copyable.
    Redis redis = Redis("tcp://127.0.0.1:6379");
    int num_test = 100;
    
      // ***** STRING commands *****
      
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i<num_test; i++) {
      string key = "key" + std::to_string(i);
      string val = "val" + std::to_string(i);
      int status = redis.set(key, val);
      //std::cout << status << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "set: " << duration.count()/num_test  << std::endl; 
    //get the keys

    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i<num_test; i++) {
      string key = "key" + std::to_string(i);
      auto val = redis.get(key);
//      if(val) {
//        std::cout << "val: " << std::endl;
//      }else {
//        std::cout << "not get value&&&" << std::endl;
//      }
    }
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;

    std::cout << "get: " <<duration.count()/num_test  << std::endl; 

    start = std::chrono::high_resolution_clock::now();
    // delete the keys
    for(int i = 0; i<num_test; i++) {
      string key = "key" + std::to_string(i);
      redis.del(key);
    }

    end = std::chrono::high_resolution_clock::now();
    duration = end - start;

    std::cout << "del: " << duration.count()/num_test  << std::endl; 
    // redis cluster
    auto redis_cluster = RedisCluster("tcp://127.0.0.1:7000");
     
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i<num_test; i++) {
      string key = "key" + std::to_string(i);
      string val = "val" + std::to_string(i);
      redis_cluster.set(key, val);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;

    std::cout << "set: " << duration.count()/num_test  << std::endl; 
    //get the keys

    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i<num_test; i++) {
      string key = "key" + std::to_string(i);
      auto val = redis_cluster.get(key);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;

    std::cout << "get: " <<duration.count()/num_test  << std::endl; 

    start = std::chrono::high_resolution_clock::now();
    // delete the keys
    for(int i = 0; i<num_test; i++) {
      string key = "key" + std::to_string(i);
      redis_cluster.del(key);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "del: " <<duration.count()/num_test  << std::endl; 
    
//    auto val = redis.get(key);    // val is of type OptionalString. See 'API Reference' section for details.
//    if (val) {
//        // Dereference val to get the returned value of std::string type.
//        std::cout << *val << std::endl;
//    }   // else key doesn't exist.
//    

  } catch (const Error &e) {
    // Error handling.
	std::cout << "catch exception" << std::endl;
  }
	return 0;
}
