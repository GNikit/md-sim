#include "MD.h"
#include "../lib/FileLoading.h"
#define PARTICLES_PER_AXIS 10  // if changed, new vx,vy,vz files need to be generated
#define NHIST 300
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
#define LOAD_DATA_PATH "C:/Users/gn/source/repos/MD-simulation/data"
#define LOAD_POSITIONS LOAD_DATA_PATH"/gaussian"	//TODO: remove gaussian in future
#else
#define LOAD_DATA_PATH "../data"
#define LOAD_POSITIONS LOAD_DATA_PATH"/gaussian"	//TODO: remove gaussian in future
#endif



//TODO: Boltzmann Dist normalisation of the particles velocities in the beggining make it C++
//TODO: Calls to Python script are not multithread safe!

MD::MD(std::string DIRECTORY, size_t run_number) {
	_dir = DIRECTORY;
	_STEPS = run_number;

	Nx = Ny = Nz = PARTICLES_PER_AXIS; // Number of particles per axis
	N = Nx * Ny * Nz;

	gr.resize(NHIST + 1, 0); // gr with Index igr
	fx.resize(N, 0); fy.resize(N, 0); fz.resize(N, 0);
	Cr.reserve(_STEPS);
	msd.reserve(_STEPS);
	u_en.reserve(_STEPS);
	k_en.reserve(_STEPS);
	pc.reserve(_STEPS);
	pk.reserve(_STEPS);
	temperature.reserve(_STEPS);
}

MD::MD(std::string DIRECTORY, size_t run_number, bool QUENCH_F) {
	_dir = DIRECTORY;
	_STEPS = run_number;

	Nx = Ny = Nz = PARTICLES_PER_AXIS; // Number of particles per axis
	N = Nx * Ny * Nz;

	gr.resize(NHIST + 1, 0); // gr with Index igr
	fx.resize(N, 0); fy.resize(N, 0); fz.resize(N, 0);
	Cr.reserve(_STEPS);
	msd.reserve(_STEPS);
	u_en.reserve(_STEPS);
	k_en.reserve(_STEPS);
	pc.reserve(_STEPS);
	pk.reserve(_STEPS);
	temperature.reserve(_STEPS);

	compression_flag = QUENCH_F;
}
MD::~MD() {}


