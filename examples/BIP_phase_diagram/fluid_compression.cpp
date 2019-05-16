#include <iostream>
#include "phase_transition.h"
#include "helper_functions.h"

#define STEPS_PER_COMPRESSION 5000
#define PARTICLES 1000
typedef std::vector<double> vec1d;

/* Linux working directory */
std::string dir_linux = ".";


int main() {
  vec1d temperature_array = helper_functions::linspace(0.006, 0.01, 1);
  /* 
   * Adjust the DENSITY, FINAL_DENSITY for the fluid->solid transition and then
   * re-adjust it for the solid->fluid. Otherwise the simulations will sample
   * a lot of unneeded densities.
   */
  for (const auto& i : temperature_array) {
    phase_transition* run1 = new phase_transition(dir_linux, STEPS_PER_COMPRESSION, true);
    run1->crystallisation(0.05, 0.1, 0.01, i, 12, 0, "BIP");
  }
}
