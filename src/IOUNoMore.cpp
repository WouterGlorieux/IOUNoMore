//============================================================================
// Name        : IOUNoMore.cpp
// Author      : Wouter Glorieux
// Version     :
// Copyright   : This code is intended as a proof-of-concept for a website that uses a network of IOU's and tries to cancel out as much IOU's as possible.
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <stdio.h>
#include <cstdlib> // for rand() and srand()
#include <ctime> // for time()
#include <math.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <map>

using namespace std;



void addNode(string ID, string label, float size );
void addEdge(string ID, string source, string target, float weight );
void changeNode(string ID, string label, float size );
//void changeEdge(string ID, string source, string target, float weight );
void deleteNode(string ID);
void deleteEdge(string ID);

float ranf();
float box_muller(float m, float s);

double GaussianDistribution(double x, double mu, double sigma){
	double value = 0.0;

	value = 1/(sigma*sqrt(2*M_PI)) * exp(-1.0/2.0 * pow( ((x-mu)/sigma),2));

	return value;
}

class IOU{
public:
	string m_strSourceID;
	string m_strTargetID;
	float m_fAmount;

	IOU(string sourceID, string targetID, float amount){
		m_strSourceID = sourceID;
		m_strTargetID = targetID;
		m_fAmount = amount;
	}
	~IOU(){}

	bool operator < (const IOU& refParam) const
	{
		if(this->m_strTargetID != refParam.m_strTargetID){
			return (this->m_strTargetID < refParam.m_strTargetID);
		}
		else{
			return (this->m_fAmount < refParam.m_fAmount);
		}
	}


	float getAmount(){ return m_fAmount;}
};

class Account;

map<string, Account> mapAccounts;

class Account{
public:
	string m_ID;
	multiset<IOU> m_setIOUsDebet;
	multiset<IOU> m_setIOUsCredit;
	float m_fBalance;

	Account(string ID){
		m_ID = ID;
		m_fBalance = 0.0;
	}
	~Account(){}

	void giveIOU(IOU iou){
		m_setIOUsDebet.insert(iou);
		m_fBalance -= iou.getAmount();

		map<string, Account>::iterator it;
		it = mapAccounts.find(iou.m_strTargetID);

		if(it == mapAccounts.end()){
			Account cAccount = Account(iou.m_strTargetID);
			addNode(iou.m_strTargetID, iou.m_strTargetID, 1.0);
			cAccount.receiveIOU(iou);
			mapAccounts.insert(pair<string, Account>(iou.m_strTargetID, cAccount));

		}
		else{
			it->second.receiveIOU(iou);
		}


		addEdge(m_ID + "-" + iou.m_strTargetID, m_ID, iou.m_strTargetID, iou.m_fAmount);
		changeNode(m_ID, m_ID, m_fBalance);

	}

	void receiveIOU(IOU iou){
		m_setIOUsCredit.insert(iou);
		m_fBalance += iou.getAmount();
		changeNode(iou.m_strTargetID, iou.m_strTargetID, m_fBalance);
	}
	void deleteIOU(IOU iou){
		m_fBalance -= iou.getAmount();
		m_setIOUsDebet.erase(iou);
	}
};




