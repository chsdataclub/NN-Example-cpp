#pragma once
#include "Network.h"
#include "Species.h"
#include <utility>
#include <thread>
using namespace std;

class Neat {
public:
	double nodeMutate;
	vector<Network> network;
	vector<pair<int, int>> connectionInnovation;
	double speciesThreshold;
	vector<Species> species;
	int speciesId;
	vector<thread> threads;

	Neat(int numNetworks, int input, int output, double mutate, double lr);

	Network start(vector<pair<vector<double>,vector<double>>>& input, int cutoff, double target, Network& bestNet); //returning network will not work
	void printNeat();
	void mutatePopulation();
	
	void trainNetworks(vector<pair<vector<double>, vector<double>>>& input);
	void mateSpecies();

	void speciateAll();
	void checkSpecies();
	void speciate(Network& n, Species* s);
	double compareGenome(int node, vector<int>& innovation, int nodeA, vector<int>& innovationA);

	//int* getInnovation(int num);
	//int findInnovation(int search[2]);
	Species& getSpecies(int id);
	Species& createSpecies(vector<Network*>& possible);
	Species& createSpecies(int startIndex, int endIndex);
	void removeSpecies(int id);
};