// Methods for MD Analysis
void MD::Initialise(vec1d &x, vec1d &y, vec1d &z,
					vec1d &vx, vec1d &vy, vec1d &vz, double TEMPERATURE) {
	/*
	  Initialises the:
	  + Position Arrays
	  + Velocity Arrays (assign random velocities)
	  + Conserves/ Scales momentum == 0
	  + Temperature
	  + Velocity Autocorrelaion Function

	  @param &x, &y, &z: X, Y, Z vector points
	  @param &vx, &vy, &vz: Vx, Vy, Vz vector points
	  @param TEMPERATURE: Thermostat target temperature
	*/

	// Initialise position matrix and velocity matrix from Cubic Centred Lattice
	if (compression_flag == false) {
		size_t n = 0;
		size_t i, j, k;
		for (i = 0; i < Nx; i++) {
			for (j = 0; j < Ny; j++) {
				for (k = 0; k < Nz; k++) {
					x.push_back((i + 0.5) * scale);
					y.push_back((j + 0.5) * scale);
					z.push_back((k + 0.5) * scale);

					rrx.push_back((i + 0.5) * scale);
					rry.push_back((j + 0.5) * scale);
					rrz.push_back((k + 0.5) * scale);

					++n;
				}
			}
		}
		// Generates Maxwell-Boltzmann dist from Python script
		MBDistribution(TEMPERATURE, true);
	}

	if (compression_flag == true && Q_counter == 0) {
		FileLoading<double> load_data;
		std::string file_name = LOAD_POSITIONS"/Positions_Velocities_particles_" + std::to_string(N) + ".txt";
		std::vector<std::vector<double>> vel =
			load_data.LoadTxt(file_name, 9, '#');
		x = vel[0];
		y = vel[1];
		z = vel[2];
		vx = vel[3];
		vy = vel[4];
		vz = vel[5];
		rrx = vel[0];
		rry = vel[1];
		rrz = vel[2];
		// Start from a highly thermalised fluid state
		// Generation of velocities with T = 10
		// MBDistribution(10);
		// TODO: Not sure what follows is correct in if-loop
		// Temperature calculation for the first step with very high T
		// scale of x, y, z
	}

	// scale of x, y, z
	double mean_vx = 0;
	double mean_vy = 0;
	double mean_vz = 0;

	size_t i;
	// Momentum conservation
	for (i = 0; i < N; i++) {
		mean_vx += vx[i] / N;
		mean_vy += vy[i] / N;
		mean_vz += vz[i] / N;
	}

	size_t tempN = N;
#pragma parallel 
#pragma loop count min(128)
// Subtracting Av. velocities from each particle
	for (i = 0; i < tempN; i++) {
		vx[i] = vx[i] - mean_vx; 
		vy[i] = vy[i] - mean_vy;
		vz[i] = vz[i] - mean_vz;
	}
	// Temperature calculation, statistically
	KE = 0;
	for (i = 0; i < N; i++) {
		KE += 0.5 * (vx[i] * vx[i] + vy[i] * vy[i] + vz[i] * vz[i]);
	}
	T = KE / (1.5 * N);
	scale_v = sqrt(TEMPERATURE / T); // scalling factor

	// Velocity scaling
#pragma parallel
#pragma loop count min(128)
	for (i = 0; i < tempN; i++) {
		vx[i] *= scale_v;
		vy[i] *= scale_v;
		vz[i] *= scale_v;
	}
	// MSD initialasation, storing first positions of particles
	MSDx = x;
	MSDy = y;
	MSDz = z;

	// VAF initialasation, storing first velocities of particles
	Cvx = vx;
	Cvy = vy;
	Cvz = vz;
	double first_val = 0;
	for (i = 0; i < N; i++) {
		first_val += (Cvx[i] * Cvx[i] + Cvy[i] * Cvy[i] + Cvz[i] * Cvz[i]) / N;
	}
	first_val /= N;
	Cr.push_back(first_val);
}

void MD::MBDistribution(double TEMPERATURE, bool run_python_script = false) {
	std::string t = ConvertToString(TEMPERATURE, 4);
	std::string particles = std::to_string(N);
	std::string dir_str = LOAD_DATA_PATH;

	if (run_python_script) {
		// Could be stored as variables and passed into FileNaming
		// rather than repeating the process
		// store in _particles_to_str, _T_to_str
		std::string command = "python " + dir_str + "/MBDistribution.py " + particles + " " + t;
		system(command.c_str());  // Creates files with MD velocities 
	}

	std::string vel_id = "_particles_" + particles + "_T_" + t + ".txt";
	FileLoading<double> obj;
	vx = obj.LoadSingleCol(LOAD_DATA_PATH"/vx" + vel_id);
	vy = obj.LoadSingleCol(LOAD_DATA_PATH"/vy" + vel_id);
	vz = obj.LoadSingleCol(LOAD_DATA_PATH"/vz" + vel_id);
	//TODO: define in heap and delete FileLoading obj
}


void MD::VerletAlgorithm(vec1d &rx, vec1d &ry, vec1d &rz,
						 vec1d &vx, vec1d &vy, vec1d &vz,
						 vec1d &rrx, vec1d &rry, vec1d &rrz) {
	size_t i;
	for (i = 0; i < N; i++) {
		vx[i] = vx[i] * scale_v + fx[i] * dt;
		vy[i] = vy[i] * scale_v + fy[i] * dt;
		vz[i] = vz[i] * scale_v + fz[i] * dt;
		rx[i] = rx[i] + vx[i] * dt;
		ry[i] = ry[i] + vy[i] * dt;
		rz[i] = rz[i] + vz[i] * dt;

		rrx[i] = rrx[i] + vx[i] * dt;
		rry[i] = rry[i] + vy[i] * dt;
		rrz[i] = rrz[i] + vz[i] * dt;

		// Kinetic Energy Calculation
		KE += 0.5 * (vx[i] * vx[i] + vy[i] * vy[i] + vz[i] * vz[i]);

		// Boundary conditions Updated
		if (rx[i] > L) {
			rx[i] = rx[i] - L;
		}
		if (ry[i] > L) {
			ry[i] = ry[i] - L;
		}
		if (rz[i] > L) {
			rz[i] = rz[i] - L;
		}
		if (rx[i] < 0.0) {
			rx[i] = rx[i] + L;
		}
		if (ry[i] < 0.0) {
			ry[i] = ry[i] + L;
		}
		if (rz[i] < 0.0) {
			rz[i] = rz[i] + L;
		}
	}
}

