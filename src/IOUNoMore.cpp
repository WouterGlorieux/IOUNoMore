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
#include <list>
#include <algorithm>

using namespace std;

string strHost = string("192.168.1.100");

struct cycle{
	vector<string> vstrCycle;
	float fLCD; 	//lowest common denominator
};


void addNode(string ID, string label, float size );
void addEdge(string ID, string source, string target, float weight );
void changeNode(string ID, string label, float size );
void changeEdge(string ID, string source, string target, float weight );
void deleteNode(string ID);
void deleteEdge(string ID);
void StringExplode(std::string str, std::string separator, std::vector<std::string>* results);
cycle StronglyConnected(string v, string w, float amount);
void cancelOutCycle(cycle cycle);

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
IOU randomIOU();

class Account;

map<string, Account> mapAccounts;

class Account{
public:
	string m_ID;
	multiset<IOU> m_setIOUsDebet;
	multiset<IOU> m_setIOUsCredit;
	float m_fBalance;

	Account(string ID){
		cout << "init called for " << ID << endl;
		m_ID = ID;
		m_fBalance = 0.0;

		map<string, Account>::iterator it;
		it = mapAccounts.find(m_ID);

		if(it == mapAccounts.end()){
			addNode(m_ID, label(), m_fBalance);
			mapAccounts.insert(pair<string, Account>(m_ID, *this));
		}

	}
	~Account(){}

	vector<string> creditors(){
		vector<string> vstrCreditors;
		multiset<IOU>::iterator it;
		string strTmpTarget = "";

		for (it = m_setIOUsDebet.begin();  it != m_setIOUsDebet.end();  it++)
		{
        	if(it->m_strTargetID != strTmpTarget){
        		//cout << "adding " << it->m_strTargetID << " to creditors" << endl;
        		vstrCreditors.push_back(it->m_strTargetID);
        		strTmpTarget = it->m_strTargetID;
        	}
		}
		return vstrCreditors;
	}

	vector<string> debtors(){
		vector<string> vstrDebtors;
		multiset<IOU>::iterator it;
		string strTmpSource = "";

		for (it = m_setIOUsCredit.begin();  it != m_setIOUsCredit.end();  it++)
		{
        	if(it->m_strSourceID != strTmpSource){
        		//cout << "adding " << it->m_strSourceID << " to debtors" << endl;
        		vstrDebtors.push_back(it->m_strSourceID);
        		strTmpSource = it->m_strSourceID;
        	}
		}
		return vstrDebtors;
	}

	float owedTo(string target){
		float fTotal = 0.0;
		multiset<IOU>::iterator it;

		for (it = m_setIOUsDebet.begin();  it != m_setIOUsDebet.end();  it++)
		{
        	if(it->m_strTargetID == target){
        		fTotal += it->m_fAmount;
        	}
		}

		return fTotal;
	}
	float owedFrom(string source){
			float fTotal = 0.0;
			multiset<IOU>::iterator it;

			for (it = m_setIOUsCredit.begin();  it != m_setIOUsCredit.end();  it++)
			{
	        	if(it->m_strSourceID == source){
	        		fTotal += it->m_fAmount;
	        	}
			}

			return fTotal;
	}
	void giveIOU(IOU iou){
		m_setIOUsDebet.insert(iou);
		m_fBalance -= iou.getAmount();

		map<string, Account>::iterator it;
		it = mapAccounts.find(iou.m_strTargetID);

		if(it == mapAccounts.end()){
			Account cAccount = Account(iou.m_strTargetID);
		}

		it = mapAccounts.find(iou.m_strTargetID);

		cout << "account from map: " << it->second.m_ID << " " << it->second.m_fBalance << endl;
		it->second.m_setIOUsCredit.insert(iou);
		it->second.m_fBalance += iou.getAmount();

		if(it->second.owedFrom(iou.m_strTargetID)<=0){
			deleteEdge(iou.m_strTargetID + "-" + iou.m_strSourceID);
		}

		changeNode(it->second.m_ID, it->second.label(), it->second.m_fBalance);

		if(owedTo(iou.m_strTargetID)-owedFrom(iou.m_strTargetID)>0){
			addEdge(m_ID + "-" + iou.m_strTargetID, m_ID, iou.m_strTargetID, owedTo(iou.m_strTargetID)-owedFrom(iou.m_strTargetID));
			changeEdge(m_ID + "-" + iou.m_strTargetID, m_ID, iou.m_strTargetID, owedTo(iou.m_strTargetID)-owedFrom(iou.m_strTargetID));
		}

		changeNode(m_ID, label(), m_fBalance);

		cycle sCycle = StronglyConnected(iou.m_strSourceID, iou.m_strTargetID, iou.m_fAmount);
		while(sCycle.vstrCycle.size() != 0){
			cout << "cycle cancelled out: " << sCycle.fLCD << endl;
			sCycle = StronglyConnected(iou.m_strSourceID, iou.m_strTargetID, iou.m_fAmount);
			cancelOutCycle(sCycle);
		}
		//cout << m_ID << "is strongly connected: " << StronglyConnected(iou.m_strSourceID, iou.m_strTargetID, 0, 5).size() << endl;

	}

