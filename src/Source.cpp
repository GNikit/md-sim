#include "MD.h"
// #include "stat_analysis.h"
#include <thread>
#include <string>

#define STEPS 10000

double getRho2(double rho1, double T1, double T2, size_t n) {
  double rho2 = rho1 * pow((T2 / T1), (3.0 / n));
  return rho2;
}

double getA2(double a1, double rho1, double rho2, size_t n) {
  double a2 = a1 * pow((rho1 / rho2), (1.0 / n));
  return a2;
}

std::vector<double> LinearSpacedArray(double a, double b, std::size_t N)
{
  double h = (b - a) / static_cast<double>(N - 1);
  std::vector<double> xs(N);
  std::vector<double>::iterator x;
  double val;
  for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h) {
    *x = val;
  }
  return xs;
}

int main() {
  size_t num = 1;
  std::string dir_windows = "C:/Code/C++/MD simulation/Archives of Data/";  // Current Working Directory
  std::string dir = "";   // Working directory of the cluster
  std::vector<size_t> n = { 6, 8, 10, 12 };
  std::vector<double> rho = { 0.5, 1.0/*, 1.5, 2.0 */}; //TODO: do in sets of 2, do 1.5, 2.0
  std::vector<double> T = { /*0.5,*/ 1.0/*, 1.5, 2.0 */ }; //TODO: do 1.0, 1.5 and 2.0 are running
  //std::vector<double> A1 = { 0, 0.25, 0.50, 0.75, 1.00, 1.25, 1.50, 1.75, 2.00, 2.50, 4.00 };
  std::vector<double> A1 = LinearSpacedArray(0, 1, 5);
  std::vector<double> A2 = LinearSpacedArray(1.25, 2.25, 5);
  std::vector<double> A3 = LinearSpacedArray(2.50, 4.50, 5);  //TODO: do this
  std::vector<double> A4 = LinearSpacedArray(5, 10, 5);       //TODO: do this

  //MD run(dir_windows, STEPS);
  //run.Simulation(0.5, 0.5, 12, 0);

  for (size_t d = 0; d < rho.size()/2; d++) {
    for (size_t t = 0; t < T.size(); t++) {
      for (size_t i = 0; i < n.size(); i++) {
        for (size_t j = 0; j < A1.size(); j++) {
          std::cout << "p: " << n[i] << " A: " << A1[j] << " run num: " << num << std::endl;

          MD* run1 = new MD(dir, STEPS);
          MD* run2 = new MD(dir, STEPS);
          MD* run3 = new MD(dir, STEPS);
          MD* run4 = new MD(dir, STEPS);

          std::thread th1(&MD::Simulation, run1, rho[d], T[t], n[i], A1[j]);
          std::thread th2(&MD::Simulation, run2, rho[d], T[t], n[i], A2[j]);
          std::thread th3(&MD::Simulation, run3, rho[d + (rho.size() / 2)], T[t], n[i], A1[j]);
          std::thread th4(&MD::Simulation, run4, rho[d + (rho.size() / 2)], T[t], n[i], A2[j]);

          th1.join(); th2.join(); th3.join(); th4.join();
          delete run1, run2, run3, run4;

          ++num;
        }
      }
    }
  }
  //std::vector<double> a;
  //a.reserve(A1.size() + A2.size() + A3.size() + A4.size());
  //a.reserve(A1.size() + A2.size());
  //a.insert(a.end(), A1.begin(), A1.end());
  //a.insert(a.end(), A2.begin(), A2.end());
  //a.insert(a.end(), A3.begin(), A3.end());
  //a.insert(a.end(), A4.begin(), A4.end());
  //
  // Stat_Analysis test(dir, A1, STEPS, 1.0, 1000, DENSITY);
  // for (size_t i = 0; i < n.size(); i++) {
  //   test.StaticDataProcessing(n[i]);
  // }
  // system("pause");
}