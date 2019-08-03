#include "phase_transition.h"
#include <iostream>

// Load Intel math lib if available
#if defined(__INTEL_COMPILER)
#include <mathimf.h>  // Intel Math library
#define COMPILER "INTEL"
#else
#include <math.h>
#endif

void phase_transition::crystallisation(double DENSITY, double FINAL_DENSITY,
                                       double DENSITY_INC, double TEMPERATURE,
                                       double POWER, double A_CST,
                                       std::string pp_type) {
  /*
   * Compress the fluid to get the phase boundary for a specific temperature.
   * 
   * ceil((FINAL_DENSITY - DENSITY) / DENSITY_INC) compressions of STEPS length
   * Performs repeated compresss of the fluid by periodically
   * incrementing the density of the fluid.
   * As a consequence the box length, the scaling factor and the
   * position vectors are also scaled in order to conserve the number
   * of particles in the box.
   *
   */
  // todo: many features do not work at this moment like particle tracking
  // todo: the POS << time_stamp stream will be a mess, same with RDF
  double current_rho = DENSITY;
  double old_box_length = 0;

  try {
    if (FINAL_DENSITY > DENSITY) {  //todo: add compression rate sensitive case +/- sign
      std::runtime_error(
          "Final density has to be smaller than initial density");
    }
    if (FINAL_DENSITY) {
      std::runtime_error(
          "Density increment has to be smaller than final density");
    }
  } catch (const std::exception &msg) {
    std::cerr << "Error: " << msg.what() << std::endl;
    exit(1);
  }
  
  // Number of compressions to occur
  size_t total_comp_steps = ceil((FINAL_DENSITY - DENSITY) / DENSITY_INC);

  for (size_t comp_step = 1; comp_step < total_comp_steps; comp_step++){

    Simulation(current_rho, TEMPERATURE, POWER, A_CST, pp_type);

    // Holds the box length of the previous simulation just run
    old_box_length = L;

    // Density incrementation
    current_rho += DENSITY_INC;

    // Simulation updates old_box_length
    // the updated current_rho can generate the new box length
    // This value gets recalculated in the next Simulation
    L = pow((N / current_rho), 1.0 / 3.0);
    double box_length_ratio = L / old_box_length;

    // Rescalling the positional vectors
    for (size_t i = 0; i < N; ++i) {
      rx[i] *= box_length_ratio;
      ry[i] *= box_length_ratio;
      rz[i] *= box_length_ratio;
    }
    ++c_counter;
  }

  reset_values(true);
}