int main() {

	srand(time(0)); // set initial seed value to system clock

	std::string strOutputFileName = "output.txt";
	std::ofstream output(strOutputFileName.c_str());

	int nDebts = 100;    				//total number of debts
	int nStatisticGroups = 10; 		//number of groups for statistical purposes


	vector<double> vdProbabilities1;		//probabilities for total number of IOU's
	vector<double> vdProbabilitiesCumulative1;
	vector<double> vdProbabilities2;		//probabilities for average amount of each IOU per account
	vector<double> vdProbabilitiesCumulative2;
	vector<double> vdProbabilities3;		//probabilities for random ammount per IOU
	vector<double> vdProbabilitiesCumulative3;
	vector<double> vdProbabilities4;		//probabilities for random ammount per IOU
	vector<double> vdProbabilitiesCumulative4;



	double cumulativeProbability1 = 0.0;
	double cumulativeProbability2 = 0.0;
	double cumulativeProbability3 = 0.0;
	double cumulativeProbability4 = 0.0;

	for(int i = 0; i < nStatisticGroups; i++){
		double probability1 = GaussianDistribution((double) i/nStatisticGroups, 0.2, 0.3);
		vdProbabilities1.push_back(probability1);
		double probability2 = GaussianDistribution((double) i/nStatisticGroups, 0.5, 0.1);
		vdProbabilities2.push_back(probability2);
		double probability3 = GaussianDistribution((double) i/nStatisticGroups, 0.2, 0.3);
		vdProbabilities3.push_back(probability3);
		double probability4 = GaussianDistribution((double) i/nStatisticGroups, 0.5, 0.1);
		vdProbabilities4.push_back(probability4);


		vdProbabilitiesCumulative1.push_back(cumulativeProbability1);
		cumulativeProbability1 += probability1;
		vdProbabilitiesCumulative2.push_back(cumulativeProbability2);
		cumulativeProbability2 += probability2;
		vdProbabilitiesCumulative3.push_back(cumulativeProbability3);
		cumulativeProbability3 += probability3;
		vdProbabilitiesCumulative4.push_back(cumulativeProbability4);
		cumulativeProbability4 += probability4;

	}

	int nLow = 0;
	int nHigh = RAND_MAX;

	double dRandom1;
	double dRandom2;
	double dRandom3;
	double dRandom4;

	for(int i = 0; i < nDebts; i++){

		stringstream ss;
		stringstream ssSource;
		stringstream ssAmount;
		stringstream ssTarget;

		float fAmount = 0.0;

		dRandom1 = ((double)((rand() % (nHigh - nLow + 1)) + nLow)/nHigh) * cumulativeProbability1 ;
		for(unsigned int i = 0; i < vdProbabilitiesCumulative1.size()-1; i++){
			if(vdProbabilitiesCumulative1.at(i) <= dRandom1 && vdProbabilitiesCumulative1.at(i+1) >= dRandom1 ){
				ss << i+1 << "_" ;
				ssSource << i+1 << "_" ;
				break;
			}
		}

		dRandom2 = ((double)((rand() % (nHigh - nLow + 1)) + nLow)/nHigh) * cumulativeProbability2 ;
		for(unsigned int i = 0; i < vdProbabilitiesCumulative2.size()-1; i++){
			if(vdProbabilitiesCumulative2.at(i) <= dRandom2 && vdProbabilitiesCumulative2.at(i+1) >= dRandom2 ){
				ss << i+1 << ";";
				ssSource << i+1;
				float fMedian = i+1;
				float fSigma = 3.0;
				fAmount = box_muller(fMedian, fSigma);

				if(fAmount < 0)
					fAmount = fAmount * -1;

				ss << fAmount << ";" ;
				ssAmount << fAmount ;

				break;
			}
		}

		dRandom3 = ((double)((rand() % (nHigh - nLow + 1)) + nLow)/nHigh) * cumulativeProbability3 ;
		for(unsigned int i = 0; i < vdProbabilitiesCumulative3.size()-1; i++){
			if(vdProbabilitiesCumulative3.at(i) <= dRandom3 && vdProbabilitiesCumulative3.at(i+1) >= dRandom3 ){
				ss << i+1 << "_" ;
				ssTarget << i+1 << "_" ;
				break;
			}
		}

		dRandom4 = ((double)((rand() % (nHigh - nLow + 1)) + nLow)/nHigh) * cumulativeProbability4 ;
		for(unsigned int i = 0; i < vdProbabilitiesCumulative4.size()-1; i++){
			if(vdProbabilitiesCumulative4.at(i) <= dRandom4 && vdProbabilitiesCumulative4.at(i+1) >= dRandom4 ){
				ss << i+1 << ";";
				ssTarget << i+1 ;
				break;
			}
		}

/*
		addNode(ssSource.str(), ssSource.str(), 5.0);
		addNode(ssTarget.str(), ssTarget.str(), 5.0);
		changeNode(ssTarget.str(), ssTarget.str(), 50.0);

		string strEdgeID = ssSource.str() + "-" + ssTarget.str();
		addEdge(strEdgeID, ssSource.str(), ssTarget.str(), fAmount);
*/

		map<string, Account>::iterator it;
		it = mapAccounts.find(ssSource.str());


		if(it == mapAccounts.end()){
			Account cAccount = Account(ssSource.str());
			addNode(ssSource.str(), ssSource.str(), 1.0);
			IOU cIOU = IOU(ssSource.str(), ssTarget.str(), fAmount);
			cAccount.giveIOU(cIOU);
			mapAccounts.insert(pair<string, Account>(ssSource.str(), cAccount));

		}
		else{
			Account cAccount = it->second;
			IOU cIOU = IOU(ssSource.str(), ssTarget.str(), fAmount);
			it->second.giveIOU(cIOU);

		}

		output << ss.str() << endl;



	}

	for( map<string,Account>::iterator it=mapAccounts.begin(); it!=mapAccounts.end(); ++it)
	{
		Account cAccount = (*it).second;
		cout << (*it).first << ": " << cAccount.m_fBalance << endl;
		multiset<IOU>::iterator it2;
		for ( it2 = cAccount.m_setIOUsDebet.begin(); it2 != cAccount.m_setIOUsDebet.end(); ++it2){
			IOU cIOU = *it2;
			cout << "\tDebet: " << cIOU.m_strTargetID << ": " << cIOU.m_fAmount << endl;
		}

		for ( it2 = cAccount.m_setIOUsCredit.begin(); it2 != cAccount.m_setIOUsCredit.end(); ++it2){
			IOU cIOU = *it2;
			cout << "\tCredit: " << cIOU.m_strSourceID << ": " << cIOU.m_fAmount << endl;
		}
	}


	return 0;
}

void addNode(string ID, string label, float size = 1 ){
	stringstream ss;

	ss << "curl 'http://localhost:8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"an\":{\"" << ID << "\":{\"label\":\"" << label << "\",\"size\":" << size << "}}}'";
	system(ss.str().c_str());

}

void addEdge(string ID, string source, string target, float weight = 1 ){
	stringstream ss;

	ss << "curl 'http://localhost:8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"ae\":{\"" << ID << "\":{\"source\":\"" << source << "\",\"target\":\"" << target << "\",\"directed\":true,\"weight\":" << weight << "}}}'";
	system(ss.str().c_str());

}

void changeNode(string ID, string label, float size = 1 ){
	stringstream ss;

	ss << "curl 'http://localhost:8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"cn\":{\"" << ID << "\":{\"label\":\"" << label << "\",\"size\":" << size << "}}}'";
	system(ss.str().c_str());

}

void changeEdge(string ID, float weight = 1 ){
	stringstream ss;

	ss << "curl 'http://localhost:8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"ce\":{\"" << ID << "\":{\"weight\":" << weight << "}}}'";
	system(ss.str().c_str());

}

void deleteNode(string ID){
	stringstream ss;

	ss << "curl 'http://localhost:8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"dn\":{\"" << ID << "\":{}}}'";
	system(ss.str().c_str());
}
void deleteEdge(string ID){
	stringstream ss;

	ss << "curl 'http://localhost:8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"de\":{\"" << ID << "\":{}}}'";
	system(ss.str().c_str());
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

