#include "stdafx.h"
#include "Network.h"
#include "Connection.h"
#include "Node.h"
#include <iostream>
using namespace std;

Network::Network(int inputI, int outputI, int id, int species, double learningRate, bool addCon)
{
	nodeList.reserve(100);
	networkId = id;
	this->learningRate = learningRate;
	this->species = species;

	//create output nodes
	for (int i = 0; i < outputI; i++) {
		output.push_back(&createNode(0));
	}

	//creates the input nodes and adds them to the network
	int startInov = 0; //this should work
	for (int i = 0; i < inputI; i++) {
		input.push_back(&createNode(100));
		if (addCon) {
			for (int a = 0; a < outputI; a++) {
				mutateConnection(input[i]->id, output[a]->id, startInov);
				startInov++;
			}
		}
	}
	input.push_back(&createNode(100)); //bias starts unconnected and will form connections over time
	fitness = -5;
	adjustedFitness = -5;
}

Network::Network()
{
}

void Network::printNetwork()
{
	cout << endl;
	cout << "Network id: " << networkId << " Species: " << species << " ";
	for (int i = 0; i < innovation.size(); i++) {
		cout << innovation[i] << " ";
	}
	cout << endl;

	for (int i = 0; i < nodeList.size(); i++) {
		Node& n = nodeList[i];
		cout << "	Node: " << n.id << " Sending: ";
		for (int a = 0; a < n.send.size(); a++) {
			cout << n.send[a].nodeTo->id << " ";
		}
		cout << "recieving: ";
		for (int a = 0; a < n.recieve.size(); a++) {
			cout << n.recieve[a]->nodeFrom->id << " ";
		}
		cout << endl;
	}
	cout << endl;
}

vector<double> Network::process(vector<double>& input) {
	//set input values
	for (int i = 0; i < input.size(); i++) {
		if (i < this->input.size()) {
			this->input[i]->setValue(input[i]);
		}
		else {
			this->input[i]->setValue(1);
		}
	}

	vector<double> ans;
	//values are calculated via connections and nodes signalling
	for (int i = 0; i < output.size(); i++) {
		ans.push_back(output[i]->value);
	}

	return ans;
}

double Network::backProp(vector<double>& input, vector<double>& desired)
{
	process(input); //set the values for the input

	double error = 0.0; //return value

	//this will calc all the influence
	for (int i = 0; i < output.size(); i++) {
		output[i]->setInfluence(output[i]->value - desired[i]);
		error += abs(output[i]->value - desired[i]);
	}

	//all the influence is set the same way as values so it is set via connections and signalling

	//actually adjusts the weights
	for (int i = 0; i < nodeList.size(); i++) {
		for (int a = 0; a < nodeList[i].recieve.size(); a++) {
			nodeList[i].recieve[a]->nextWeight -= (nodeList[i].recieve[a]->nodeFrom->value) * nodeList[i].influence * learningRate;
		}
	}

	return error;
}

double Network::trainset(vector<pair<vector<double>, vector<double>>>& input, vector<pair<vector<double>, vector<double>>>& valid, int lim)
{
	double errorChange = -1000.0; //percent of error change
	double lastError = 1000.0;
	double lastValid = 100000.0;
	//initializes best weights
	vector<vector<double>> bestWeight;

	resetWeight(); //clears the current weight values

	int strikes = 10; //number of times in a row that error can increase
	for (int z = 1; strikes > 0 && z < lim && lastError > .000001; z++) {
		double currentError = 0.0;

		//resets all the nextWeights
		for (int i = 0; i < nodeList.size(); i++) {
			for (int a = 0; a < nodeList[i].send.size(); a++) {
				nodeList[i].send[a].nextWeight = 0;
			}
		}

		//trains each input
		for (int i = 0; i < input.size(); i++) { //TODO: might not work
			currentError += backProp(input[i].first, input[i].second);
		}

		//updates all the weight
		for (int i = 0; i < nodeList.size(); i++) {
			for (int a = 0; a < nodeList[i].send.size(); a++) {
				nodeList[i].send[a].weight += nodeList[i].send[a].nextWeight / input.size();
			}
		}

		errorChange = (currentError - lastError) / lastError;
		lastError = currentError;

		double validError = 0;
		for (int i = 0; i < valid.size(); i++) {
			vector<double> val = process(valid[i].first);
			for (int a = 0; a < valid[i].second.size(); a++) {
				validError += abs(val[a]-valid[i].second[a]);
			}
		}

		if (validError > lastValid) {
			strikes--;
		}
		else if (errorChange >= 0) {		//decreases the number of strikes or resets them and changes best weight
			strikes--;
		}
		else {
			bestWeight.clear();
			for (int i = 0; i < nodeList.size(); i++) {
				vector<double> one;
				one.reserve(nodeList[i].send.size());
				for (int a = 0; a < nodeList[i].send.size(); a++) {
					one.push_back(nodeList[i].send[a].weight);
				}
				bestWeight.push_back(one);
			}
			strikes = 10;
		}

		lastValid = validError;
		//cout << "Error: " << currentError << " change " << errorChange << endl;
	}

	//sets the weights back to the best
	for (int i = 0; i < bestWeight.size(); i++) {
		for (int a = 0; a < bestWeight[i].size(); a++) {
			nodeList[i].send[a].weight = bestWeight[i][a];
		}
	}

	//calculate the final error
	double final = 0.0;
	for (int i = 0; i < input.size(); i++) {
		vector<double> stuff = process(input[i].first);
		for (int a = 0; a < stuff.size(); a++) {
			final += abs(stuff[a] - input[i].second[a]);
		}
	}

	fitness = 1 / final;
	return final;
}

