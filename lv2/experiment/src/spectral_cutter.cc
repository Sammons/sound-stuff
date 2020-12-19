#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "fftw3.h"

struct SpectralCutter
{
  fftwf_plan forward_plan = nullptr;
  fftwf_plan backward_plan = nullptr;

  fftwf_complex *analysis_of_current_input = nullptr;

  float *elimination_buffer = nullptr;
  float *input_buffer = nullptr;
  float *window_buffer = nullptr;
  float window_gain_correction = 0;

  int frequency_bins = 0;
  int window_size = 0;

  inline float to_mag(float re, float im)
  {
    return sqrt((re * re) + (im * im));
  }

  inline float to_phase(float re, float im)
  {
    return atan2(im, re);
  }

  inline float calc_real(float mag, float phase)
  {
    return mag * cos(phase);
  }

  inline float calc_im(float mag, float phase)
  {
    return mag * sin(phase);
  }

  void initialize(int window_size)
  {
    this->window_size = window_size;
    window_buffer = (float *)fftwf_alloc_real(window_size);
    if (window_buffer == nullptr) {
      exit(1);
    }
    for (int n = 0; n < window_size; ++n)
    {
      window_buffer[n] = 1.0;
      window_gain_correction += window_buffer[n];
    }
    frequency_bins = window_size / 2 + 1;
    analysis_of_current_input = new fftwf_complex[window_size];
    if (analysis_of_current_input == nullptr) {
      exit(1);
    }
    for (int i = 0; i < window_size; ++i) {
      analysis_of_current_input[i][0] = 0;
      analysis_of_current_input[i][1] = 0;
    }
    padded_input = fftwf_alloc_real(window_size);
    printf("plan b\n");
    forward_plan = fftwf_plan_dft_r2c_1d(
        window_size,
        padded_input,
        analysis_of_current_input,
        FFTW_ESTIMATE);
    printf("plan c\n");
    backward_plan = fftwf_plan_dft_c2r_1d(
        window_size,
        analysis_of_current_input,
        padded_input,
        FFTW_ESTIMATE);

    printf("Initialized Spectral Cutter\n");

  }
  ~SpectralCutter()
  {
    fftwf_destroy_plan(forward_plan);
    fftwf_destroy_plan(backward_plan);
    fftwf_free(analysis_of_current_input);
    fftwf_cleanup();
    free(window_buffer);
  }

  float *padded_input = nullptr;
  void cut_part(float *to_eleminate_from, long len)
  {
    for (int i = 0; i < window_size; ++i) {
      if (i < len) {
        padded_input[i] = to_eleminate_from[i] * window_buffer[i];
      } else {
        padded_input[i] = 0;
      }
    }
    memcpy(&padded_input[0], &to_eleminate_from[0], len * sizeof(float));

    fftwf_execute_dft_r2c(forward_plan, &padded_input[0], &analysis_of_current_input[0]);
    // printf("execute fwd\n");
    // double avg_mag = 0;
    // for (int i = 0; i < frequency_bins; ++i)
    // {
    //   double mag = to_mag(analysis_of_current_input[i][0], analysis_of_current_input[i][1]);
    //   avg_mag += mag;
    // }
    // avg_mag = avg_mag / frequency_bins;
    for (int i = 0; i < frequency_bins; ++i)
    {
      // double phase = to_phase(analysis_of_current_input[i][0], analysis_of_current_input[i][1]);
      double mag = to_mag(analysis_of_current_input[i][0], analysis_of_current_input[i][1]);
      // printf("mag %f\n", mag);
      if (mag < 0.01) {
        analysis_of_current_input[i][0] = 0;
        analysis_of_current_input[i][1] = 0;
      }

    }
    fftwf_execute_dft_c2r(backward_plan, &analysis_of_current_input[0], &padded_input[0]);
    // printf("execute backwd\n");
    for (int i = 0; i < len; ++i) {
      to_eleminate_from[i] = (padded_input[i]) / window_size;
    }
  }

  void cut(float *to_eliminate_from, long len) {
    for (int w = 0; w < len/window_size; ++w) {
      long offset = w * window_size;
      cut_part(&to_eliminate_from[offset], window_size);
    }
    long normal_windows = len/window_size;
    long remainder = len - (normal_windows * window_size);
    if (remainder > 0) {
      long offset = normal_windows * window_size;
      cut_part(&to_eliminate_from[offset], remainder);
    }
  }
  
};