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
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <ctime>
#include <time.h>

using namespace std;

string strHost = string("192.168.1.100");
bool bEnableGraph = false;
bool bEnableChains = true;
bool bEnableCycles = true;


struct cycle{
	vector<string> vstrCycle;
	long long llLCD; 	//lowest common denominator
};


void addNode(string ID, string label, float size );
void addEdge(string ID, string source, string target, float weight );
void changeNode(string ID, string label, float size );
void changeEdge(string ID, string source, string target, float weight );
void deleteNode(string ID);
void deleteEdge(string ID);
void StringExplode(std::string str, std::string separator, std::vector<std::string>* results);
cycle StronglyConnected(string v, string w, long long amount);
void cancelOutCycle(cycle cycle);


bool validateNetwork();
bool validateOwedFromTo(string a, string b);

float ranf();
float box_muller(float m, float s);

double GaussianDistribution(double x, double mu, double sigma){
	double value = 0.0;

	value = 1/(sigma*sqrt(2*M_PI)) * exp(-1.0/2.0 * pow( ((x-mu)/sigma),2));

	return value;
}

static unsigned long long llIOUID = 1 ;
static long long llTotalIOUamount = 0;
static long long llTotalAmountCancelledOut = 0;


class IOU{


public:
	long long m_llIOUID;
	string m_strSourceID;
	string m_strTargetID;
	long long m_llAmount;

	IOU(string sourceID, string targetID, float amount){
		m_strSourceID = sourceID;
		m_strTargetID = targetID;
		setAmount(amount);
	}
	~IOU(){}

	long long getID(){
		return m_llIOUID;
	}

	void setID(long long IOUID){
		m_llIOUID = IOUID;
	}

	long long getAmount(){
		return m_llAmount;
	}

	void setAmount(long long amount){
		m_llAmount = amount;
	}

	void display(){
		cout << "  IOU " << m_llIOUID << ": "<< m_strSourceID << " owes " << getAmount() << " to " << m_strTargetID << endl;
	}
	bool operator < (const IOU& refParam) const
	{
		if(this->m_strTargetID != refParam.m_strTargetID){
			return (this->m_strTargetID < refParam.m_strTargetID);
		}
		else{
			return (this->m_llAmount < refParam.m_llAmount);
		}
	}

};

void newTransaction(IOU iou	, bool checkCycle);
IOU randomIOU();

vector<cycle> possibleDebtorChains(string source);
void optimalCycle(string source, string target, long long amount);
long long profit(cycle cycle);

class Account;

map<string, Account> mapAccounts;

class Account{
private:
	long long m_llBalance;
public:
	string m_ID;
	multiset<IOU> m_setIOUsGiven;
	multiset<IOU> m_setIOUsReceived;
	//float m_fBalance;

	Account(string ID){
		//cout << "init called for " << ID << endl;
		m_ID = ID;
		setBalance(0);

		map<string, Account>::iterator it;
		it = mapAccounts.find(m_ID);

		if(it == mapAccounts.end()){
			if(bEnableGraph){
				addNode(m_ID, label(), getBalance());
			}
			mapAccounts.insert(pair<string, Account>(m_ID, *this));
		}

	}
	~Account(){}

	long long balance(){
		long long llBalance=0;
		long long llDebet=0;
		long long llCredit=0;

		multiset<IOU>::iterator it;
		for (it = m_setIOUsGiven.begin();  it != m_setIOUsGiven.end();  it++)
		{
			llDebet += it->m_llAmount;
		}
		for (it = m_setIOUsReceived.begin();  it != m_setIOUsReceived.end();  it++)
		{
			llCredit += it->m_llAmount;
		}
		llBalance = llCredit -llDebet;

		/*if(llBalance < 0 && m_ID != string("IOU")){
			cout << "account balance check failed!" << endl;
			cout << "balance of " << m_ID << "= " << llBalance << endl;
			cout << llCredit << " " << llDebet << endl;
 			char ch;
			//cin >> ch;
		}*/
		setBalance(llBalance);
		return llBalance;
	}

	long long getBalance(){
		return m_llBalance;
	}

	void setBalance(long long balance){
		m_llBalance = balance;

	}

