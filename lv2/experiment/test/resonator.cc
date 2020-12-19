#include <sndfile.hh>
#include <alsa/asoundlib.h>
#include "../src/spectral-cutter.cc"
#include "../src/filters.cc"
#include "../src/detector.cc"

#define MAX_CHANNELS 16
#define BUFFER_ELEMENTS 1024

template <class T>
class audiolacer
{
public:
  int channels;
  long frames;
  T *data[MAX_CHANNELS];
  ~audiolacer()
  {
    for (int i = 0; i < channels; ++i)
    {
      free(data[i]);
      data[i] = nullptr;
    }
  }
  audiolacer(int channels, long frames, const T *data)
  {
    this->channels = channels;
    this->frames = frames;
    for (int i = 0; i < channels; ++i)
    {
      this->data[i] = (T *)calloc(frames, sizeof(T));
    }
    for (long f = 0, offset = 0; f < frames; f++, offset += channels)
    {
      for (int i = 0; i < channels; ++i)
      {
        this->data[i][f] = data[offset + i];
      }
    }
  }
  void relace_channel_with_zero(int channel, T* out) {
    for (long i = 0; i < this->frames; ++i) {
      out[i * 2 + 0] = this->data[0][i];
      out[i * 2 + 1] = 0;
    }
  }
};

int main(int argc, char const *argv[])
{
  SndfileHandle file("./assets/base-with-static.wav", SFM_READ);

  const double sample_rate = file.samplerate();
  SndfileHandle output("./assets/test_out_schecter_bass_raw.ogg", SFM_WRITE, SF_FORMAT_OGG | SF_FORMAT_VORBIS, 1, sample_rate);

  const auto frames = file.frames();
  const auto channels = file.channels();
  double buffer[BUFFER_ELEMENTS] = {0.0};
  int read = 0;

  float delay_seconds = 0.1;
  int delay_frames = delay_seconds * sample_rate;
  double* delay_buffer = (double*) calloc(delay_frames, sizeof(double));
  int cur_delay_writer_idx = 0;
  int cur_delay_reader_idx = 1;
  printf("Processing input with sr=%f and frames=%ld and channels=%d into wav file with 1 channel\n", sample_rate, file.frames(), file.channels());

  double frequency = 9000.0;
  double steepness = 0.707;
  double dryness = 0.0;
  double wetness = 1;
  Filter filter = Filter();
  Filter lpf = Filter();
  PassFilterParams params({
    frequency, steepness, dryness, wetness, sample_rate
  });
  filter.configure_hpf2(params);
  
  double lpf_freq = 6000;
  PassFilterParams lpf_params({
    lpf_freq, steepness, dryness, wetness, sample_rate
  });
  lpf_params.steepness = M_PI;
  lpf.configure_lpf2(lpf_params);

  TubeAPre amp = TubeAPre();

  AudioDetectorRMS detector;

  SpectralCutter cutter;
  double empty[] = {0.0};
  bool cutter_initialized = 0;
  

  detector.configure(sample_rate, 2);

  amp.configure(sample_rate, 1.7);

  double pink_frequency = 0.0;
  double phase = 0.0;
  double phase_change = 0.0;
  double phase_accel = 0.015;

  long early_read_max = 3 * sample_rate;
  long early_read_pos = 0;
  int sample_rate_i = sample_rate;
  double *cutter_buffer = (double*) calloc(sizeof(double), sample_rate);
  // while ((read = file.read((double *)&buffer[0], BUFFER_ELEMENTS / channels)) > 0 && early_read_pos < early_read_max) {
  //   audiolacer<double> delacer = audiolacer<double>(channels, read, &buffer[0]);
  //   const long channel_length = read/channels;
  //   memset((void *)&buffer[0], 0, sizeof(buffer));
  //   for (long idx = 0; idx < channel_length; ++idx) {
  //     cutter_buffer[early_read_pos % sample_rate_i] = delacer.data[0][idx];
  //     ++early_read_pos;
  //   }    
  // }
  lpf.reset();
  cutter.initialize(cutter_buffer, sample_rate, sample_rate);
  // discard first three seconds of the file
  while ((read = file.read((double *)&buffer[0], BUFFER_ELEMENTS / channels)) > 0)
  {
    audiolacer<double> delacer = audiolacer<double>(channels, read, &buffer[0]);
    const long channel_length = read/channels;
    memset((void *)&buffer[0], 0, sizeof(buffer));
    for (long idx = 0; idx < channel_length; ++idx) {
      buffer[idx] = delacer.data[0][idx];
    }

    // for (long f = 0; f < channel_length; ++f) {
    //   phase_change += phase_accel;
    //   phase += phase_change;
    //   buffer[f] = sin((phase) / sample_rate);
    // }

    // // do lpf1
    double prev = 0.0;
    double target_min = -30.0;
    double new_target_min = -20.0;
    // cutter.cut(&buffer[0], channel_length);
    for (long f = 0; f < channel_length; ++f) {
      // buffer[f] = amp.processSample(buffer[f]);
      // buffer[f] = filter.processSample(buffer[f] * -1) * 5;
      // buffer[f] = lpf.processSample(buffer[f]);
      //printf("%f\n", );
      buffer[f] = buffer[f] * 100;
      // double gain = detector.processAudioSample(buffer[f]);
      // if (gain < target_min) {
      //   buffer[f] = 0;
      // }
      // buffer[f] = buffer[f] * 0.00001;
      // buffer[f] = buffer[f] * (1.0/0.00001);
      // prev = buffer[f];
      // if (gain < -50) {
      //   buffer[f] = 0;
      // } else {
      // }
      // buffer[f] = lpf.processSample(buffer[f]);

    }


    // introduce delay
    // for (long f = 0; f < read; ++f) {
    //   delay_buffer[cur_delay_writer_idx] = buffer[f];
    //   buffer[f] = (buffer[f] - delay_buffer[cur_delay_reader_idx]) / 2;
    //   cur_delay_reader_idx = (cur_delay_reader_idx + 1) % delay_frames;
    //   cur_delay_writer_idx = (cur_delay_writer_idx + 1) % delay_frames;
    // }
    //

    if (output.writef(&buffer[0], channel_length) == 0) {
      printf("Failed to write %s\n", output.strError());
    }
    memset((void *)&buffer[0], 0, sizeof(buffer));
  }

  output.writeSync();

  return 0;
}
