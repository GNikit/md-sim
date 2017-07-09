#include "MD.h"
//#include "stat_analysis.h"
#include <memory>

using namespace std;


int main() {
	// freopen_s(&stream, "LOG.txt", "a+", stderr);
	size_t steps = 5000;
	vector<double> A_par = { 0.0, 0.25, 0.5,  0.75, 1.0, 1.25, 1.50, 1.75,
								  2.0, 2.25, 2.50, 2.75, 3.0, 3.5,  4.0 };
	vector<size_t> power = {6, 7, 8, 9, 10, 12};
	// cerr << "Log file of the MD simulation" << endl;
	// cerr << "Power:\tA:\tsteps:" << endl;
	size_t num = 1;
	char full = 'y';
	std::string dir = "../../Archives of Data/";
	//if (full == 'y')
	//  {
	//       for (size_t n = 0; n < power.size(); n++)
	// 	   {
	// 		   for (size_t a = 0; a < A_par.size(); a++)
	// 		   {
	// 			   cout << "run: " << num << endl;
	// 			   //srand(time(NULL));
	// 			   std::auto_ptr<MD> run(new MD(dir, 0.5, steps));
	// 			   run->Simulation(power.at(n), A_par.at(a));
	// 			   ++num;
	// 			   //cerr << power.at(n) << "\t" << A_parameter.at(a) << "\t" <<  steps << "rho = 0.8" << endl;
	// 		   }
	// 		   //StaticDataProcessing(power.at(n));
	// 	   }
	// }
	std::vector<int> p{ 6, 8, 10, 12 };
	double density = 0.5;
	
	for (size_t i = 0; i < p.size(); i++) {
		srand(time(NULL));
		MD run(dir, density, steps);
		run.Simulation(p[i], 0);
	}

	//system("pause");
} 

