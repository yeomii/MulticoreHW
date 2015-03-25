#include <cstdlib>
#include <iostream>
#include <ctime>
#include <list>
#include <cmath>

using namespace std;

unsigned const int case_number = 10000;

float rand_float(){
	return (static_cast<float>(rand())/ (RAND_MAX) * pow(10.0, (rand() % 40) -20));
}

float add_f(list<float> lst){
	float f = 0;
	for (list<float>::iterator it=lst.begin(); it!=lst.end(); it++){
		f += *it;
	}
	return f;
}

double add_d(list<float> lst){
	double d = 0;
	for (list<float>::iterator it=lst.begin(); it!=lst.end(); it++){
		d += (static_cast<double>(*it));
	}
	return d;
}

int main(){
	list<float> lst;
	srand((unsigned int)time(NULL));
	
	int size, iter;
	cout << "enter size of array : ";
	cin >> size;
	cout << "enter the number of iteration : ";
	cin >> iter;

	float asc_f, desc_f;
	double asc_d, desc_d;
	int asc_n=0, desc_n=0;
	for (int i=0; i<iter; i++){
		for (int j=0; j<size; j++){
			lst.push_back(rand_float());
		}
		lst.sort();
		asc_f = add_f(lst); asc_d = add_d(lst);
		lst.reverse();
		desc_f = add_f(lst); desc_d = add_d(lst);

		if (asc_f != desc_f && asc_d == desc_d){
			double asc_e = abs(static_cast<double>(asc_f) - asc_d);
			double desc_e = abs(static_cast<double>(desc_f) - desc_d);
			if (asc_e > desc_e)
				asc_n++;
			else if (asc_e < desc_e)
				desc_n++;
		}
		lst.clear();
	}
		
	cout << "# of cases where the error of ascending order is bigger : " << asc_n << endl;
	cout << "# of cases where the error of descending order is bigger : " << desc_n << endl;
	return 0;
}
