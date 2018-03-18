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

	Neat(int numNetworks, int input, int output, double mutate, double lr, double(*activation)(double value), double(*activationDerivative)(double value));

	Network start(vector<pair<vector<double>, vector<double>>>& input, vector<pair<vector<double>, vector<double>>>& valid, int cutoff, double target, Network& betNet); //returning network will not work
	void printNeat();
	void mutatePopulation();

	void trainNetworks(vector<pair<vector<double>, vector<double>>>& input, vector<pair<vector<double>, vector<double>>>& valid);
	void mateSpecies();

	void speciateAll();
	void checkSpecies();
	void speciate(Network& n, Species* s);
	double compareGenome(int node, vector<int>& innovation, int nodeA, vector<int>& innovationA);

	//pair<int,int> getInnovation(int num);
	//int addInnovation(int a, int b);
	//int innovationSize(int a);

	Species& getSpecies(int id);
	Species& createSpecies(vector<Network*>& possible);
	Species& createSpecies(int startIndex, int endIndex);
	void removeSpecies(int id);
};