int Network::getInnovation(int pos)
{
	return innovation[pos];
}

void Network::addInnovation(int num)
{
	innovation.push_back(num);
}

bool Network::containsInnovation(int num)
{
	for (int i = 0; i < innovation.size(); i++) {
		if (innovation[i] == num) {
			return true;
		}
	}

	return false;
}

void Network::removeInnovation(int num)
{
	for (int i = 0; i < innovation.size(); i++) {
		if (innovation[i] == num) {
			innovation.erase(innovation.begin() + i);
		}
	}
}

void Network::mutateConnection(int from, int to, int innovation)
{
	getNode(to).addRecCon(&getNode(from).addSendCon(Connection(&getNode(from), &getNode(to), innovation)));
	addInnovation(innovation);
}

void Network::mutateConnection(int from, int to, int innovation, double weight)
{
	Connection& c = getNode(to).addRecCon(&getNode(from).addSendCon(Connection(&getNode(from), &getNode(to), innovation)));
	c.weight = weight;
	addInnovation(innovation);
}

int Network::numConnection()
{
	double ans = 0;
	for (int i = 0; i < nodeList.size(); i++) {
		ans += nodeList[i].send.size();
	}

	return ans;
}

void Network::resetWeight()
{
	for (int i = 0; i < nodeList.size(); i++) {
		for (int a = 0; a < nodeList[i].send.size(); a++) {
			nodeList[i].send[a].randWeight();
			nodeList[i].send[a].nextWeight = 0;
		}
	}
}

Node& Network::getNode(int i)
{
	return nodeList[i];
}

Node& Network::createNode(int send)
{
	int a = nodeList.size();
	nodeList.push_back(Node(a, send));
	return nodeList.back();
}

int Network::getNextNodeId()
{
	return nodeList.size();
}

int Network::mutateNode(int from, int to, int innovationA, int innovationB)
{
	Node& fromNode = getNode(from);
	Node& toNode = getNode(to);
	Node& newNode = createNode(100);

	addInnovation(innovationA);
	addInnovation(innovationB);

	//changes the connection recieved by toNode to a connection sent by newNode
	for (int i = 0; i < toNode.recieve.size(); i++) {
		if (fromNode.id == toNode.recieve[i]->nodeFrom->id) {
			removeInnovation(toNode.recieve[i]->innovation);
			toNode.recieve[i] = &newNode.addSendCon(Connection(&newNode, &toNode, innovationB));
		}
	}

	//modifies the connection from fromNode by changing the toNode for the connection to newNode from toNode
	for (int i = 0; i < fromNode.send.size(); i++) {
		if (fromNode.send[i].nodeTo->id == toNode.id) {
			fromNode.send[i].nodeTo = &newNode;
			fromNode.send[i].innovation = innovationA;

			newNode.addRecCon(&(fromNode.send[i]));
		}
	}

	return newNode.id;
}

bool Network::checkCircleMaster(Node& n, int goal)
{
	const int s = nodeList.size();
	int* preCheck = new int[nodeList.size()];

	for (int i = 0; i < nodeList.size(); i++) {
		preCheck[i] = i;
	}

	bool ans = checkCircle(n, goal, preCheck);

	delete[] preCheck;

	return ans;
}

bool Network::checkCircle(Node& n, int goal, int preCheck[])
{
	bool ans = false;
	if (n.id == goal) {
		return true;
	}

	//checks for the precheck
	if (preCheck[n.id] == -1) {
		return false;
	}

	//checks next stop down
	for (int i = 0; i < n.recieve.size(); i++) {
		ans = checkCircle((*n.recieve[i]->nodeFrom), goal, preCheck);
		if (ans) {
			break;
		}
	}

	//sets the precheck
	if (!ans) {
		preCheck[n.id] = -1;
	}

	return ans;
}

void clone(Network n, Network& ans, vector<pair<int, int>>* innovationDict)
{
	int max = n.nodeList.size();

	vector<pair<int, double>> innovation;
	for (int i = 0; i < n.nodeList.size(); i++) {
		for (int a = 0; a < n.nodeList[i].send.size(); a++) {
			innovation.push_back(pair<int, double>(n.nodeList[i].send[a].innovation, n.nodeList[i].send[a].weight));
		}
	}

	//need to totally reconstruct because otherwise the pointers in connections and such would be screwed up
	ans = Network(n.input.size() - 1, n.output.size(), n.networkId, n.species, n.learningRate, false);

	for (int i = 0; i < max - ans.input.size() - ans.output.size(); i++) {
		ans.createNode(100);
	}

	for (int i = 0; i < innovation.size(); i++) {
		ans.mutateConnection((*innovationDict)[innovation[i].first].first, (*innovationDict)[innovation[i].first].second, innovation[i].first, innovation[i].second);
	}

	ans.fitness = n.fitness;
}
