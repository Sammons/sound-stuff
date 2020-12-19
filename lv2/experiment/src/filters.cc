
#include <math.h>
#include <stdio.h>
#include "./biquad.cc"

struct PassFilterParams
{
  double cutoff_frequency;
  double steepness;
  double dryness;
  double wetness;
  double sample_rate;
  double boost_db;
};

struct Filter
{
  BiquadTransposeCanonical<double> biquad = BiquadTransposeCanonical<double>();
  PassFilterParams params;

public:
  /* From testing the cliff on this is awful, but it does quiet frequencies well above the cutoff */
  Filter()
  {
  }
  // low pass filter
  void configure_lpf1(PassFilterParams &params)
  {
    this->params = params;
    const double theta = 2.0 * M_PI * params.cutoff_frequency / params.sample_rate;
    const double gamma = cos(theta) / (1.0 + sin(theta));
    const double a0 = (1.0 - gamma) / 2.0;
    const double a1 = (1.0 - gamma) / 2.0;
    const double a2 = 0.0;
    const double b1 = -gamma;
    const double b2 = 0.0;
    const double c0 = params.wetness;
    const double d0 = params.dryness;
    printf("Initialized LPF1 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);
    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }
  // high pass filter
  void configure_hpf1(PassFilterParams &params)
  {
    this->params = params;
    const double theta = 2.0 * M_PI * params.cutoff_frequency / params.sample_rate;
    const double gamma = cos(theta) / (1.0 + sin(theta));
    const double a0 = (1.0 + gamma) / 2.0;
    const double a1 = -((1.0 + gamma) / 2.0);
    const double a2 = 0.0;
    const double b1 = -gamma;
    const double b2 = 0.0;
    const double c0 = params.wetness;
    const double d0 = params.dryness;
    printf("Initialized HPF1 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);
    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }
  // high pass filter
  void configure_hpf2(PassFilterParams &params)
  {
    this->params = params;
    const double theta = 2.0 * M_PI * params.cutoff_frequency / params.sample_rate;
    if (params.steepness <= 0)
    {
      params.steepness = 0.707;
    }
    const double d = 1.0 / params.steepness;
    const double beta_numerator = 1.0 - (d / 2.0) * sin(theta);
    const double beta_denominator = 1.0 + (d / 2.0) * sin(theta);
    const double beta = 0.5 * (beta_numerator / beta_denominator);
    const double gamma = (0.5 + beta) * cos(theta);
    const double alpha = (0.5 + beta + gamma) / 2.0;

    const double a0 = alpha;
    const double a1 = -2.0 * alpha;
    const double a2 = alpha;
    const double b1 = -2.0 * gamma;
    const double b2 = 2.0 * beta;

    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized HPF2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }
  // low pass filter
  void configure_lpf2(PassFilterParams &params)
  {
    this->params = params;
    const double theta = 2.0 * M_PI * params.cutoff_frequency / params.sample_rate;
    if (params.steepness <= 0)
    {
      params.steepness = 0.707;
    }
    const double d = 1.0 / params.steepness;
    const double beta_numerator = 1.0 - (d / 2.0) * sin(theta);
    const double beta_denominator = 1.0 + (d / 2.0) * sin(theta);
    const double beta = 0.5 * (beta_numerator / beta_denominator);
    const double gamma = (0.5 + beta) * cos(theta);
    const double alpha = (0.5 + beta - gamma) / 2.0;

    const double a0 = alpha;
    const double a1 = 2.0 * alpha;
    const double a2 = alpha;
    const double b1 = -2.0 * gamma;
    const double b2 = 2.0 * beta;

    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized LPF2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }
  // band pass filter
  void configure_bpf2(PassFilterParams &params)
  {
    this->params = params;
    const double theta = M_PI * params.cutoff_frequency / params.sample_rate;
    const double k = tan(theta);
    const double ksquared = k * k;
    if (params.steepness <= 0)
    {
      params.steepness = 0.707;
    }
    const double delta = ksquared * params.steepness + k + params.steepness;

    const double a0 = k / delta;
    const double a1 = 0.0;
    const double a2 = -k / delta;
    const double b1 = ((2.0 * params.steepness) * (ksquared - 1.0)) / delta;
    const double b2 = (ksquared * params.steepness - k + params.steepness) / delta;

    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized BPF2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  // band stop filter
  void configure_bsf2(PassFilterParams &params)
  {
    this->params = params;
    const double theta = M_PI * params.cutoff_frequency / params.sample_rate;
    const double k = tan(theta);
    const double ksquared = k * k;
    if (params.steepness <= 0)
    {
      params.steepness = 0.707;
    }
    const double delta = ksquared * params.steepness + k + params.steepness;

    const double a0 = (params.steepness * (ksquared + 1)) / delta;
    const double a1 = (2.0 * params.steepness * (ksquared - 1)) / delta;
    const double a2 = (params.steepness * (ksquared + 1)) / delta;
    const double b1 = (2.0 * params.steepness * (ksquared - 1)) / delta;
    const double b2 = ((ksquared * params.steepness) - k + params.steepness) / delta;

    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized BSF2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  // lpf w/ steepness = 0.707
  void configure_butterworth_lpf2(PassFilterParams &params)
  {

    params.steepness = 0.707; // not used but essentially hardcoded

    const double theta = M_PI * params.cutoff_frequency / params.sample_rate;
    const double c = 1.0 / (tan(theta));
    const double csquared = c * c;
    const double a0 = 1.0 / (1.0 + M_SQRT2 + csquared);
    const double a1 = 2.0 * a0;
    const double a2 = a0;
    const double b1 = 2.0 * a0 * (1.0 - csquared);
    const double b2 = a0 * (1 - M_SQRT2 * c + csquared);

    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized Butterworth LPF2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  // hpf w/ steepness = 0.707
  void configure_butterworth_hpf2(PassFilterParams &params)
  {

    params.steepness = 0.707; // not used but essentially hardcoded

    const double theta = M_PI * params.cutoff_frequency / params.sample_rate;
    const double c = tan(theta);
    const double csquared = c * c;
    const double a0 = 1.0 / (1.0 + M_SQRT2 + csquared);
    const double a1 = -2.0 * a0;
    const double a2 = a0;
    const double b1 = 2.0 * a0 * (csquared - 1.0);
    const double b2 = a0 * (1 - M_SQRT2 * c + csquared);

    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized Butterworth HPF2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  // corner freq has -6db and sums out symmetrically with the hpf, good for crossovers
  void configure_linkwitz_riley_lpf2(PassFilterParams &params)
  {
    const double theta = M_PI * params.cutoff_frequency / params.sample_rate;
    const double omega = M_PI * params.cutoff_frequency;
    const double omega_squared = omega * omega;
    const double k = omega / tan(theta);
    const double ksquared = k * k;
    const double delta = ksquared + omega_squared + 2.0 * k * omega;
    const double a0 = omega_squared / delta;
    const double a1 = 2.0 * omega_squared / delta;
    const double a2 = omega_squared / delta;
    const double b1 = (-2.0 * ksquared + 2.0 * omega_squared) / delta;
    const double b2 = (-2.0 * k * omega + ksquared + omega_squared) / delta;
    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized Linkwitz Riley LPF2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  // corner freq has -6db and sums out symmetrically with the lpf, good for crossovers
  void configure_linkwitz_riley_hpf2(PassFilterParams &params)
  {
    const double theta = M_PI * params.cutoff_frequency / params.sample_rate;
    const double omega = M_PI * params.cutoff_frequency;
    const double omega_squared = omega * omega;
    const double k = omega / tan(theta);
    const double ksquared = k * k;
    const double delta = ksquared + omega_squared + 2.0 * k * omega;
    const double a0 = ksquared / delta;
    const double a1 = -2.0 * ksquared / delta;
    const double a2 = ksquared / delta;
    const double b1 = (-2.0 * ksquared + 2.0 * omega_squared) / delta;
    const double b2 = (-2.0 * k * omega + ksquared + omega_squared) / delta;
    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized Linkwitz Riley HPF2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }
  // apf applies phase shift like an LPF but no response change
  void configure_apf1(PassFilterParams &params)
  {
    const double theta = (M_PI * params.cutoff_frequency / params.sample_rate);
    const double alpha = (tan(theta) - 1.0) / (tan(theta) + 1);

    const double a0 = alpha;
    const double a1 = 1.0;
    const double a2 = 0.0;
    const double b1 = alpha;
    const double b2 = 0.0;

    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized APF1 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  // apf applies phase shift like an LPF but no response change
  void configure_apf2(PassFilterParams &params)
  {
    if (params.steepness <= 0)
    {
      params.steepness = 0.707;
    }
    const double bw = params.cutoff_frequency / params.steepness;
    const double bw_theta = (M_PI * bw / params.sample_rate);
    const double alpha = (tan(bw_theta) - 1.0) / (tan(bw_theta) + 1.0);
    const double beta = -cos((2.0 * M_PI * params.cutoff_frequency) / params.sample_rate);
    const double a0 = -alpha;
    const double a1 = beta * (1.0 - alpha);
    const double a2 = 1.0;
    const double b1 = a1;
    const double b2 = a0;

    const double c0 = params.wetness;
    const double d0 = params.dryness;

    printf("Initialized APF2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  void configure_low_shelf1(PassFilterParams &params)
  {
    const double theta = 2.0 * M_PI * params.cutoff_frequency / params.sample_rate;
    const double u = pow((double)10.0, params.boost_db / 20.0); // actually accept db
    const double beta = 4.0 / (1 + u);
    const double delta = beta * tan(theta / 2.0);
    const double gamma = (1.0 - delta) / (1.0 + delta);

    const double a0 = (1.0 - gamma) / 2.0;
    const double a1 = a0;
    const double a2 = 0.0;
    const double b1 = -gamma;
    const double b2 = 0.0;
    const double c0 = u - 1.0;
    const double d0 = 1.0;

    params.wetness = c0;
    params.dryness = d0;

    printf("Initialized Low Shelf1 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f u=%f\n",
           a0, a1, a2, b1, b2, c0, d0, u);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  void configure_high_shelf1(PassFilterParams &params)
  {
    const double theta = 2.0 * M_PI * params.cutoff_frequency / params.sample_rate;
    const double u = pow((double)10.0, params.boost_db / 20.0);
    const double beta = (1.0 + u) / 4.0;
    const double delta = beta * tan(theta / 2.0);
    const double gamma = (1.0 - delta) / (1.0 + delta);

    const double a0 = (1.0 + gamma) / 2.0;
    const double a1 = -a0;
    const double a2 = 0.0;
    const double b1 = -gamma;
    const double b2 = 0.0;
    const double c0 = u - 1.0;
    const double d0 = 1.0;

    params.wetness = c0;
    params.dryness = d0;

    printf("Initialized Low Shelf1 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  void configure_non_constant_eq(PassFilterParams &params)
  {
    const double theta = 2.0 * M_PI * params.cutoff_frequency / params.sample_rate;
    const double u = pow((double)10.0, params.boost_db / 20.0);
    const double epsilon = 4.0 / (1.0 + u);
    const double beta = 0.5 * ((1.0 - epsilon * tan(theta / (2.0 * params.steepness))) /
                               (1.0 + epsilon * tan(theta / (2.0 * params.steepness))));
    const double gamma = (0.5 + beta) * cos(theta);

    const double a0 = 0.5 - beta;
    const double a1 = 0.0;
    const double a2 = -(0.5 - beta);
    const double b1 = -2.0 * gamma;
    const double b2 = 2.0 * beta;
    const double c0 = u - 1.0;
    const double d0 = 1.0;

    params.wetness = c0;
    params.dryness = d0;

    printf("Initialized Low Shelf1 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  void configure_constant_eq_boost(PassFilterParams &params)
  {
    const double theta = M_PI * params.cutoff_frequency / params.sample_rate;
    const double k = tan(theta);
    const double ksquared = k * k;
    const double v = pow(10, params.boost_db / 20.0);
    const double d = (1.0) + (1.0 / params.boost_db) * k + ksquared;
    const double e = (1.0) + (1.0 / (v * params.steepness)) * k + ksquared;
    const double alpha = 1.0 + (v / params.steepness) * k + ksquared;
    const double beta = 2.0 * (ksquared - 1.0);
    const double gamma = 1.0 - (v / params.steepness) * k + ksquared;
    const double delta = 1.0 - (1.0 / params.steepness) * k + ksquared;
    const double rho = 1.0 - (1.0 / (params.steepness * v)) * k + ksquared;

    const double a0 = alpha / d;
    const double a1 = beta / d;
    const double a2 = gamma / d;
    const double b1 = beta / d;
    const double b2 = delta / d;
    const double c0 = 1.0;
    const double d0 = 0.0;

    params.wetness = c0;
    params.dryness = d0;

    printf("Initialized Constant EQ Boost with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }
  void configure_constant_eq_cut(PassFilterParams &params)
  {
    const double theta = M_PI * params.cutoff_frequency / params.sample_rate;
    const double k = tan(theta);
    const double ksquared = k * k;
    const double v = pow(10, params.boost_db / 20.0);
    const double d = (1.0) + (1.0 / params.boost_db) * k + ksquared;
    const double e = (1.0) + (1.0 / (v * params.steepness)) * k + ksquared;
    const double alpha = 1.0 + (v / params.steepness) * k + ksquared;
    const double beta = 2.0 * (ksquared - 1.0);
    const double gamma = 1.0 - (v / params.steepness) * k + ksquared;
    const double delta = 1.0 - (1.0 / params.steepness) * k + ksquared;
    const double rho = 1.0 - (1.0 / (params.steepness * v)) * k + ksquared;

    const double a0 = d / e;
    const double a1 = beta / e;
    const double a2 = delta / e;
    const double b1 = a1;
    const double b2 = rho / e;
    const double c0 = 1.0;
    const double d0 = 0.0;

    params.wetness = c0;
    params.dryness = d0;

    printf("Initialized Constant EQ Boost with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  void configure_all_pole1(PassFilterParams &params)
  {
    const double theta = 2.0 * M_PI * params.cutoff_frequency / params.sample_rate;
    const double gamma = 2.0 - cos(theta);
    const double b1 = sqrt(gamma * gamma - 1.0 - gamma);
    const double a0 = 1.0 + b1;
    const double a1 = 0.0;
    const double a2 = 0.0;
    const double b2 = 0.0;
    const double c0 = 1.0;
    const double d0 = 0.0;

    params.wetness = c0;
    params.dryness = d0;

    printf("Initialized AllPole1 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  // special MMA MIDI 
  void configure_all_pole2(PassFilterParams &params)
  {
    const double theta = 2.0 * M_PI * params.cutoff_frequency / params.sample_rate;
    const double q = params.steepness;
    const double resonance =
        q <= 0.707
            ? 0.0
            : 20 * log10((q * q) / sqrt(q * q - 0.25));
    const double r =
        (cos(theta) + sin(theta) * sqrt(pow(10.0, resonance / 10.0) - 1.0)) /
        (pow(10, resonance / 20.0) * sin(theta) + 1.0);
    const double g = 10.0;
    const double b1 = -2.0 * r * cos(theta);
    const double b2 = r * r;
    const double a0 = g * (1.0 + b1 + b2);
    const double a1 = 0.0;
    const double a2 = 0.0;
    const double c0 = 1.0;
    const double d0 = 0.0;

    params.wetness = c0;
    params.dryness = d0;

    printf("Initialized AllPole2 with a0=%f a1=%f a2=%f b1=%f b2=%f c0=%f d0=%f\n",
           a0, a1, a2, b1, b2, c0, d0);

    this->biquad.set_params({a0, a1, a2, b1, b2, c0, d0});
  }

  double processSample(double xn)
  {
    return (this->params.dryness * xn) +
           (this->params.wetness * this->biquad.process_sample(xn));
  }

  void reset()
  {
    this->biquad.reset();
  }
};

struct TriodeA {
  private:
  PassFilterParams hpf1params;
  PassFilterParams lowshelfparams;
  Filter hpf1;
  Filter lowshelf;
  double saturation;
  bool invert;
  double gain;
  double sample_rate;

  public:
  TriodeA() {}

  void configure(double hpf_fc, double hpf_boost, double lsf_fc, double lsf_boost, double sample_rate, double saturation, bool invert, double gain) {
    this->saturation = saturation;
    this->invert = invert;
    this->gain = gain;
    this->sample_rate = sample_rate;

    this->hpf1params.boost_db = hpf_boost;
    this->hpf1params.sample_rate = sample_rate;
    this->hpf1params.cutoff_frequency = hpf_fc;

    this->lowshelfparams.boost_db = lsf_boost;
    this->lowshelfparams.sample_rate = sample_rate;
    this->lowshelfparams.cutoff_frequency = hpf_fc;

    this->hpf1.configure_hpf1(this->hpf1params);
    this->lowshelf.configure_low_shelf1(this->lowshelfparams);
  }


  inline double sgn(double xn) {
    if (xn < 0) {
      return -1;
    }
    return 1;
  }
  inline double waveShape(double xn) {
    // return (0.5 * xn + abs(xn)) ;
    // return (xn + tanh(0.9 * xn) / tanh(xn)) / 2;
    return abs(xn);
  }

  void reset() {
    this->lowshelf.reset();
    this->hpf1.reset();
  }

  inline double processSample(double xn) {
    double output = this->waveShape(xn);
    if (this->invert) {
      output *= -1;
    }
    // output = (this->hpf1.processSample(output) + this->lowshelf.processSample(output))/2;
    // output = this->hpf1.processSample(output);
    return output;
  }
};

struct TubeAPre {
  TriodeA tubes[6];

  double low_shelf = 2050;
  double triode_low_shelf = 2050;
  double triode_lsf_boost = -3;
  double high_shelf = 10000;
  double high_shelf_gain = -3;
  double drive = 2;
  double output_gain = 0.5;

  Filter lsf;
  Filter hsf;

  public:
  TubeAPre() {}
  void configure(double sample_rate, double saturation) {
    this->high_shelf = sample_rate/2;
    int len = sizeof(tubes) / sizeof(TriodeA);
    for (int i = 0; i < len; ++i) {
      tubes[i] = TriodeA();
      tubes[i].configure(this->high_shelf, this->high_shelf_gain, this->triode_low_shelf, this->triode_lsf_boost, sample_rate, saturation, true, 1);
    }

    PassFilterParams lsf_params;
    lsf_params.cutoff_frequency = sample_rate;
    lsf_params.sample_rate = sample_rate;
    lsf_params.boost_db = 1;
    lsf_params.steepness = 1.707;

    this->lsf.configure_linkwitz_riley_lpf2(lsf_params);

    PassFilterParams hsf_params;
    hsf_params.cutoff_frequency = sample_rate/2;
    hsf_params.sample_rate = sample_rate;
    hsf_params.boost_db = -3;
    this->hsf.configure_high_shelf1(hsf_params);
  }
  inline double processSample(double xn) {
    double output = xn;
    int len = sizeof(tubes) / sizeof(TriodeA);
    for (int i = 0; i < len - 1; ++i) {
      output = tubes[i].processSample(output * 2);
    }
    // output = (this->lsf.processSample(output * 2) + this->hsf.processSample(output * 2)) / 2;
    // output = this->hsf.processSample(output);
    // output = this->lsf.processSample(output);
    output = tubes[len -1].processSample(output);
    return output;
  }
};