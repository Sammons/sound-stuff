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
      zmq_message_queue.pop_back ();
    }
  }
}

void listen_to_zmq() {
  char addr[] = "tcp://127.0.0.1:4002";
  zsock_t *push = zsock_new_push("tcp://127.0.0.1:4003");
  zsock_connect(push, "tcp://127.0.0.1:4003");
  /** <name>,(<param>=<type>,<default>,[min],[max])+ */
  /** register noise gate */
  if (zstr_send(push, "Noise Gate,Active=bool,1,Threshold=number,-30") < 0) {
    printf("Failed: %s\n", zmq_strerror(zmq_errno()));
  };

  /** register delay plugin */
  if (zstr_send(push, "Simple Delay,Active=bool,1,Duration=number,100,1,2000") < 0) {
    printf("Failed\n");
  };

  /** register amplifier plugin */
  if (zstr_send(push, "Amp,Active=bool,1,Gain=number,1,0,50") < 0) {
    printf("Failed\n");
  };

  printf("Tried to send effect registration messages\n");

  /** - */
  zmq_close(push);

  zsock_t *pull = zsock_new_pull(addr);
  if (pull != nullptr) {
    while (true)
    {
      char *string = zstr_recv_nowait(pull);
      if (string) {
        std::string s = std::string(string);
        access_queue(&s, nullptr);
        zstr_free(&string);
      }
    }
  } else {
    printf("ZMQ Failed to boot: %s\n", zmq_strerror(zmq_errno()));
  }
}

std::string pop_zmq_message() {
  std::string out = std::string();
  access_queue(nullptr, &out);
  return out;
}