	IOU deleteIOU(IOU iou){
			m_fBalance -= iou.getAmount();
			m_setIOUsDebet.erase(iou);
			return iou;
	}

	void cancelOut(string debtor, string creditor, float amount){
		cout << m_ID << " is now cancelling " << amount << " from " << debtor << " and " << creditor << endl;
		multiset<IOU>::iterator it;
		float remainingAmount = amount;
		for (it = m_setIOUsDebet.begin();  it != m_setIOUsDebet.end();  it++)
		{
			cout << "original iou: " << it->m_strSourceID << "->" << it->m_strTargetID << ": " << it->m_fAmount << endl;

			if(it->m_strTargetID == debtor){
        		if(it->m_fAmount >= remainingAmount){
        			IOU iou = deleteIOU(*it);
        			iou.m_fAmount -= remainingAmount;
        			m_setIOUsDebet.insert(iou);
        		}
        		else if(it->m_fAmount < remainingAmount){
        			deleteIOU(*it);
        			remainingAmount -= it->m_fAmount;
        		}

        	}

		}

		remainingAmount = amount;
		for (it = m_setIOUsCredit.begin();  it != m_setIOUsCredit.end();  it++)
		{
			cout << "original iou: " << it->m_strSourceID << "->" << it->m_strTargetID << ": " << it->m_fAmount << endl;
			if(it->m_strSourceID == creditor){
        		if(it->m_fAmount >= remainingAmount){
        			IOU iou = deleteIOU(*it);
        			iou.m_fAmount -= remainingAmount;
        			m_setIOUsCredit.insert(iou);
        		}
        		else if(it->m_fAmount < remainingAmount){
        			deleteIOU(*it);
        			remainingAmount -= it->m_fAmount;
        		}
        	}

		}



	}



	string label(){
		stringstream ss;
		ss << m_ID << ":" << m_fBalance;
		return ss.str();
	}
};

double dRandom1;
double dRandom2;
double dRandom3;
double dRandom4;

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

int nLow = 0;
int nHigh = RAND_MAX;


