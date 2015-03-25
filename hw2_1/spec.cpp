#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string.h>

using namespace std;

void float_to_binary(float *f){
	unsigned u = *(reinterpret_cast<unsigned*>(f));
	unsigned char c[4] = { (u >> 24), (u >> 16), (u >> 8), u };
	cout << setprecision(30) << *f << "\t" ;
	for (int i = 0; i < 4; i++) {
		int ic = c[i];
		for (int j = 0; j < 8; j++) {
			cout << (bool)(ic & 128);
			ic <<= 1;
		}
		cout << " ";
	}
	cout << endl;
}

void double_to_binary(double *d){
	unsigned u = *(reinterpret_cast<unsigned*>(d));
	unsigned char c[8] = { (u >> 56), (u>>48), (u>>40), (u >> 32), (u >> 24), (u >> 16), (u >> 8), u };
	cout << setprecision(30) << *d << "\t" ;
	for (int i = 0; i < 8; i++) {
		int ic = c[i];
		for (int j = 0; j < 8; j++) {
			cout << (bool)(ic & 128);
			ic <<= 1;
		}
		cout << " ";
	}
	cout << endl;
}
int main(){
	float x = 1.0f;
	float y = 2.0f;
	float z = 16777215.0f;
	float asc = 0, desc = 0;
	double asc_d = 0, desc_d = 0;

	float_to_binary(&x);
	float_to_binary(&y);
	float_to_binary(&z);
	float asc_m = x+y, desc_m = z+y;
	float_to_binary(&asc_m);
	float_to_binary(&desc_m);
	double asc_md = (static_cast<double>(x) + static_cast<double>(y));
	double desc_md = (static_cast<double>(y) + static_cast<double>(y));
	asc = ((x + y) + z);
	desc = ((z + y) + x);
	float_to_binary(&asc);
	float_to_binary(&desc);
	asc_d = ((static_cast<double>(x) + static_cast<double>(y)) + static_cast<double>(z));
	desc_d = ((static_cast<double>(z) + static_cast<double>(y)) + static_cast<double>(x));
	double_to_binary(&asc_d);
	double_to_binary(&desc_d);
	return 0;
}
