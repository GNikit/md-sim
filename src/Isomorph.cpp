#include "Isomorph.h"

class Isomorph {
  typedef std::vector<double> vec1d;
  vec1d _T, _RHO, _A; // T points
  double _rho_r;      // Reference density
  double _T_r;        // Reference temperature
  double _A_r;        // Reference A
                      /*----------------------------------*/
  double _rho_out;    // Output density
  double _T_out;      // Output temperature
  double _A_out;      // Output A
public:
  Isomorph(double RHO, double T, double Ar, vec1d T_in) {
    /*
    * Takes as arguments a, Reference density, temperature and A parameter
    * T_in is the input Temperature vector, where the isomorph will be placed
    */
    _rho_r = RHO;
    _T_r = T;
    _A_r = Ar;
    _T = T_in;
  }

  double getRho(double rho1, double T1, double T2, size_t n) {
    double rho2 = rho1 * pow((T2 / T1), (3.0 / n));
    return rho2;
  }

  double getA(double a1, double rho1, double rho2, size_t n) {
    double a2 = a1 * pow((rho1 / rho2), (1.0 / 3.0));
    return a2;
  }
  std::tuple<vec1d, vec1d> GenLine(size_t n) {
    /*
    * This method returns a tuple of Isomorphic points
    * for the density and A parameter in the form of a vector.
    */
    for (size_t i = 0; i < _T.size(); i++) {
      _T_out = _T[i];   // reduntant step
      _rho_out = getRho(_rho_r, _T_r, _T_out, n);
      _A_out = getA(_A_r, _rho_r, _rho_out, n);
      _RHO.push_back(_rho_out);
      _A.push_back(_A_out);
    }
    return std::make_tuple(_RHO, _A);
  }
};