void MD::VelocityAutocorrelationFunction(vec1d &Cvx,
										 vec1d &Cvy,
										 vec1d &Cvz) {
	double temp = 0; // resets every time step
	size_t i;
	for (i = 0; i < N; i++) {
		temp += (Cvx[i] * vx[i] + Cvy[i] * vy[i] + Cvz[i] * vz[i]);
	}
	temp /= N;
	Cr.push_back(temp);	
}

void MD::RadialDistributionFunction(bool normalise) {
	/*normalise by default is TRUE*/
	double R = 0;
	double norm = 1;
	double cor_rho = _rho * (N - 1) / N;
	size_t i;
	for (i = 1; i < NHIST; i++) {  // Changed initial loop value from 0 -> 1
		if (normalise) {
			R = rg * i / NHIST;
			norm = (cor_rho * 2 * PI * R * R * N * _STEPS * dr);
		}
		gr[i] /= norm;	// not really needed
		Hist << gr[i] << std::endl;
	}
}

void MD::MeanSquareDisplacement(vec1d &MSDx,
								vec1d &MSDy,
								vec1d &MSDz) {
	double msd_temp = 0;
	for (size_t i = 0; i < N; ++i) {
		msd_temp += (pow((rrx[i] - MSDx[i]), 2) + pow((rry[i] - MSDy[i]), 2) +
					 pow((rrz[i] - MSDz[i]), 2));
	}
	msd_temp /= N;
	msd.push_back(msd_temp);
}

void MD::DensityCompression(int steps_quench, double TEMPERATURE) {
	// Increase _rho by 0.01
	_rho += 0.01;
	// Re-using this piece of code from MD::Simulation
	scale = pow((N / _rho), (1.0 / 3.0)) / PARTICLES_PER_AXIS;
	L = pow((N / _rho), 1.0 / 3.0);
	Vol = N / _rho;
	Initialise(rx, ry, rz, vx, vy, vz, TEMPERATURE);

}

