#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "./spectral_cutter.cc"
#include "./resolution_crusher.cc"
// #ifndef S_EXPERIMENT
// #define S_EXPERIMENT

#define LOOKBACK_MAX_SIZE 2 * 96000

class ExperimentEffect {
  float lookback[LOOKBACK_MAX_SIZE] = { 0 };
  long lookback_read_pos = LOOKBACK_MAX_SIZE - 9600; // 100ms
  long lookback_write_pos = 0;
  long window_size = 0;
  int zero_written = 0;
  int active = 0;
  SpectralCutter cutter;
  
  public:
  void configure(double sample_rate, int duration_ms, int active) {
    this->window_size = (long)(((double)duration_ms/1000.0) * sample_rate);
    this->active = active;
    if (this->window_size >= LOOKBACK_MAX_SIZE || this->window_size <= 0) {
      printf("Invalid duration, lookback space required is too large, truncating to max.\n");
      this->window_size = LOOKBACK_MAX_SIZE - 1;
    } else {
      printf("distance between reader/writer %ld\n", this->window_size);
    }
    this->lookback_read_pos = this->lookback_write_pos - this->window_size;
    if (this->lookback_read_pos < 0) {
      this->lookback_read_pos = LOOKBACK_MAX_SIZE + this->lookback_read_pos;
    }
    printf("Position w%ld r%ld\n", this->lookback_write_pos, this->lookback_read_pos);
    // cutter.initialize(2048);
  }
  void process_frames(const float* input_buffer, float* output_buffer, uint32_t size, double sample_rate) {
    for (int i = 0; i < size; ++i) {
      lookback[lookback_write_pos] = input_buffer[i];
      // float delta = lookback[(lookback_read_pos + 1) % max_lookback_position] - lookback[lookback_read_pos];
      // if (active == 1) {
        // if (!zero_written && -lookback[lookback_read_pos] > 0.00001) {
        //   output_buffer[i] = input_buffer[i];
        // } else {
        //   if (!zero_written) {
        //     zero_written = 1;
        //   }
        // }
      output_buffer[i] = input_buffer[i] + lookback[lookback_read_pos];
      // } else {
      //   output_buffer[i] = input_buffer[i];
      // }
      // cutter.cut(&output_buffer[0], size);
      // ResolutionCrusher::process_frames(&output_buffer[0], size, 100);

      ++lookback_write_pos;
      ++lookback_read_pos;
      lookback_write_pos %= LOOKBACK_MAX_SIZE;
      lookback_read_pos %= LOOKBACK_MAX_SIZE;
    }
  }
  void reset_state() {
    memset((void*) &lookback[0], 0, sizeof(lookback));
    lookback_read_pos = 0;
    zero_written = 0;
  }
};

// #endif