int main() {

	srand(time(0)); // set initial seed value to system clock

	std::string strOutputFileName = "output.txt";
	std::ofstream output(strOutputFileName.c_str());

	//int nDebts = 100;    				//total number of debts
	int nStatisticGroups = 10; 		//number of groups for statistical purposes



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

	string input;
	vector<string> vstrData;
	vstrData.push_back(string("initial value"));

	vector<IOU> vIOUdefault;

	while(vstrData.at(0) != string("exit")){
		cout << "enter IOU: source amount target" << endl;
		getline(cin, input);

		if(input == ""){
			//continue;
			input = "random 1";
		}

		vstrData.clear();
		StringExplode(input, " ", &vstrData);



		int nIOU = 0; // number of random iou's to generate
		if(vstrData.at(0) == string("random")){
			if(vstrData.size() >= 2){
				nIOU = atoi(vstrData.at(1).c_str());
			}
		}
		else if(vstrData.at(0) == string("default")){
			/*
			vIOUdefault.push_back(IOU(string("A"), string("B"), 1));
			vIOUdefault.push_back(IOU(string("B"), string("C"), 1));
			vIOUdefault.push_back(IOU(string("C"), string("A"), 1));
			*/

			vIOUdefault.push_back(IOU(string("A"), string("D"), 6));
			vIOUdefault.push_back(IOU(string("D"), string("E"), 4));
			vIOUdefault.push_back(IOU(string("E"), string("F"), 7));
			vIOUdefault.push_back(IOU(string("F"), string("A"), 5));

			nIOU = vIOUdefault.size();
		}
		else if(vstrData.size() == 3){
			nIOU = 1;
			if(vstrData.at(0) == vstrData.at(2)){
				cout << "It is not allowed to give an IOU to yourself." << endl;
				continue;
			}
		}

		for(int i = 0; i < nIOU; i++){
			IOU iou = IOU(string(""), string(""), 0);
			if(vstrData.at(0) == string("random")){
				iou = randomIOU();
				if(iou.m_strSourceID == iou.m_strTargetID){
					cout << "source and target are the same.";
					//exit(1);
				}
			}
			else if(vstrData.at(0) == string("default")){
				iou = vIOUdefault.at(i);
			}
			else{
				iou = IOU(vstrData.at(0), vstrData.at(2), atof(vstrData.at(1).c_str()));
			}

			map<string, Account>::iterator it;
			it = mapAccounts.find(iou.m_strSourceID);

			if(it == mapAccounts.end()){
				Account cAccount = Account(iou.m_strSourceID);
			}

			it = mapAccounts.find(iou.m_strSourceID);
			it->second.giveIOU(iou);

			output << iou.m_strSourceID << ";" << iou.m_fAmount << ";" << iou.m_strTargetID << endl;
		}
	}

	//list all accounts with their balance
	for( map<string,Account>::iterator it=mapAccounts.begin(); it!=mapAccounts.end(); ++it)
	{
		Account cAccount = (*it).second;
		cout << (*it).first << " balance: " << cAccount.m_fBalance << endl;
		multiset<IOU>::iterator it2;
		for ( it2 = cAccount.m_setIOUsDebet.begin(); it2 != cAccount.m_setIOUsDebet.end(); ++it2){
			IOU cIOU = *it2;
			cout << "\tgiven to " << cIOU.m_strTargetID << ": " << cIOU.m_fAmount << endl;
		}

		for ( it2 = cAccount.m_setIOUsCredit.begin(); it2 != cAccount.m_setIOUsCredit.end(); ++it2){
			IOU cIOU = *it2;
			cout << "\treceived from " << cIOU.m_strSourceID << ": " << cIOU.m_fAmount << endl;
		}
	}

	return 0;
}

IOU randomIOU(){
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

	if(ssSource.str()==ssTarget.str()){
		ssTarget.str(string("bad target"));
	}

	IOU iou = IOU(ssSource.str(), ssTarget.str(), fAmount);

	return iou;
}

void addNode(string ID, string label, float size = 1 ){
	stringstream ss;

	ss << "curl 'http://" << strHost <<":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"an\":{\"" << ID << "\":{\"label\":\"" << label << "\",\"size\":" << size << "}}}'";
	system(ss.str().c_str());

}

void addEdge(string ID, string source, string target, float weight = 1 ){
	stringstream ss;

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"ae\":{\"" << ID << "\":{\"source\":\"" << source << "\",\"target\":\"" << target << "\",\"directed\":true,\"weight\":" << weight << ",\"label\":\"" << weight << "\"}}}'";
	system(ss.str().c_str());

}

void changeNode(string ID, string label, float size = 1 ){
	stringstream ss;

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"cn\":{\"" << ID << "\":{\"label\":\"" << label << "\",\"size\":" << size << "}}}'";
	system(ss.str().c_str());

}

void changeEdge(string ID, string source, string target, float weight = 1 ){
	stringstream ss;

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"ce\":{\"" << ID << "\":{\"source\":\"" << source << "\",\"target\":\"" << target << "\",\"directed\":true,\"weight\":" << weight << ",\"label\":\"" << weight << "\"}}}'";
	system(ss.str().c_str());

}