// MD Simulation
void MD::Simulation(double DENSITY, double TEMPERATURE, int POWER, double A_CST) {
	// Initialise scalling variables
	// If Simulation(...) is not run, _T0, _rho need to be initialised elsewhere
	_T0 = TEMPERATURE;
	_rho = DENSITY;
	dt /= sqrt(_T0);
	// Box length scalling
	scale = pow((N / _rho), (1.0 / 3.0)) / PARTICLES_PER_AXIS;
	L = pow((N / _rho), 1.0 / 3.0);
	Vol = N / _rho;

	// cut_off redefinition
	cut_off = 3.0;//L / 2.;
	rg = cut_off;
	dr = rg / NHIST;

	// Filenaming should not be called
	FileNaming(POWER, A_CST);
	OpenFiles();
	TimeStamp(DATA, "# step \t rho \t U \t K \t Pc \t Pk \t MSD \t VAF");

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	Initialise(rx, ry, rz, vx, vy, vz, TEMPERATURE);

	double xx, yy, zz;
	for (_STEP_INDEX = 0; _STEP_INDEX < _STEPS; _STEP_INDEX++) {
		// Forces loop
		// Resetting forces
		std::fill(fx.begin(), fx.end(), 0);
		std::fill(fy.begin(), fy.end(), 0);
		std::fill(fz.begin(), fz.end(), 0);

		U = 0; // seting Potential U to 0
		PC = 0;

		size_t steps_quench = 5000;	// steps between each quenching
		if (compression_flag == true && _STEP_INDEX != 0 && _STEP_INDEX % steps_quench == 0) {
			++Q_counter;
			DensityCompression(steps_quench, TEMPERATURE);
			std::cout << "compression: " << Q_counter << " rho: " << _rho << std::endl;
		}

		size_t i, j;
		for (i = 0; i < N - 1; i++) {
			for (j = i + 1; j < N; j++) {
				x = rx[i] - rx[j]; // Separation distance
				y = ry[i] - ry[j]; // between particles i and j
				z = rz[i] - rz[j]; // in Cartesian

				xx = x;
				yy = y;
				zz = z;

				// Transposing elements with Periodic BC
				// xx, yy, zz are for the MSD calculation
				if (x > (0.5 * L)) {
					x = x - L;
					xx = xx - L;
				}
				if (y > (0.5 * L)) {
					y = y - L;
					yy = yy - L;
				}
				if (z > (0.5 * L)) {
					z = z - L;
					zz = zz - L;
				}
				if (x < (-0.5 * L)) {
					x = x + L;
					xx = xx + L;
				}
				if (y < (-0.5 * L)) {
					y = y + L;
					yy = yy + L;
				}
				if (z < (-0.5 * L)) {
					z = z + L;
					zz = zz + L;
				}

				r = sqrt((x * x) + (y * y) + (z * z));
				//TODO: enable q for BIP potential
				long double q = sqrt(r * r + A_CST * A_CST);

				// Force loop
				if (r < cut_off) {
					//TODO: implement functionally different potentials, currently 
					//		using comment-uncomment to implement
					// BIP potential of the form: phi = 1/[(r**2 + a**2)**(n/2)]
					//TODO: BIP force
					long double ff =
						(POWER)*r *	pow(q, ((-POWER - 2.0))); // Force for particles

					//TODO: Gausian-force with sigma=1 and epsilon=1
					//long double ff = 2 * r * exp(-r * r);


					fx[i] += x * ff / r;
					fx[j] -= x * ff / r; // Canceling the ij and ji pairs
					fy[i] += y * ff / r; // Taking the lower triangular matrix
					fy[j] -= y * ff / r;
					fz[i] += z * ff / r;
					fz[j] -= z * ff / r;

					PC += r * ff;
					//TODO:Gaussian-Potential configurational Pressure
					// integral not evaluated

					//TODO: Add infinity and edge correction, do same for Pc

					//TODO: BIP potential
					U += pow(q, (-POWER));

					//TODO: Gaussian Potential GCM
					//U += exp(-r * r);

					// Radial Distribution
					igr = round(NHIST * r / rg);
					gr[igr] += 1;
					//rn = (igr - 0.5)*dr;
				}
			}
		}

		// Average Potential Energy per particle
		u_en.push_back(U / N);

		// Average Configurational Pressure Pc
		pc.push_back(PC / (3 * Vol));

		// Isothermal Calibration
		scale_v = sqrt(_T0 / T);	// using T & KE from prev timestep
		KE = 0;  // resetting Kintetic Energy per iteration

		VerletAlgorithm(rx, ry, rz, vx, vy, vz, rrx, rry, rrz);

		MeanSquareDisplacement(MSDx, MSDy, MSDz);

		VelocityAutocorrelationFunction(Cvx, Cvy, Cvz);

		// Average Temperature
		T = KE / (1.5 * N);
		temperature.push_back(T);

		// Kinetic Pressure
		PK = _rho * T;
		pk.push_back(PK);

		// Average Kintetic Energy
		KE /= N;
		k_en.push_back(KE);

		// Density
		density.push_back(_rho);

		//ShowRun(500);  // shows every 500 steps
	}
	// Simulation Ends HERE

	WriteToFiles();
	// Saving Last Position
	TimeStamp(POS, "# X\tY\tZ\tVx\tVy\tVz\tFx\tFy\tFz");
	for (size_t el = 0; el < rx.size(); el++) {
		POS << rx[el] << '\t' << ry[el] << '\t'
			<< rz[el] << '\t' << vx[el] << '\t'
			<< vy[el] << '\t' << vz[el] << '\t'
			<< fx[el] << '\t' << fy[el] << '\t'
			<< fz[el] << std::endl;
	}

	RadialDistributionFunction(true);	// normalisation argument
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout
		<< "CPU run time = "
		<< std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() /
		60
		<< " min "
		<< std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() %
		60
		<< "s" << std::endl;
	// Streams should not close, vectors should not be cleared if object is to be reused
	ResetValues(); // no need to call if object is not reused
}

