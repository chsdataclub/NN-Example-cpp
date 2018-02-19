#pragma once
#include <vector>
#include "Node.h"
using namespace std;

class Network {
public:
	vector<Node> nodeList;
	vector<int> innovation;
	double learningRate;
	vector<Node*> input;
	vector<Node*> output;
	double fitness;
	double adjustedFitness;
	int networkId;
	int species;
	
	Network(int input, int output, int id, int species, double learningRate, bool addCon);
	Network(); // do not use

	void printNetwork();
	vector<double> process(vector<double>& input);
	double backProp(vector<double>& input, vector<double>& desired);
	double trainset(vector<pair<vector<double>, vector<double>>>& input, vector<pair<vector<double>, vector<double>>>& valid, int lim);
	int getInnovation(int pos);
	void addInnovation(int num);
	bool containsInnovation(int num);
	void removeInnovation(int num);

	void mutateConnection(int from, int to, int innovation);
	void mutateConnection(int from, int to, int innovation, double weight);
	int numConnection();
	void resetWeight();

	Node& getNode(int i);
	Node& createNode(int send);
	int getNextNodeId();
	int mutateNode(int from, int to, int innovationA, int innovationB);
	bool checkCircleMaster(Node& n, int goal);
	bool checkCircle(Node& n, int goal, int preCheck[]);
};

void clone(Network n, Network& ans, vector<pair<int, int>>* innovationDict);
