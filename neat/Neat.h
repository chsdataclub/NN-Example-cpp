#pragma once
#include "Network.h"
#include "Species.h"
#include <utility>
using namespace std;

class Neat {
public:
	double nodeMutate;
	vector<Network> network;
	vector<int*> connectionInnovation;
	double speciesThreshold;
	vector<Species> species;
	int speciesId;

	Neat(int numNetworks, int input, int output, double mutate, double lr);

	Network start(vector<pair<vector<double>,vector<double>>>& input, int cutoff, double target);
	void printNeat();
	void mutatePopulation();
	
	void speciateAll();
	void checkSpecies();
	void speciate(Network& n);
	double compareGenome(int node, vector<int>& innovation, int nodeA, vector<int>& innovationA);

	//int* getInnovation(int num);
	//int findInnovation(int search[2]);
	Species& getSpecies(int id);
	Species& createSpecies(vector<Network*>& possible);
	Species& createSpecies(int startIndex, int endIndex);
	void removeSpecies(int id);
};