void deleteNode(string ID){
	stringstream ss;

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"dn\":{\"" << ID << "\":{}}}'";
	system(ss.str().c_str());
}
void deleteEdge(string ID){
	stringstream ss;

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"de\":{\"" << ID << "\":{}}}'";
	system(ss.str().c_str());
}

cycle StronglyConnected(string v, string w, float amount){
	cycle sCycle;
	sCycle.fLCD = amount;

	bool stronglyConnected = false;
	list<string> listOpen;
	list<string> listClosed;
	vector<string> vstrCycle;

	listOpen.push_back(w);

	string strNode = "";

	map<string, Account>::iterator it;
	while(listOpen.size()>0 && !stronglyConnected){
		list<string>::const_iterator listIt;
		/*cout << v << "->" << w << endl;
		cout << "list open: " ;
		for (listIt = listOpen.begin();  listIt != listOpen.end();  listIt++)
		{
			cout << *listIt << ", ";
		}
		cout << endl << "list closed: ";
		for (listIt = listClosed.begin();  listIt != listClosed.end();  listIt++)
		{
			cout << *listIt << ", ";
		}
		cout << endl;
		 */

		strNode = listOpen.front();
		listOpen.pop_front();
		//cout << v << " open node: " << strNode << endl;
		listClosed.push_back(strNode);

		if(v != strNode){
			//cout << v << " not equal to " << strNode << endl;
			it = mapAccounts.find(strNode);
			vector<string> vstrCreditors;
			vstrCreditors = it->second.creditors();
			//cout << strNode << " has " << vstrCreditors.size() << " creditors" << endl;
			for(unsigned int i=0; i<vstrCreditors.size(); i++){
				if(std::find(listClosed.begin(), listClosed.end(), vstrCreditors.at(i)) == listClosed.end()){
					listOpen.push_back(vstrCreditors.at(i));
				}
			}
		}
		else{
			stronglyConnected = true;
			list<string>::reverse_iterator listIt;


			string strNode = listClosed.back();
			vstrCycle.push_back(strNode);
			for (listIt = listClosed.rbegin();  listIt != listClosed.rend();  listIt++)
			{
				it = mapAccounts.find(strNode);
				vector<string> vstrDebtors;
				vstrDebtors = it->second.debtors();

				if(std::find(vstrDebtors.begin(), vstrDebtors.end(), *listIt)!=vstrDebtors.end()){
					//cout << "found " << *listIt << " among debtors of " << strNode << endl;
					vstrCycle.push_back(*listIt);
					strNode = *listIt;

					float fOwedFrom = it->second.owedFrom(*listIt);
					if(fOwedFrom < sCycle.fLCD){
						sCycle.fLCD = fOwedFrom;
					}

				}
			}

			cout << "found cycle: " ;
			for(unsigned int i = 0; i < vstrCycle.size(); i++){
				cout << vstrCycle.at(i) << ", " ;
			}
			cout << endl;
		}
	}

	if(!stronglyConnected){
		vstrCycle.clear();
	}

	sCycle.vstrCycle = vstrCycle;
	return sCycle;
}

void cancelOutCycle(cycle cycle){
	for(unsigned int i = 0; i < cycle.vstrCycle.size(); i++){
		map<string, Account>::iterator it;
		it = mapAccounts.find(cycle.vstrCycle.at(i));

		if(i == 0){
			it->second.cancelOut(cycle.vstrCycle.at(cycle.vstrCycle.size()-1), cycle.vstrCycle.at(1), cycle.fLCD);
		}
		else if(i == (cycle.vstrCycle.size()-1)){
			it->second.cancelOut(cycle.vstrCycle.at(i-1), cycle.vstrCycle.at(0), cycle.fLCD);
		}
		else{
			it->second.cancelOut(cycle.vstrCycle.at(i-1), cycle.vstrCycle.at(i+1), cycle.fLCD);
		}
	}
}
void StringExplode(std::string str, std::string separator, std::vector<std::string>* results){
    std::size_t found;
    found = str.find_first_of(separator);
    while(found != std::string::npos){
        if(found > 0){
            results->push_back(str.substr(0,found));
        }
        str = str.substr(found+1);
        found = str.find_first_of(separator);
    }
    if(str.length() > 0){
        results->push_back(str);
    }
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



