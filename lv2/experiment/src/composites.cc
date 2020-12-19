#include "./filters.cc";

/**
 * normalize the signal to a known range
 * truncate low db elements
 * effects module:
 *   phaser effect
 *   reverb effect
 * apply wave shaper[s] with gradient coefficient rel to known range
 * finally clean up with 2nd order lpf and clip smoothing, set the freq really high since we just want to anti-alias
 * 
 * */

struct Effect1 {

};