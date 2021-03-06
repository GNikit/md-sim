#include <iostream>
#include <string>
#include <vector>
#include "helper_functions.h"
#include "phase_transition.h"

int main() {
  size_t compress_steps = 5000;

  /*
   * This method effectively replaces timestep iterations with
   * density iterations by equating the compression timestep to 1
   *
   */
  phase_transition run(compress_steps, {7, 7, 7}, "FCC");
  run.set_compression_flag(true);
  // todo: fix phase transition
  // run.two_way_compression("compress_", 0.05, 0.15, 0.05, 0.003, 0, 0);
}
