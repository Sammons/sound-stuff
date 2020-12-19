#include <string>
#include <stdio.h>

// #include "./zmq.hh"
#include <czmq.h>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>

std::vector<std::string> zmq_message_queue = std::vector<std::string>();
std::mutex zmq_queue_mut;

void access_queue(std::string* to_write, std::string* read_into) {
  std::lock_guard<std::mutex> lk(zmq_queue_mut);
  if (to_write != nullptr) {
    zmq_message_queue.push_back(*to_write);
  } else {
    if (zmq_message_queue.size() > 0) {
      *read_into = zmq_message_queue[zmq_message_queue.size() - 1];
      zmq_message_queue.pop_back();
    }
  }
}

void listen_to_zmq() {
  char addr[] = "tcp://127.0.0.1:4002";
  zsock_t *pull = zsock_new_pull(addr);
  while (true)
  {
    char *string = zstr_recv_nowait(pull);
    if (string) {
      std::string s = std::string(string);
      access_queue(&s, nullptr);
      zstr_free(&string);
    }
  }
}

int main(int argc, char const *argv[])
{
  /* code */
  std::thread listener = std::thread(&listen_to_zmq);
  listener.detach();
  while (true) {
    std::string out = std::string();
    access_queue(nullptr, &out);
    if (!out.empty()) {
      int active;
      long ts;
      sscanf(out.c_str(), "ts=[%ld],active=[%d]",&ts, &active);
      printf("%ld, %d\n", ts, active);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}