	vector<string> creditors(){
		vector<string> vstrCreditors;
		multiset<IOU>::iterator it;
		string strTmpTarget = "";

		for (it = m_setIOUsGiven.begin();  it != m_setIOUsGiven.end();  it++)
		{
        	if(it->m_strTargetID != strTmpTarget && (owedTo(it->m_strTargetID)-owedFrom(it->m_strTargetID)) > 0){
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

		for (it = m_setIOUsReceived.begin();  it != m_setIOUsReceived.end();  it++)
		{

			if(it->m_strSourceID != string("IOU") && it->m_strSourceID != strTmpSource && (owedFrom(it->m_strSourceID)-owedTo(it->m_strSourceID)) > 0){
        		//cout << "adding " << it->m_strSourceID << " to debtors" << endl;
        		vstrDebtors.push_back(it->m_strSourceID);
        		strTmpSource = it->m_strSourceID;
        	}
		}
		return vstrDebtors;
	}

	string maxDebtor(){
		multiset<IOU>::iterator it;
		string strTmpSource = "";
		long long maxOwedFrom  = 0;
		string maxDebtor = "";

		for (it = m_setIOUsReceived.begin();  it != m_setIOUsReceived.end();  it++)
		{
			if(it->m_strSourceID != strTmpSource && (owedFrom(it->m_strSourceID)-owedTo(it->m_strSourceID)) > maxOwedFrom){
        		strTmpSource = it->m_strSourceID;
        		maxOwedFrom = owedFrom(it->m_strSourceID)-owedTo(it->m_strSourceID);
        		maxDebtor = it->m_strSourceID;
			}
		}

		return maxDebtor;
	}

	long long owedTo(string target){
		long long llTotal = 0.0;
		multiset<IOU>::iterator it;

		for (it = m_setIOUsGiven.begin();  it != m_setIOUsGiven.end();  it++)
		{
        	if(it->m_strTargetID == target){
        		llTotal += it->m_llAmount;
        	}
		}

		return llTotal;
	}
	long long owedFrom(string source){
			long long llTotal = 0.0;
			multiset<IOU>::iterator it;

			for (it = m_setIOUsReceived.begin();  it != m_setIOUsReceived.end();  it++)
			{
	        	if(it->m_strSourceID == source){
	        		llTotal += it->m_llAmount;
	        	}
			}

			return llTotal;
	}
	void giveIOU(IOU iou, bool checkCycle = true){
		m_setIOUsGiven.insert(iou);
		//m_fBalance -= iou.getAmount();

		map<string, Account>::iterator it;
		it = mapAccounts.find(iou.m_strTargetID);

		if(it == mapAccounts.end()){
			Account cAccount = Account(iou.m_strTargetID);
		}

		it = mapAccounts.find(iou.m_strTargetID);

		//cout << "account from map: " << it->second.m_ID << " " << it->second.m_fBalance << endl;
		it->second.m_setIOUsReceived.insert(iou);
		it->second.setBalance(it->second.getBalance() + iou.getAmount());

		if(it->second.owedFrom(iou.m_strTargetID)<=0 && bEnableGraph){
			deleteEdge(iou.m_strTargetID + "-" + iou.m_strSourceID);
		}

		if(bEnableGraph){
			changeNode(it->second.m_ID, it->second.label(), it->second.getBalance());
		}
		if(owedTo(iou.m_strTargetID)-owedFrom(iou.m_strTargetID)>0 && bEnableGraph){
			addEdge(m_ID + "-" + iou.m_strTargetID, m_ID, iou.m_strTargetID, owedTo(iou.m_strTargetID)-owedFrom(iou.m_strTargetID));
			changeEdge(m_ID + "-" + iou.m_strTargetID, m_ID, iou.m_strTargetID, owedTo(iou.m_strTargetID)-owedFrom(iou.m_strTargetID));
		}


		if(checkCycle && bEnableCycles){
			long long llCanceledOut = 0;
			cycle sCycle = StronglyConnected(iou.m_strSourceID, iou.m_strTargetID, iou.getAmount());
			while(sCycle.vstrCycle.size() >= 2 && llCanceledOut < iou.getAmount()){
				cout << "\tdetected cycle: " ;
				for(unsigned int i = 0; i < sCycle.vstrCycle.size(); i++){
					cout << sCycle.vstrCycle.at(i) << ", " ;
				}
				cout << " LCD: " << sCycle.llLCD  << "\tTotal value: " << sCycle.llLCD * sCycle.vstrCycle.size() << endl;

				cancelOutCycle(sCycle);
				llCanceledOut += sCycle.llLCD;
				llTotalAmountCancelledOut += sCycle.llLCD * sCycle.vstrCycle.size();
				cout << "\t\t" << llCanceledOut << " of " << iou.getAmount() << " has been cancelled out." << endl;

				sCycle = StronglyConnected(iou.m_strSourceID, iou.m_strTargetID, iou.getAmount()-llCanceledOut);
			}

		//cout << m_ID << "is strongly connected: " << StronglyConnected(iou.m_strSourceID, iou.m_strTargetID, 0, 5).size() << endl;

			balance();
			if(bEnableGraph){
				changeNode(m_ID, label(), getBalance());
			}
			float fRandom = ranf();

			if(bEnableChains && fRandom < 0.1 && iou.m_strSourceID != string("IOU") && checkCycle){
				optimalCycle(iou.m_strSourceID, iou.m_strTargetID, iou.m_llAmount-llCanceledOut);
			}

		}
		//cout << "total amount: " << llTotalIOUamount << endl;
		//cout << "total cancelled out: " << llTotalAmountCancelledOut << endl;

	}



	void reduceIOUfrom(string source, long long amount){
		multiset<IOU>::iterator it;
		map<string, Account>::iterator itAccount;

		long long remainingAmount = amount;
		for (it = m_setIOUsReceived.begin();  it != m_setIOUsReceived.end();  it++)
		{
			if(it->m_strSourceID == source && remainingAmount > 0){
				if(it->m_llAmount >= remainingAmount){
					IOU iou = *it;
					iou.setAmount(iou.getAmount() - remainingAmount);
					itAccount = mapAccounts.find(iou.m_strSourceID);
					itAccount->second.reduceIOUto(iou.getID(), remainingAmount);
					remainingAmount = 0;
					if(bEnableGraph){
						deleteEdge(iou.m_strSourceID + "-" + iou.m_strTargetID);
					}
					m_setIOUsReceived.erase(it);
        			if(iou.getAmount() > 0){
        				m_setIOUsReceived.insert(iou);
        				if(bEnableGraph){
        					addEdge(iou.m_strSourceID + "-" + iou.m_strTargetID, iou.m_strSourceID, iou.m_strTargetID, owedFrom(iou.m_strSourceID));
        				}
        				it = m_setIOUsReceived.begin();
        			}


        		}
        		else if(it->m_llAmount < remainingAmount){
        			remainingAmount -= it->m_llAmount;
        			itAccount = mapAccounts.find(it->m_strSourceID);
        		    itAccount->second.reduceIOUto(it->m_llIOUID, it->m_llAmount);

        			string strEdge = it->m_strSourceID + "-" + it->m_strTargetID;
        			if(bEnableGraph){
        				deleteEdge(strEdge);
        			}
        			m_setIOUsReceived.erase(it);
        		}
        	}
		}
		balance();
	}

	void reduceIOUto(long long IOUID, long long amount){
		multiset<IOU>::iterator it;
		for (it = m_setIOUsGiven.begin();  it != m_setIOUsGiven.end();  it++)
		{
			if(it->m_llIOUID == IOUID && amount > 0){
				IOU iou = *it;
				iou.setAmount(iou.getAmount() - amount);
				m_setIOUsGiven.erase(it);
				if(iou.getAmount() > 0){
					m_setIOUsGiven.insert(iou);
				}
				break;
        	}
		}
		balance();
	}

	string label(){
		stringstream ss;
		ss << m_ID << ":" << getBalance();
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

	std::string strAnalysisFileName = "analysis.txt";
	std::ofstream analysis(strAnalysisFileName.c_str());


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
		vIOUdefault.push_back(IOU(string("A"), string("B"), 1));
		//vIOUdefault.push_back(IOU(string("A"), string("B"), 1));
		//vIOUdefault.push_back(IOU(string("A"), string("B"), 1));
		vIOUdefault.push_back(IOU(string("B"), string("C"), 1));
		vIOUdefault.push_back(IOU(string("B"), string("C"), 1));
		//vIOUdefault.push_back(IOU(string("B"), string("C"), 1));
		//vIOUdefault.push_back(IOU(string("B"), string("C"), 1));
		vIOUdefault.push_back(IOU(string("C"), string("A"), 1));
		vIOUdefault.push_back(IOU(string("C"), string("A"), 1));


		vIOUdefault.push_back(IOU(string("A"), string("B"), 7));
		vIOUdefault.push_back(IOU(string("B"), string("C"), 6));
		vIOUdefault.push_back(IOU(string("C"), string("D"), 5));
		vIOUdefault.push_back(IOU(string("D"), string("E"), 4));
		vIOUdefault.push_back(IOU(string("E"), string("C"), 3));
		vIOUdefault.push_back(IOU(string("C"), string("F"), 2));
		vIOUdefault.push_back(IOU(string("F"), string("A"), 1));

*/

			vIOUdefault.push_back(IOU(string("A"), string("B"), 100));
			vIOUdefault.push_back(IOU(string("B"), string("C"), 100));
			//vIOUdefault.push_back(IOU(string("C"), string("D"), 100));
			vIOUdefault.push_back(IOU(string("D"), string("E"), 100));
			vIOUdefault.push_back(IOU(string("E"), string("F"), 100));
			vIOUdefault.push_back(IOU(string("C"), string("D"), 100));
			//vIOUdefault.push_back(IOU(string("Amajor"), string("B"), 300));
			//vIOUdefault.push_back(IOU(string("F"), string("G"), 200));
			/*
			vIOUdefault.push_back(IOU(string("D"), string("E"), 1));
			vIOUdefault.push_back(IOU(string("E"), string("F"), 1));
			vIOUdefault.push_back(IOU(string("F"), string("G"), 1));
			vIOUdefault.push_back(IOU(string("G"), string("D"), 1));

			vIOUdefault.push_back(IOU(string("A"), string("B"), 1));
			vIOUdefault.push_back(IOU(string("B"), string("C"), 1));
			vIOUdefault.push_back(IOU(string("A"), string("D"), 1));
			vIOUdefault.push_back(IOU(string("D"), string("C"), 1));
			vIOUdefault.push_back(IOU(string("C"), string("A"), 2));

			vIOUdefault.push_back(IOU(string("A"), string("B"), 2.81853));
			vIOUdefault.push_back(IOU(string("C"), string("D"), 6.10824));
			vIOUdefault.push_back(IOU(string("E"), string("F"), 8.73577));
			vIOUdefault.push_back(IOU(string("F"), string("G"), 5.57332));

			vIOUdefault.push_back(IOU(string("2_6"), string("5_7"), 2.924));
			vIOUdefault.push_back(IOU(string("5_7"), string("3_7"), 2.954));
			vIOUdefault.push_back(IOU(string("2_6"), string("5_6"), 2.404));
			vIOUdefault.push_back(IOU(string("5_6"), string("3_7"), 5.904));
			vIOUdefault.push_back(IOU(string("2_6"), string("4_5"), 1.94));
			vIOUdefault.push_back(IOU(string("4_5"), string("3_7"), 2.804));
			vIOUdefault.push_back(IOU(string("2_6"), string("3_5"), 8.904));
			vIOUdefault.push_back(IOU(string("3_5"), string("3_7"), 1.904));
			vIOUdefault.push_back(IOU(string("2_6"), string("7_7"), 2.934));
			vIOUdefault.push_back(IOU(string("7_7"), string("5_7"), 4.904));
			vIOUdefault.push_back(IOU(string("5_7"), string("3_7"), 10.904));
			vIOUdefault.push_back(IOU(string("2_6"), string("4_5"), 2.4));
			vIOUdefault.push_back(IOU(string("4_5"), string("5_7"), 7.943));
			vIOUdefault.push_back(IOU(string("5_7"), string("3_7"), 2.604));
			vIOUdefault.push_back(IOU(string("2_6"), string("2_7"), 2.964));
			vIOUdefault.push_back(IOU(string("2_7"), string("6_5"), 9.904));
			vIOUdefault.push_back(IOU(string("6_5"), string("2_7"), 0.904));
			vIOUdefault.push_back(IOU(string("2_7"), string("5_6"), 2.104));
			vIOUdefault.push_back(IOU(string("5_6"), string("3_7"), 0.04));
			vIOUdefault.push_back(IOU(string("2_6"), string("1_6"), 2.903));
			vIOUdefault.push_back(IOU(string("1_6"), string("4_7"), 2.905));
			vIOUdefault.push_back(IOU(string("4_7"), string("3_7"), 2.994));
			vIOUdefault.push_back(IOU(string("2_6"), string("2_7"), 2.924));
			vIOUdefault.push_back(IOU(string("2_7"), string("3_5"), 6.04));
			vIOUdefault.push_back(IOU(string("3_5"), string("3_7"), 2.204));
			vIOUdefault.push_back(IOU(string("2_6"), string("1_6"), 8.903));
			vIOUdefault.push_back(IOU(string("1_6"), string("4_7"), 3.905));
			vIOUdefault.push_back(IOU(string("4_7"), string("8_6"), 7.904));
			vIOUdefault.push_back(IOU(string("8_6"), string("8_7"), 9.904));
			vIOUdefault.push_back(IOU(string("8_7"), string("3_7"), 10.904));
			vIOUdefault.push_back(IOU(string("2_6"), string("3_7"), 6.904));
*/


			nIOU = vIOUdefault.size();
		}
		else if(vstrData.size() == 3){
			nIOU = 1;
			if(vstrData.at(0) == vstrData.at(2)){
				cout << "It is not allowed to give an IOU to yourself." << endl;
				continue;
			}
		}


		clock_t start, finish;
		for(int i = 0; i < nIOU; i++){
			start = clock();
			long long llOldIOUID = llIOUID;
			IOU iou = IOU(string(""), string(""), 0);
			if(vstrData.at(0) == string("random")){
				iou = randomIOU();
			}
			else if(vstrData.at(0) == string("default")){
				iou = vIOUdefault.at(i);
			}
			else{
				iou = IOU(vstrData.at(0), vstrData.at(2), atof(vstrData.at(1).c_str()));
			}

			if(iou.m_strSourceID == iou.m_strTargetID){
				i--;
				continue;
			}

			cout << endl << i+1 << ": ";

			if(iou.getAmount() != 0){
				newTransaction(iou, true);
				output << iou.m_strSourceID << ";" << iou.getAmount() << ";" << iou.m_strTargetID << endl;

				finish = clock();


				analysis << i+1 << ";";
				analysis << llTotalIOUamount << ";" ;
				analysis << llTotalAmountCancelledOut << ";" ;
				analysis << (float)llTotalAmountCancelledOut/llTotalIOUamount << ";" ;
				analysis << llIOUID-llOldIOUID << ";";
				analysis << (double) (finish - start)/CLOCKS_PER_SEC << ";" ;
				analysis << endl;

			}
			else{
				i--;
				continue;
			}


		}
	}

	//list all accounts with their balance
	validateNetwork();
	long long totalBalance = 0;
	for( map<string,Account>::iterator it=mapAccounts.begin(); it!=mapAccounts.end(); ++it)
	{
		Account cAccount = (*it).second;
		cout << (*it).first << " balance: " << cAccount.getBalance() << endl;
		multiset<IOU>::iterator it2;
		for ( it2 = cAccount.m_setIOUsGiven.begin(); it2 != cAccount.m_setIOUsGiven.end(); ++it2){
			IOU cIOU = *it2;
			cout << "\tIOU: " << cIOU.getID() << " given to " << cIOU.m_strTargetID << ": " << cIOU.getAmount() << endl;
		}

		for ( it2 = cAccount.m_setIOUsReceived.begin(); it2 != cAccount.m_setIOUsReceived.end(); ++it2){
			IOU cIOU = *it2;
			cout << "\tIOU: " << cIOU.getID() << " received from " << cIOU.m_strSourceID << ": " << cIOU.getAmount() << endl;
		}

		if(cAccount.m_ID != string("IOU")){
			totalBalance += cAccount.getBalance();
		}
	}


	cout << endl << "Total of all balances: " << totalBalance << endl;
	map<string,Account>::iterator it=mapAccounts.find(string("IOU"));
	cout << "Balance of IOU: " << it->second.getBalance() << endl;
	return 0;
}

void newTransaction(IOU iou, bool checkCycle){

	if(iou.m_strSourceID != string("IOU")){
		cout << "New transaction: " << endl;
		llTotalIOUamount += iou.m_llAmount;
	}

	iou.setID(llIOUID++);

	//float fRounded = floor(iou.m_fAmount * 100) / 100;
	//iou.m_fAmount = fRounded;
		iou.display();

	map<string, Account>::iterator it;
	it = mapAccounts.find(iou.m_strSourceID);

	if(it == mapAccounts.end()){
		Account cAccount = Account(iou.m_strSourceID);
	}
	it = mapAccounts.find(iou.m_strSourceID);

	if(it->second.balance() < iou.getAmount() && iou.m_strSourceID != string("IOU")){
		newTransaction(IOU(string("IOU"), iou.m_strSourceID, iou.getAmount()-it->second.getBalance()), checkCycle);
	}

	it->second.giveIOU(iou, checkCycle);

	validateNetwork();
}

IOU randomIOU(){
	stringstream ss;
	stringstream ssSource;
	stringstream ssAmount;
	stringstream ssTarget;

	long long llAmount = 0;

	dRandom1 = ((double)((rand() % (nHigh - nLow + 1)) + nLow)/nHigh) * cumulativeProbability1 ;
	for(unsigned int i = 0; i < vdProbabilitiesCumulative1.size()-1; i++){
		if(vdProbabilitiesCumulative1.at(i) <= dRandom1 && vdProbabilitiesCumulative1.at(i+1) >+ dRandom1 ){
			ss << i+1 << "_" ;
			ssSource << i+1 << "_" ;
			break;
		}
	}

	dRandom2 = ((double)((rand() % (nHigh - nLow + 1)) + nLow)/nHigh) * cumulativeProbability2 ;
	for(unsigned int i = 0; i < vdProbabilitiesCumulative2.size()-1; i++){
		if(vdProbabilitiesCumulative2.at(i) <= dRandom2 && vdProbabilitiesCumulative2.at(i+1) >+ dRandom2 ){
			ss << i+1 << ";";
			ssSource << i+1;
			float fMedian = i+1;
			float fSigma = 3.0;
			llAmount = box_muller(fMedian, fSigma);

			if(llAmount < 0){
				llAmount = llAmount * -1;
			}

			llAmount = llAmount * 100;

			ss << llAmount << ";" ;
			ssAmount << llAmount ;

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

	IOU iou = IOU(ssSource.str(), ssTarget.str(), llAmount);

	return iou;
}

void addNode(string ID, string label, float size = 1 ){
	stringstream ss;

	size = size/100;

	if(ID == string("IOU")){
			size = 100;
	}

	ss << "curl 'http://" << strHost <<":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"an\":{\"" << ID << "\":{\"label\":\"" << label << "\",\"size\":" << size << "}}}' -s -o 'curlOutput.txt'";
	system(ss.str().c_str());

}

void addEdge(string ID, string source, string target, float weight = 1 ){
	stringstream ss;

	weight = weight/100;

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"ae\":{\"" << ID << "\":{\"source\":\"" << source << "\",\"target\":\"" << target << "\",\"directed\":true,\"weight\":" << weight << ",\"label\":\"" << weight << "\"}}}' -s -o 'curlOutput.txt'";
	system(ss.str().c_str());

}

void changeNode(string ID, string label, float size = 1 ){
	stringstream ss;

	size = size/100;

	if(ID == string("IOU")){
		size = 100;
	}

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"cn\":{\"" << ID << "\":{\"label\":\"" << label << "\",\"size\":" << size << "}}}' -s -o 'curlOutput.txt'";
	system(ss.str().c_str());

}

void changeEdge(string ID, string source, string target, float weight = 1 ){
	stringstream ss;

	weight = weight/100;

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"ce\":{\"" << ID << "\":{\"source\":\"" << source << "\",\"target\":\"" << target << "\",\"directed\":true,\"weight\":" << weight << ",\"label\":\"" << weight << "\"}}}' -s -o 'curlOutput.txt'";
	system(ss.str().c_str());

}


void deleteNode(string ID){
	stringstream ss;

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"dn\":{\"" << ID << "\":{}}}' -s -o 'curlOutput.txt'";
	system(ss.str().c_str());
}
void deleteEdge(string ID){
	stringstream ss;

	ss << "curl 'http://" << strHost << ":8080/workspace0?operation=updateGraph' -d ";
	ss << "'{\"de\":{\"" << ID << "\":{}}}' -s -o 'curlOutput.txt'";
	system(ss.str().c_str());
}

cycle StronglyConnected(string v, string w, long long amount){
	cycle sCycle;
	sCycle.llLCD = amount;

	bool stronglyConnected = false;
	list<string> listOpen;
	list<string> listClosed;
	vector<string> vstrCycle;

	listOpen.push_back(w);

	string strNode = "";

	map<string, Account>::iterator it;
	while(listOpen.size()>0 && !stronglyConnected){
		strNode = listOpen.front();
		listOpen.pop_front();
		listClosed.push_back(strNode);

		if(v != strNode){
			it = mapAccounts.find(strNode);
			vector<string> vstrCreditors;
			vstrCreditors = it->second.creditors();
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
					vstrCycle.push_back(*listIt);
					strNode = *listIt;

					long long llOwedFrom = it->second.owedFrom(*listIt);
					if(llOwedFrom < sCycle.llLCD){
						sCycle.llLCD = llOwedFrom;
					}
				}
			}
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
		string debtor, creditor;

		if(i == (cycle.vstrCycle.size()-1)){
			debtor = cycle.vstrCycle.at(0);
		}
		else{
			debtor = cycle.vstrCycle.at(i+1);
		}

		//cout << it->second.m_ID << " is now reducing " << debtor << "'s debt." << endl;

		it->second.reduceIOUfrom(debtor, cycle.llLCD);
		//it->second.cancelOut(debtor, creditor, cycle.llLCD);

		it->second.balance();

		//dont do validate owedFromTo if its a 2nd degree cycle
		if(cycle.vstrCycle.size() > 2){
			//validateOwedFromTo(it->second.m_ID, creditor);
			validateOwedFromTo(debtor, it->second.m_ID);
		}

	}

	validateNetwork();
}

vector<cycle> possibleCreditorChains(string source){
	vector<cycle> vsCreditorChains;

	list<string> listOpen;
	list<string> listClosed;

	listOpen.push_front(source);
	string strNode = "";
	map<string, Account>::iterator it;

	while(listOpen.size()>0 ){
		cycle sCycle;
		vector<string> vstrCycle;
		sCycle.llLCD = -1;

		strNode = listOpen.front();
		listOpen.pop_front();
		listClosed.push_back(strNode);

		it = mapAccounts.find(strNode);
		vector<string> vstrCreditors;
		vstrCreditors = it->second.creditors();
		for(unsigned int i=0; i<vstrCreditors.size(); i++){
			if(std::find(listClosed.begin(), listClosed.end(), vstrCreditors.at(i)) == listClosed.end()){
				listOpen.push_back(vstrCreditors.at(i));
			}
		}

		list<string>::reverse_iterator listIt;
		string strNode = listClosed.back();
		vstrCycle.push_back(strNode);
		for (listIt = listClosed.rbegin();  listIt != listClosed.rend();  listIt++)
		{
			it = mapAccounts.find(strNode);
			vector<string> vstrDebtors;
			vstrDebtors = it->second.debtors();
			if(std::find(vstrDebtors.begin(), vstrDebtors.end(), *listIt)!=vstrDebtors.end()){
				vstrCycle.push_back(*listIt);
				strNode = *listIt;
				long long llOwedFrom = it->second.owedFrom(*listIt);
				if(llOwedFrom < sCycle.llLCD || sCycle.llLCD == -1){
					sCycle.llLCD = llOwedFrom;
				}
			}
		}
		sCycle.vstrCycle = vstrCycle;
		vsCreditorChains.push_back(sCycle);
	}
/*
	cout << "Possible creditor chains: " << endl;
	for(unsigned int i = 0; i < vsCreditorChains.size(); i++){
		cycle creditorChain = vsCreditorChains.at(i);
		cout << creditorChain.llLCD << "\t-> " ;

		for(unsigned int j = 0; j< creditorChain.vstrCycle.size(); j++){
			cout << creditorChain.vstrCycle.at(j) << ", " ;
		}
		cout << endl;
	}
*/

	return vsCreditorChains;
}


vector<cycle> possibleDebtorChains(string source){
	vector<cycle> vsDebtorChains;

	list<string> listOpen;
	list<string> listClosed;

	listOpen.push_front(source);
	string strNode = "";
	map<string, Account>::iterator it;

	while(listOpen.size()>0 ){
		cycle sCycle;
		vector<string> vstrCycle;
		sCycle.llLCD = -1;

		strNode = listOpen.front();
		listOpen.pop_front();
		listClosed.push_back(strNode);

		it = mapAccounts.find(strNode);
		vector<string> vstrDebtors;
		vstrDebtors = it->second.debtors();
		for(unsigned int i=0; i<vstrDebtors.size(); i++){
			if(std::find(listClosed.begin(), listClosed.end(), vstrDebtors.at(i)) == listClosed.end()){
				listOpen.push_back(vstrDebtors.at(i));
			}
		}

		list<string>::reverse_iterator listIt;
		string strNode = listClosed.back();
		vstrCycle.push_back(strNode);
		for (listIt = listClosed.rbegin();  listIt != listClosed.rend();  listIt++)
		{
			it = mapAccounts.find(strNode);
			vector<string> vstrCreditors;
			vstrCreditors = it->second.creditors();
			if(std::find(vstrCreditors.begin(), vstrCreditors.end(), *listIt)!=vstrCreditors.end()){
				vstrCycle.push_back(*listIt);
				strNode = *listIt;
				long long llOwedTo = it->second.owedTo(*listIt);
				if(llOwedTo < sCycle.llLCD || sCycle.llLCD == -1){
					sCycle.llLCD = llOwedTo;
				}
			}
		}
		sCycle.vstrCycle = vstrCycle;
		vsDebtorChains.push_back(sCycle);
	}

/*	cout << "Possible debtor chains: " << endl;
	for(unsigned int i = 0; i < vsDebtorChains.size(); i++){
		cycle debtorChain = vsDebtorChains.at(i);
		cout << debtorChain.llLCD << "\t-> " ;

		for(unsigned int j = 0; j< debtorChain.vstrCycle.size(); j++){
			cout << debtorChain.vstrCycle.at(j) << ", " ;
		}
		cout << endl;
	}
*/

	return vsDebtorChains;
}

cycle bestChain(vector<cycle> possibleChains){
	cycle bestChain;
	long long llValue = 0;

	for(unsigned int i = 0; i < possibleChains.size(); i++){
		if(possibleChains.at(i).llLCD != -1 && possibleChains.at(i).llLCD * possibleChains.at(i).vstrCycle.size() > llValue){
			bestChain = possibleChains.at(i);
			llValue = possibleChains.at(i).llLCD * possibleChains.at(i).vstrCycle.size();
		}
	}

	return bestChain;
}

cycle reverseChain(cycle chain){
	cycle sCycle;
	sCycle.llLCD = chain.llLCD;
	for(int i = chain.vstrCycle.size()-1; i >= 0; i--){
		sCycle.vstrCycle.push_back(chain.vstrCycle.at(i));
	}


	return sCycle;
}


void optimalCycle(string source, string target, long long amount){
	IOU iou = IOU(string(""), string(""), 0);

	vector<cycle> vsPossibleDebtorChains = possibleDebtorChains(source);
	vector<cycle> vsPossibleCreditorChains = possibleCreditorChains(target);

	vector<cycle> vsPossibleChains;

	for(unsigned int i = 0; i < vsPossibleDebtorChains.size(); i++){
		cycle sTempChain;

		for(unsigned int j = 0; j < vsPossibleCreditorChains.size(); j++){
			sTempChain = vsPossibleDebtorChains.at(i);
			cycle sTempCycle = vsPossibleCreditorChains.at(j);


			if(sTempCycle.llLCD != -1 && sTempCycle.llLCD < sTempChain.llLCD){
				sTempChain.llLCD = sTempCycle.llLCD;
			}
			if(amount < sTempChain.llLCD){
				sTempChain.llLCD = amount;
			}


			//for(int k = 0; k < sTempCycle.vstrCycle.size()-1; k++){
			for(int k = sTempCycle.vstrCycle.size()-1; k >= 0; k--){
				sTempChain.vstrCycle.push_back(sTempCycle.vstrCycle.at(k));
			}
			vsPossibleChains.push_back(sTempChain);
		}
	}


	cycle sOptimalChain = reverseChain(bestChain(vsPossibleChains));


		if(sOptimalChain.llLCD != -1 && sOptimalChain.llLCD >= sOptimalChain.vstrCycle.size() && sOptimalChain.vstrCycle.size() >= 6){
			cout << "Optimal chain: " ;
			for(unsigned int i = 0; i < sOptimalChain.vstrCycle.size(); i++){
				cout << sOptimalChain.vstrCycle.at(i) << ", " ;
			}
			cout << endl << "LCD: " << sOptimalChain.llLCD << "\tTotal value: " << sOptimalChain.llLCD * sOptimalChain.vstrCycle.size() << endl;
			cout << endl ;

			iou.m_strSourceID = sOptimalChain.vstrCycle.at(0);
			iou.m_strTargetID = sOptimalChain.vstrCycle.at(sOptimalChain.vstrCycle.size()-1);
			iou.setAmount(sOptimalChain.llLCD);


			newTransaction(iou, false);
			cancelOutCycle(sOptimalChain);

			long long llCanceledOut = sOptimalChain.llLCD;
			llTotalAmountCancelledOut += sOptimalChain.llLCD * sOptimalChain.vstrCycle.size();
			cout << "\t\t" << llCanceledOut << " has been cancelled out." << endl;

			newTransaction(IOU(iou.m_strTargetID, iou.m_strSourceID, iou.getAmount()), true);
			long long llTotalPaid = 0;
			for(unsigned int i = 1; i < sOptimalChain.vstrCycle.size()-1; i++){
				string strSource = sOptimalChain.vstrCycle.at(i);
				string strTarget = string("IOUnetwork");
				long long amount = sOptimalChain.llLCD/100;
				llTotalPaid += amount;
				cout << strSource << " --> " << strTarget << " : " << amount << endl;
				newTransaction(IOU(strSource, strTarget, amount), true);
			}
			long long llShare = llTotalPaid/3;
			newTransaction(IOU(string("IOUnetwork"), iou.m_strSourceID, llShare), true);
			newTransaction(IOU(string("IOUnetwork"), iou.m_strTargetID, llShare), true);

		}

}

long long profit(cycle cycle){
	long long llProfit;

	llProfit = (cycle.vstrCycle.size()-1) * (cycle.llLCD/100);
	//cout << llProfit << endl;
	return llProfit;
}

bool validateNetwork(){
	bool bOk = false;

	long long llTotalBalance = 0;
	for( map<string,Account>::iterator it=mapAccounts.begin(); it!=mapAccounts.end(); ++it)
	{
		Account cAccount = (*it).second;
		cAccount.balance();
		if(cAccount.m_ID != string("IOU")){
			llTotalBalance += cAccount.getBalance();
		}
	}

	map<string,Account>::iterator it = mapAccounts.find(string("IOU"));

	long long llNetworkBalance = it->second.getBalance();
	if(abs(llNetworkBalance) == llTotalBalance ){
		bOk = true;

	}
	else{
		cout << "Network balance check failed!" << endl;
		cout << "Balance IOU: " << llNetworkBalance << endl;
		cout << "Balance net: " << llTotalBalance << endl;
		cout << "difference: " << abs(llNetworkBalance) - llTotalBalance << endl;
		char ch;
		//cin >> ch;
	}

	return bOk;
}

bool validateOwedFromTo(string a, string b){
	bool bOk = false;

	map<string,Account>::iterator itA, itB;
	itA = mapAccounts.find(a);
	itB = mapAccounts.find(b);

	if(itA->second.owedTo(b) == itB->second.owedFrom(a)){
		bOk = true;
		//cout << "\tcheck OK" << endl;
	}
	else{
		cout << "check failed: owed from is not equal to owed to" << endl;
		cout << a << " owes " << itA->second.owedTo(b) << " to " << b << endl;
		cout << b << " owes " << itB->second.owedTo(a) << " from " << a << endl;
		char ch;
		//cin >> ch;

	}

	return bOk;
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



