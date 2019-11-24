#include <iostream>
#include <string>
#include <vector>
#include "helper_functions.h"
#include "phase_transition.h"

int main() {
  std::string dir = "./";  // Places files in the execution dir
  size_t compress_steps = 5000;

  /*
   * This method effectively replaces timestep iterations with
   * density iterations by equating the compression timestep to 1
   *
   */
  phase_transition run(dir, compress_steps, true, 500, 7, "FCC", false, 500);

  std::vector<double> temperatures = {0.001, 0.003, 0.0033, 0.0035, 0.0038};
  for (size_t t = 0; t < temperatures.size(); ++t) {
    run.crystallisation(0.05, 0.30, 0.025, temperatures[t], 0, 0, "GCM");
  }
}