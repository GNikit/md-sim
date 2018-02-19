#include "MD.h"
#include "stat_analysis.h"
#include <thread>
#include <string>

#define DENSITY 0.5
#define STEPS 10
#define DIR "../../Archives of Data/Particles_10^3/"
#define DIR0_5 "../../Archives of Data/Particles_10^3/"
#define DIR1 "../../Archives of Data/Particles_10^3/"
//TODO: change Isothermal_step_... TO T_1.4_step_...


//void MultiThread(MD class_object, std::vector<long double> n, std::vector<int> A){
//  for (size_t i = 0; i < n.size(); i++){
//    for (size_t j = 0; j < A.size(); j++){
//      std::cout << "Thread2: p: " << n[i] << " A: " << A[j] << std::endl;
//      class_object.Simulation(n[i], A[j]);
//    }
//  }
//}

int main() {
  size_t num = 1;
  //std::string dir = "/home/gn/Code/C++/MD simulation/Archives of Data/Particles_10^3/";
  std::string dir = DIR;
  std::string file = dir + "Density_0.5/Isothermal_step_" + std::to_string(STEPS) + "/";
  std::vector<int> p = { 6, 8, 10, 12 };
  double a_value = sqrt(0.94387);
  std::vector<double> A1 = {/* 0, 0.25, 0.50, 0.75,*/ a_value, 1.00 };
  std::vector<double> A2 = { 1.25, 1.50, 1.75, 2.00, 3, 4.00 };
  //std::vector<double> A1 = { 0.0, 0.25/*, 0.50, 0.65, 0.7, 0.75 */};
  //std::vector<double> A2 = { 0.5, 0.75/*, 0.90, 0.95, 1.00, 1.05 */};
  //std::vector<double> A3 = { 1., 1.25/*, 1.25, 1.50, 1.75, 2.00 */};
  //std::vector<double> A4 = { 2, 2.5/*, 2.75, 3.0, 3.5, 4.0*/ };
  MD run(DIR0_5, 0.5, DENSITY, STEPS);
  //MD *run2 = new MD(DIR1, 1.0, DENSITY, STEPS);
  //MD *run3 = new MD(DIR, DENSITY, STEPS);
  //MD *run4 = new MD(DIR, DENSITY, STEPS);

  for (size_t i = 0; i < p.size(); i++) {
    for (size_t j = 0; j < A1.size(); j++) {
      std::cout << "p: " << p[i] << " A: " << A1[j] << " run num: " << num << std::endl;
      //std::thread th(&MD::Simulation, run2, p[i], A2[j]);
      //std::thread th2(&MD::Simulation, run3, p[i], A3[j]);
      //std::thread th3(&MD::Simulation, run4, p[i], A4[j]);
      run.Simulation(p[i], A1[j]);
      //th.join();
      //th2.join();
      //th3.join();
      ++num;
    }
  }
  //delete run2;// , run3, run4;
  std::vector<double> a;
  //a.reserve(A1.size() + A2.size() + A3.size() + A4.size());
  a.reserve(A1.size() + A2.size());
  a.insert(a.end(), A1.begin(), A1.end());
  a.insert(a.end(), A2.begin(), A2.end());
  //a.insert(a.end(), A3.begin(), A3.end());
  //a.insert(a.end(), A4.begin(), A4.end());

  Stat_Analysis test(file, a);
  for (size_t i = 0; i < p.size(); i++) {
    test.StaticDataProcessing(p[i]);
  }
  //system("pause");
}