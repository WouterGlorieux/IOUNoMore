//============================================================================
// Name        : IOUNoMore.cpp
// Author      : Wouter Glorieux
// Version     :
// Copyright   : This code is intended as a proof-of-concept for a website that uses a network of IOU's and tries to cancel out as much IOU's as possible.
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <cstdlib> // for rand() and srand()
#include <ctime> // for time()
#include <math.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>


using namespace std;

float ranf();
float box_muller(float m, float s);

double GaussianDistribution(double x, double mu, double sigma){
	double value = 0.0;

	value = 1/(sigma*sqrt(2*M_PI)) * exp(-1.0/2.0 * pow( ((x-mu)/sigma),2));

	return value;
}


int main() {

	srand(time(0)); // set initial seed value to system clock

	std::string strOutputFileName = "output.txt";
	std::ofstream output(strOutputFileName.c_str());

	int nDebts = 60000;    				//total number of debts
	int nStatisticGroups = 100; 		//number of groups for statistical purposes


	vector<double> vdProbabilities1;		//probabilities for total number of IOU's
	vector<double> vdProbabilitiesCumulative1;
	vector<double> vdProbabilities2;		//probabilities for average amount of each IOU per account
	vector<double> vdProbabilitiesCumulative2;
	vector<double> vdProbabilities3;		//probabilities for random ammount per IOU
	vector<double> vdProbabilitiesCumulative3;



	double cumulativeProbability1 = 0.0;
	double cumulativeProbability2 = 0.0;

	for(int i = 0; i < nStatisticGroups; i++){
		double probability1 = GaussianDistribution((double) i/nStatisticGroups, 0.2, 0.3);
		vdProbabilities1.push_back(probability1);
		double probability2 = GaussianDistribution((double) i/nStatisticGroups, 0.5, 0.1);
		vdProbabilities2.push_back(probability2);

		vdProbabilitiesCumulative1.push_back(cumulativeProbability1);
		cumulativeProbability1 += probability1;
		vdProbabilitiesCumulative2.push_back(cumulativeProbability2);
		cumulativeProbability2 += probability2;
	}

	int nLow = 0;
	int nHigh = RAND_MAX;

	double dRandom1;
	double dRandom2;

	for(int i = 0; i < nDebts; i++){

		stringstream ss;

		dRandom1 = ((double)((rand() % (nHigh - nLow + 1)) + nLow)/nHigh) * cumulativeProbability1 ;
		for(unsigned int i = 0; i < vdProbabilitiesCumulative1.size()-1; i++){
			if(vdProbabilitiesCumulative1.at(i) <= dRandom1 && vdProbabilitiesCumulative1.at(i+1) >= dRandom1 ){
				ss << i+1 << "_" ;
				break;
			}
		}

		dRandom2 = ((double)((rand() % (nHigh - nLow + 1)) + nLow)/nHigh) * cumulativeProbability2 ;
		for(unsigned int i = 0; i < vdProbabilitiesCumulative2.size()-1; i++){
			if(vdProbabilitiesCumulative2.at(i) <= dRandom2 && vdProbabilitiesCumulative2.at(i+1) >= dRandom2 ){
				ss << i+1 << ";";
				float fMedian = i+1;
				float fSigma = 3.0;
				ss << box_muller(fMedian, fSigma) ;


				break;
			}
		}

		output << ss.str() << endl;

	}

	return 0;
}



/***************************************************************************************************************
boxmuller.c Implements the Polar form of the Box-Muller
Transformation

(c) Copyright 1994, Everett F. Carter Jr.
Permission is granted by the author to use
this software for any application provided this
copyright notice is preserved.

*/

/* ranf() is uniform in 0..1 */
float ranf(){

int nLow = 0;
int nHigh = RAND_MAX;
int data = (rand() % (nHigh - nLow + 1)) + nLow;

return (float)data/nHigh;
}


float box_muller(float m, float s) /* normal random variate generator */
{ /* mean m, standard deviation s */
float x1, x2, w, y1;
static float y2;
static int use_last = 0;

if (use_last) /* use value from previous call */
{
y1 = y2;
use_last = 0;
}
else
{
do {
x1 = 2.0 * ranf() - 1.0;
x2 = 2.0 * ranf() - 1.0;
w = x1 * x1 + x2 * x2;
} while ( w >= 1.0 );

w = sqrt( (-2.0 * log( w ) ) / w );
y1 = x1 * w;
y2 = x2 * w;
use_last = 1;
}

return( m + y1 * s );
}

