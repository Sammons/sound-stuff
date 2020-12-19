#include <math.h>
#include <fenv.h>
#include <stdio.h>
// ln(0.368)
#define TC -0.9996723408132061

struct AudioDetectorRMS {
  double attack = 0.0;
  double release = 0.0;
  double sample_rate = 0.0;
  double last_envelope = 0.0;
  void configure(double sample_rate, double attack_ms) {
    this->sample_rate = sample_rate;
    this->attack = exp(TC / (attack_ms * sample_rate * 0.001));
    this->release = exp(TC / (attack_ms * sample_rate * 0.001));
  }
  void test_underflow() {
    // int flags = fetestexcept(FE_UNDERFLOW);
    // if (flags != 0) {
    //   printf("Underflow detected!\n");
    // }
  }
  inline const double processAudioSample(double xn) {
    double xn_modified = fabs(xn);
    // assume RMS
    xn_modified = xn_modified * xn_modified;
    double curr_envelope = xn_modified > last_envelope
      ? attack * (last_envelope - xn_modified) + xn_modified
      : release * (last_envelope - xn_modified) + xn_modified;
    
    test_underflow();

    // not sure if we need clamping?
    curr_envelope = fmin(curr_envelope, 1.0);

    curr_envelope = fmax(curr_envelope, 0.0);


    last_envelope = curr_envelope;

    // rms
    curr_envelope = pow(curr_envelope, 0.5);

    if (curr_envelope <= 0)
      return -96.0;

    // capture for next frame

    // convert to decibels
    return 20.0 * log10(curr_envelope);
  }
};