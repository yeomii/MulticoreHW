#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <vector>
#include <chrono>
#include <time.h>

using namespace std;
using namespace chrono;

float rand_float(bool normal){
	float exp;
	if (normal)
		exp = (rand() % 40) - 20;
	else
		exp = (rand() % 10) - 5;
	return (static_cast<float>(rand()) / (RAND_MAX) * pow(10.0, exp));
}

void accuracy(){
	int n;
	int fma_e=0, def_e=0;
	double fma_error = 0, def_error = 0;
	cout << "enter the number of iterations : ";
	cin >> n;
	for (int i=0; i<n; i++){
		float x = rand_float(true), y = rand_float(true), z = rand_float(true);
		double dx = static_cast<double>(x), dy = static_cast<double>(y), dz = static_cast<double>(z);
		float fma_f = fmaf(x,y,z);
		float def_f = x*y+z;
		double fma_d = fma(dx,dy,dz);
		double def_d = dx*dy+dz;
		double fma_gap = abs(static_cast<double>(fma_f) - fma_d);
		double def_gap = abs(static_cast<double>(def_f) - def_d);
		if (fma_f != def_f && fma_d == def_d){
			if (fma_gap > def_gap)
				fma_e++;
			else if (fma_gap < def_gap)
				def_e++;
		}
		fma_error += (fma_gap/fma_d);
		def_error += (def_gap/def_d);
	}
	cout << "# of bigger error with fma : " << fma_e << endl;
	cout << "# of bigger error without fma : " << def_e << endl;
	cout << "average relative error with fma : " << fma_error / static_cast<double>(n) << endl;
	cout << "average relative error without fma : " << def_error / static_cast<double>(n) << endl;
}

void speed(){
	int n;
	cout << "enter the number of operations : ";
	cin >> n;
	float fma_f=0, def_f=0;
	vector<float> lst;
	for (int i=0; i<n; i++){
		lst.push_back(rand_float(false));
		lst.push_back(rand_float(false));
	}
	auto c1 = high_resolution_clock::now();
	for (int i=0; i<n; i++){
		fma_f = fmaf(lst[2*i], lst[2*i+1], fma_f);
	}
	auto c2 = high_resolution_clock::now();
	for (int i=0; i<n; i++){
		def_f = lst[2*i]*lst[2*i+1] + def_f;
	}
	auto c3 = high_resolution_clock::now();
	cout << fma_f << endl << def_f << endl;
	cout << "duration with fma : " << duration_cast<nanoseconds>(c2-c1).count() << "ns" << endl;
	cout << "duration without fma : " << duration_cast<nanoseconds>(c3-c2).count() << "ns" << endl;
}


int main(){
	srand((unsigned int) time(NULL));

#ifdef FP_FAST_FMA
	cout << setprecision(30) << "FMA enabled" << endl;
#endif

	/* this code is meaningless if compiled with an optimization option */
	
	cout << "accuracy test" << endl;	
	accuracy();

	cout << "speed test" << endl;
	speed();

	return 0;
}
