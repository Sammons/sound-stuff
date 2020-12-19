#include <stdint.h>

struct ResolutionCrusher {

  static void process_frames(float* buffer, uint32_t size, int crush) {
     for (int i = 0; i < size; ++i) {
       buffer[i] = ((float)((int)(buffer[i] * crush))) / (float)crush;
     }
  }
};