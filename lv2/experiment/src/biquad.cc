
// 5 = number of coefficients
struct BiquadParams {
  /**
   * The biquad uses 2 second order components.
   * This means it involves z-1 and z-2 which are two prior samples, on
   * either y(n) or x(n) which are the output and input samples respectively.
   * in terms of math this means a "second order" polynomial (from the exponents)
   * 
   * for a single sample:
   * 
   * y(n) = (a0 * x(n)) + (a1 * (x(n - 1)) + (a2 * (x(n - 2)) - (b1 * y(n - 1)) - (b2 * y(n - 2))
   * x(n - 2) = x(n - 1)
   * x(n - 1) = x(n)
   * y(n - 2) = y(n - 1)
   * y(n - 1) = y(n)
   * 
   * which comes from the transfer function defined in terms of z:
   * 
   * H(z) = a0 * ((1 + a1*z^-1 + a2*z^-2) /
   *              (1 + b1*z^-1 + b2*z^-2))
   * 
   * z = e^(jwt * x) where x = 0, -1, -2 above
   * 
   * to reduce it algebraically euler's is applied
   * 
   * */
  double a0;
  double a1;
  double a2;
  double b1;
  double b2;
  double c0; // wetness
  double d0; // dryness
};

/**
 *  y(x) = transfer(x(n)) 
 * and 
 * 
 *  z refers to the offset from the current sample
 *  z ^ -1 is the previous sample, z ^ -2 is the one before that.
 * 
 * this is because it plugs into e^jwt(# goes here) 
 * so incrementing the -1 to -2, -3, -4 just moves us around the imaginary unit circle
 * 
 * */
enum relative_samples {
  x_z1_idx,
  x_z2_idx,
  y_z1_idx,
  y_z2_idx,
  state_len
};

template <class T> struct BiquadDirect {
  BiquadParams params;
  T state[state_len] = { 0.0 };
  public:
  BiquadDirect() {}
  void set_params(BiquadParams params) {
    this->params = params;
  }
  const T process_sample(const T xn) {
    const T yn = 
      (this->params.a0 * xn) + 
      (this->params.a1 * this->state[x_z1_idx]) + 
      (this->params.a2 * this->state[x_z2_idx]) - 
      (this->params.b1 * this->state[y_z1_idx]) - 
      (this->params.b2 * this->state[y_z2_idx]);
    this->state[x_z2_idx] = this->state[x_z1_idx];
    this->state[x_z1_idx] = xn;
    this->state[y_z2_idx] = this->state[y_z2_idx];
    this->state[y_z1_idx] = yn;
    return yn;
  }
  void reset() {
    this->state[x_z2_idx] = 0.0;
    this->state[x_z1_idx] = 0.0;
    this->state[y_z2_idx] = 0.0;
    this->state[y_z1_idx] = 0.0;
  }
};

template <class T> struct BiquadCanonical {
  BiquadParams params;
  T state[state_len] = { 0.0 };
  BiquadCanonical(const BiquadParams params) {
    this->params = params;
  }
  public:
    const T processAudioSample(const T xn) {
    const T wn = 
      xn - (this->params.b1 * this->state[x_z1_idx]) - (this->params.b2 * this->state[x_z2_idx]);
    const T yn =
      this->params.a0 * wn +
      this->params.a1 * this->state[x_z1_idx] +
      this->params.a2 * this->state[x_z2_idx];
      
    this->state[x_z2_idx] = this->state[x_z1_idx];
    this->state[x_z1_idx] = xn;

    return yn;
  }
  void reset() {
    this->state[x_z2_idx] = 0.0;
    this->state[x_z1_idx] = 0.0;
    this->state[y_z2_idx] = 0.0;
    this->state[y_z1_idx] = 0.0;
  }
};

template <class T> struct BiquadTransposeDirect {
  BiquadParams params;
  T state[state_len] = { 0.0 };
  BiquadTransposeDirect(const BiquadParams params) {
    this->params = params;
  }
  public:
  const T processAudioSample(const T xn) {
    const T wn = this->state[y_z1_idx] + xn;
    const T yn = (wn * this->params.a0) + this->state[x_z1_idx];

    this->state[y_z1_idx] = this->state[y_z2_idx] - this->params.b1 * wn;
    this->state[y_z2_idx] = wn * -this->params.b2;
    this->state[x_z1_idx] = this->state[x_z2_idx] + this->params.a1 * wn;
    this->state[x_z2_idx] = this->params.a2 * wn;

    return yn;
  }
  void reset() {
    this->state[x_z2_idx] = 0.0;
    this->state[x_z1_idx] = 0.0;
    this->state[y_z2_idx] = 0.0;
    this->state[y_z1_idx] = 0.0;
  }
};

template <class T> struct BiquadTransposeCanonical {
  BiquadParams params;
  T state[state_len] = { 0.0 };
  public:
  BiquadTransposeCanonical() {}
  void set_params(BiquadParams params) {
    this->params = params;
  }
  const T process_sample(const T xn) {
    const T yn = xn * this->params.a0 + this->state[x_z1_idx];

    this->state[x_z1_idx] = xn * this->params.a1 - yn * this->params.b1 + this->state[x_z2_idx];
    this->state[x_z2_idx] = xn * this->params.a2 - yn * this->params.b2;

    return yn;
  }
  const void reset() {
    this->state[x_z2_idx] = 0.0;
    this->state[x_z1_idx] = 0.0;
    this->state[y_z2_idx] = 0.0;
    this->state[y_z1_idx] = 0.0;
  }
};