std::string MD::getDir() {
	/*
	* Returns the directory in a string
	*/
	return _dir;
}

void MD::InitialiseTest(double TEMPERATURE) {
	/*
	  Unit test for the Initialisation method.
	  Tests whether the bool compression_flag trigers the correct loops.
	  If MBDistribution.py performs as expected.
	  If FileLoading.LoadSingleCol() works as expected.
	  If the file directories with preprocessor commands work.
	*/
	// Initialise position matrix and velocity matrix
	//TODO: once the feature is complete remove Q_counter == 0
	if (compression_flag == false) {
		size_t n = 0;
		size_t i, j, k;
		for (i = 0; i < Nx; i++) {
			for (j = 0; j < Ny; j++) {
				for (k = 0; k < Nz; k++) {
					rx.push_back((i + 0.5) * scale);
					ry.push_back((j + 0.5) * scale);
					rz.push_back((k + 0.5) * scale);

					rrx.push_back((i + 0.5) * scale);
					rry.push_back((j + 0.5) * scale);
					rrz.push_back((k + 0.5) * scale);

					++n;
				}
			}
		}
		// Generates Maxwell-Boltzmann dist from Python script
		// Initialieses vx, vy, vz internally
		std::cout << "Compression is not configured" << std::endl;
		MBDistribution(TEMPERATURE);
	}

	if (compression_flag == true) {
		// Start from a highly thermalised fluid state
		std::cout << "High thermalisation in preparation of compression" << std::endl;
		MBDistribution(10);
		// Temperature calculation for the first step with very high T
		// scale of x, y, z
		double mean_vx = 0;
		double mean_vy = 0;
		double mean_vz = 0;

		size_t i;
		// Momentum conservation array
		for (i = 0; i < N; i++) {
			mean_vx += vx[i] / N; // Calculating Average velocity for each dimension
			mean_vy += vy[i] / N;
			mean_vz += vz[i] / N;
		}

		size_t tempN = N; 
#pragma parallel 
#pragma loop count min(128)
		for (i = 0; i < tempN; i++) {
			vx[i] = vx[i] - mean_vx; // Subtracting Av. velocities from each particle
			vy[i] = vy[i] - mean_vy;
			vz[i] = vz[i] - mean_vz;
		}
		// T Calc
		KE = 0;
		for (i = 0; i < N; i++) {
			KE += 0.5 * (vx[i] * vx[i] + vy[i] * vy[i] + vz[i] * vz[i]);
		}
		T = KE / (1.5 * N);
		scale_v = sqrt(10 / T); // scalling factor
	  // Velocity scaling
#pragma parallel
#pragma loop count min(128)
		for (i = 0; i < tempN; i++) {
			vx[i] *= scale_v;
			vy[i] *= scale_v;
			vz[i] *= scale_v;
		}
	}
	// Restoring to normal operation of Initialisation
	// scale of x, y, z
	double mean_vx = 0;
	double mean_vy = 0;
	double mean_vz = 0;

	size_t i;
	// Momentum conservation array
	for (i = 0; i < N; i++) {
		mean_vx += vx[i] / N; // Calculating Average velocity for each dimension
		mean_vy += vy[i] / N;
		mean_vz += vz[i] / N;
	}

	size_t tempN = N; 
#pragma parallel 
#pragma loop count min(128)
	for (i = 0; i < tempN; i++) {
		vx[i] = vx[i] - mean_vx; // Subtracting Av. velocities from each particle
		vy[i] = vy[i] - mean_vy;
		vz[i] = vz[i] - mean_vz;
	}
	// T Calc
	KE = 0;
	for (i = 0; i < N; i++) {
		KE += 0.5 * (vx[i] * vx[i] + vy[i] * vy[i] + vz[i] * vz[i]);
	}
	T = KE / (1.5 * N);
	scale_v = sqrt(TEMPERATURE / T); // scalling factor
	// Velocity scaling
#pragma parallel
#pragma loop count min(128)
	for (i = 0; i < tempN; i++) {
		vx[i] *= scale_v;
		vy[i] *= scale_v;
		vz[i] *= scale_v;
	}

}

