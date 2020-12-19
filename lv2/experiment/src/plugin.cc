/** 
 * LV2 Plugin for single port in/out dsp effects, testbed
 */
#include <lv2.h>
#include <thread>
#include <vector>
#include <sstream>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "./zmq.cc"

#include "experiment.cc"

#define PLUGIN_URI "http://sammons.io/plugins/experiment"

typedef enum
{
  // Matches TTL index values
  PLUGIN_INPUT = 0,
  PLUGIN_OUTPUT = 1
} PortIndex;

typedef struct
{
  // Matches TTL definition
  // these values represent the bells and whistles
  // exposed to the application
  const float *input;
  float *output;
  double sample_rate;
  ExperimentEffect effect;
  std::thread zmq_thread;
} ModuleContext;

static LV2_Handle
instantiate(const LV2_Descriptor *descriptor,
            double sample_rate,
            const char *bundle_path,
            const LV2_Feature *const *features)
{
  printf("ALLOCATE SPACE FOR PLUGIN\n");
  ModuleContext *converter = (ModuleContext *)calloc(1, sizeof(ModuleContext));
  converter->sample_rate = sample_rate;
  converter->effect = ExperimentEffect();
  printf("CONFIGURE PLUGIN\n");
  converter->effect.configure(sample_rate, 100, 1);
  printf("BOOT ZMQ\n");
  converter->zmq_thread = std::thread(&listen_to_zmq);
  converter->zmq_thread.detach();
  return (LV2_Handle)converter;
}

static void
connect_port(LV2_Handle instance,
             unsigned int port,
             void *data)
{
  // fprintf(stdout, "CONNECT\n");
  ModuleContext *converter = (ModuleContext *)instance;
  switch ((PortIndex)port)
  {
  case PLUGIN_INPUT:
    converter->input = (const float *)data;
    break;
  case PLUGIN_OUTPUT:
    converter->output = (float *)data;
    break;
  }
}

/** reset everything except connect_port */
static void
activate(LV2_Handle instance)
{
  printf("ACTIVATE\n");
  // const ModuleContext *converter = (const ModuleContext *)instance;
}

const std::string noise_gate_name = "Noise Gate";
const std::string delay_name = "Simple Delay";
const std::string amp_name = "Amp";
const std::string active_param_name = "Active";
const std::string threshold_param_name = "Threshold";
const std::string duration_param_name = "Duration";
const std::string gain_param_name = "Gain";

static void populate_vec_with_csv(const std::string *csv, std::vector<std::string> *vec)
{
  std::stringstream s_stream(*csv);
  while (s_stream.good())
  {
    std::string out;
    getline(s_stream, out, ',');
    vec->push_back(out);
  }
}

static void apply_configuration_string(const std::string *str, ModuleContext **converter)
{
  std::vector<std::string> pieces;
  populate_vec_with_csv(str, &pieces);
  if (pieces[0] == noise_gate_name)
  {
    int active = 0;
    float threshold = 0;
    int position = 1;
    while (position < pieces.size())
    {
      if (pieces[position].find_first_of(active_param_name) != pieces[position].npos)
      {
        sscanf(pieces[position].c_str(), "Active=%u", &active);
      }
      if (pieces[position].find_first_of(threshold_param_name) != pieces[position].npos)
      {
        sscanf(pieces[position].c_str(), "Threshold=%f", &threshold);
      }
      position++;
    }
  } else if (pieces[0] == delay_name)
  {
    int active = 0;
    int duration = 0;
    int position = 1;
    while (position < pieces.size())
    {
      if (pieces[position].find_first_of(active_param_name) != pieces[position].npos)
      {
        sscanf(pieces[position].c_str(), "Active=%u", &active);
      }
      if (pieces[position].find_first_of(duration_param_name) != pieces[position].npos)
      {
        sscanf(pieces[position].c_str(), "Duration=%d", &duration);
      }
      position++;
    }
    (*converter)->effect.configure((*converter)->sample_rate, duration, active);
    printf("Updated delay configuration with %d milliseconds, and active=%d\n", duration, active);
  } else if (pieces[0] == amp_name)
  {
    int active = 0;
    int gain = 0;
    int position = 1;
    while (position < pieces.size())
    {
      if (pieces[position].find_first_of(active_param_name) != pieces[position].npos)
      {
        sscanf(pieces[position].c_str(), "Active=%u", &active);
      }
      if (pieces[position].find_first_of(gain_param_name) != pieces[position].npos)
      {
        sscanf(pieces[position].c_str(), "Gain=%d", &gain);
      }
      position++;
    }
  }
}

static void
run(LV2_Handle instance, uint32_t n_samples)
{
  ModuleContext *converter = (ModuleContext *)instance;
  std::string out;
  do
  {
    out = pop_zmq_message();
    if (!out.empty())
    {
      // long ts;
      apply_configuration_string(&out, &converter);
      // printf("received, should be active %d\n", active == 1);
    }
  } while (!out.empty());

  converter->effect.process_frames(converter->input, converter->output, n_samples, converter->sample_rate);
}

static void
deactivate(LV2_Handle instance)
{
  ModuleContext *converter = (ModuleContext *)instance;
  converter->effect.reset_state();
}

static void
cleanup(LV2_Handle instance)
{
  // const ModuleContext *converter = (const ModuleContext *)instance;
}

static const void *
extension_data(const char *uri)
{
  return nullptr;
}

static const LV2_Descriptor descriptor = {
    PLUGIN_URI,
    instantiate,
    connect_port,
    activate,
    run,
    deactivate,
    cleanup,
    extension_data};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  printf("DESCRIPTOR\n");
  switch (index)
  {
  case 0: // TODO: research and explain why they would give us non-0
    return &descriptor;
  default:
    return nullptr;
  }
}