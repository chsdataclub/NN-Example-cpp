#pragma once
#include <vector>
#include "Network.h"
using namespace std;

class Species {
public:
	static vector<pair<int, int>>* innovationDict;
	vector<Network*> network;
	vector<int> connectionInnovation;
	vector<int> commonInnovation;
	int id;
	double mutate;

	Species(int id, vector<Network*> networks, double mutate);
	Species();

	void addCI(int a);
	void removeCI(int a);
	int& getInovOcc(int i);
	int& incrementInov(int i);
	int& reduceInov(int i);
	void checkCI();
	//pair<int, int> getInnovationRef(int num);
	//int createNewInnovation(int a, int b);
	void sortInnovation();

	Network& getNetworkAt(int a);
	void removeNetwork(int id);
	Network& getNetwork(int id);
	void addNetwork(Network& n);

	void updateStereotype();
	void mutateNetwork(Network& network);
	void mateNetwork(vector<Node>& nB, vector<Node>& nA, bool bBetter, Network& ans);
	void trainNetworks(vector<pair<vector<double>, vector<double>>>& trainingSet, vector<pair<vector<double>, vector<double>>>& valid);
	void mateSpecies();
	void adjustFitness();
	int avgNode();
};