// File Handling
void MD::FileNaming(int POWER, double A_cst) {
	/*
	* Generates file names for the different I/O operations
	*/
	std::stringstream A_stream, rho_stream, T_stream;

	T_stream << std::fixed << std::setprecision(4) << _T0;    // 4 decimal
	A_stream << std::fixed << std::setprecision(5) << A_cst;  // 5 decimals
	rho_stream << std::fixed << std::setprecision(4) << _rho;	// 4 decimal

	_step_to_str = "_step_" + std::to_string(_STEPS);
	_particles_to_str = "_particles_" + std::to_string(N);
	_rho_to_str = "_rho_" + rho_stream.str();
	_T_to_str = "_T_" + T_stream.str();
	_n_to_str = "_n_" + std::to_string(POWER);
	_A_to_str = "_A_" + A_stream.str();

	_FILE_ID = _step_to_str + _particles_to_str + _rho_to_str +
		_T_to_str + _n_to_str + _A_to_str;

	// Explicit defitions 
	_FILE_EXT = ".txt";
	data = "Data";
	pos = "Positions_Velocities";
	HIST = "Hist";

	// Path addition
	data = _dir + data + _FILE_ID + _FILE_EXT;
	pos = _dir + pos + _FILE_ID + _FILE_EXT;
	HIST = _dir + HIST + _FILE_ID + _FILE_EXT;
}

void MD::OpenFiles() {
	/*
	* Open/Create if file does not exist
	* Overwrite existing data
	*/
	Hist.open(HIST, std::ios::out | std::ios::trunc);
	DATA.open(data, std::ios::out | std::ios::trunc);
	POS.open(pos, std::ios::out | std::ios::trunc);
}

void MD::WriteToFiles() {
	/*
	* Writes values of parameters to file
	*/
	for (size_t i = 0; i < _STEPS; i++) {
		DATA << (i + 1) << '\t' << density[i] << '\t'
			<< temperature[i] << '\t' << u_en[i] << '\t'
			<< k_en[i] << '\t' << pc[i] << '\t' << pk[i]
			<< '\t' << msd[i] << '\t' << Cr[i] << std::endl;
	}
}

void MD::ShowRun(size_t step_size_show) {
	/*
	* Displays the system parameters every step_size_show of steps
	* Input the increment step
	*/
	if (_STEP_INDEX == 0) {
		std::cout << "step:\tT:\tKE:\tU:\tU+K:\tPC:\tPK:\t(PK+PC):" << std::endl;
	}

	if (_STEP_INDEX % step_size_show == 0 || _STEP_INDEX == 1) {
		std::cout.precision(5);
		std::cout << _STEP_INDEX << "\t" << T << "\t" << KE << "\t" << U << "\t"
			<< (U + KE) << "\t" << PC << "\t" << PK << "\t" << (PK + PC)
			<< std::endl;
	}
}

void MD::ResetValues() {
	/*
	* Closes open file streams and resets sizes and values to 0
	* For multiple simulations
	*/
	// Close streams
	Hist.close();
	DATA.close();
	POS.close();

	// Clear values, size, but reserve capacity
	rx.clear();	ry.clear();	rz.clear();
	rrx.clear();	rry.clear();	rrz.clear();
	vx.clear();	vy.clear();	vz.clear();
	density.clear();
	temperature.clear();
	u_en.clear();	k_en.clear();
	pc.clear();	pk.clear();
	msd.clear();	Cr.clear();
	gr.resize(NHIST + 1, 0); // gr with Index igr
	fx.resize(N, 0);	fy.resize(N, 0);	fz.resize(N, 0);
}

void MD::TimeStamp(std::ofstream& stream, std::string variables) {
	/*
	* Dates the file and allows the input of a header
	* Input a file stream to write and string of characters to display as headers
	*/
	std::chrono::time_point<std::chrono::system_clock> instance;
	instance = std::chrono::system_clock::now();
	std::time_t date_time = std::chrono::system_clock::to_time_t(instance);
	stream << "# Created on: " << std::ctime(&date_time);
	stream << variables << std::endl;
}

std::string MD::ConvertToString(const double & x, const int & precision) {
	static std::ostringstream ss;
	ss.str(std::string()); // don't forget to empty the stream
	ss << std::fixed << std::setprecision(precision) << x;

	return ss.str();
}