#include "stdafx.h"
#include "Network.h"
#include "Connection.h"
#include "Node.h"
#include <iostream>
#include "Activation.h"
#include <math.h>
#include <mutex>
using namespace std;

Network::Network(int inputI, int outputI, int id, int species, double learningRate, bool addCon, double(*activation)(double value), double(*activationDerivative)(double value))
{
	nodeList.reserve(100);
	networkId = id;
	this->learningRate = learningRate;
	this->species = species;
	this->activation = activation;
	this->activationDerivative = activationDerivative;

	//create output nodes
	for (int i = 0; i < outputI; i++) {
		output.push_back(&createNode(0, activation, activationDerivative));
	}

	//creates the input nodes and adds them to the network
	int startInov = 0; //this should work
	for (int i = 0; i < inputI; i++) {
		input.push_back(&createNode(100, activation, activationDerivative));
		if (addCon) {
			for (int a = 0; a < outputI; a++) {
				mutateConnection(input[i]->id, output[a]->id, startInov);
				startInov++;
			}
		}
	}
	input.push_back(&createNode(100, activation, activationDerivative)); //bias starts unconnected and will form connections over time
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
		string act = "";
		if (n.activation == &tanh) { //ignore if error
			act = "tanh";
		}
		else if (n.activation == &sigmoid) {
			act = "sig";
		}
		else if (n.activation == &lRelu) {
			act = "lRelu";
		}

		cout << "	Node: " << n.id << " activation: " << act.c_str() << " Sending: ";
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
			nodeList[i].recieve[a]->nextWeight += (nodeList[i].recieve[a]->nodeFrom->value) * nodeList[i].influence;
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
	double globalBest = 100000;
	resetWeight(); //clears the current weight values

	int strikes = 10; //number of times in a row that error can increase
	for (int z = 1; strikes > 0 && z < lim && lastError > .000001; z++) {
		double currentError = 0.0;

		//trains each input
		for (int i = 0; i < input.size(); i++) {
			currentError += backProp(input[i].first, input[i].second);
		}

		//updates all the weight
		for (int i = 0; i < nodeList.size(); i++) {
			for (int a = 0; a < nodeList[i].send.size(); a++) {
				Connection& c = nodeList[i].send[a];
				double g = c.nextWeight / input.size();
				c.momentum = (c.beta*c.momentum) + ((1 - c.beta)*g);
				c.velocity = c.betaA*c.velocity + (1 - c.betaA)*pow(g, 2);
				double vhat = c.velocity / (1 - pow(c.betaA, z)); //z has to start at 1
				double mhat = c.momentum / (1 - pow(c.beta, z));
				nodeList[i].send[a].setWeight(c.weight - (learningRate / (sqrt(vhat) + c.epsilon)) * (c.beta*mhat + ((1 - c.beta)*g / (1 - pow(c.beta, z)))));
			}
		}

		errorChange = (currentError - lastError) / lastError;
		lastError = currentError;

		double validError = 0;
		for (int i = 0; i < valid.size(); i++) {
			vector<double> val = process(valid[i].first);
			for (int a = 0; a < valid[i].second.size(); a++) {
				validError += abs(val[a] - valid[i].second[a]);
			}
		}

		//decreases the number of strikes or resets them and changes best weight
		if (validError > lastValid) {
			strikes--;
		}
		else if (errorChange >= 0) {
			strikes--;
		}
		else if (currentError < globalBest) {
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

			globalBest = currentError;
			lastValid = validError;
		}

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
	//cout << "final training is " << fitness << endl;
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
	int numIn = -1;
	int numOut = -1;

	vector<Node*> currentNodes;
	for (int i = 0; i < input.size(); i++) {
		currentNodes.push_back(input[i]);
	}

	while (numOut != 0) {
		numIn = currentNodes.size();
		numOut = 0;
		vector<Node*> secondary;

		for (int i = 0; i < currentNodes.size(); i++) {
			numOut += currentNodes[i]->send.size();
			for (int a = 0; a < currentNodes[i]->send.size(); a++) {
				secondary.push_back(currentNodes[i]->send[a].nodeTo);
			}
		}

		double value = 2 / (double)(numIn + numOut);

		for (int i = 0; i < currentNodes.size(); i++) {
			for (int a = 0; a < currentNodes[i]->send.size(); a++) {
				currentNodes[i]->send[a].weight = random(0 - value, value);
				currentNodes[i]->send[a].nextWeight = 0;;
				currentNodes[i]->send[a].momentum = 0;
				currentNodes[i]->send[a].velocity = 0;
			}
		}

		currentNodes = secondary;
	}
}

Node& Network::getNode(int i)
{
	return nodeList[i];
}

Node& Network::createNode(int send)
{
	double(*activation)(double value);
	double(*activationDerivative)(double value);

	int rand = random(1, 9);
	if (rand <= 3) {
		activation = &lRelu;
		activationDerivative = &lReluDerivative;
	}
	else if (rand <= 6) {
		activation = &sigmoid;
		activationDerivative = &sigmoidDerivative;
	}
	else {
		activation = &tanh;
		activationDerivative = &tanhDerivative;
	}
	int a = nodeList.size();
	nodeList.push_back(Node(a, send, activation, activationDerivative));
	return nodeList.back();
}
Node& Network::createNode(int send, double(*activation)(double value), double(*activationDerivative)(double value))
{
	Node& n = createNode(send);
	n.activation = activation;
	n.activationDerivative = activationDerivative;
	return n;
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

int Network::mutateNode(int from, int to, int innovationA, int innovationB, double(*activation)(double value), double(*activationDerivative)(double value))
{
	Node& n = nodeList[mutateNode(from, to, innovationA, innovationB)];
	n.activation = activation;
	n.activationDerivative = activationDerivative;
	return 0;
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

mutex cloneMutex;
void clone(Network n, Network& ans, vector<pair<int, int>>* innovationDict)
{
	ans = Network(n.input.size() - 1, n.output.size(), n.networkId, n.species, n.learningRate, false, ans.activation, ans.activationDerivative);

	for (int i = n.output.size() + n.input.size(); i < n.nodeList.size(); i++) {
		ans.createNode(100, n.nodeList[i].activation, n.nodeList[i].activationDerivative);
	}
	for (int i = 0; i < n.nodeList.size(); i++) {
		Node* node = &n.nodeList[i];
		for (int a = 0; a < n.nodeList[i].send.size(); a++) {
			cloneMutex.lock();
			pair<int, int> v = (*innovationDict)[node->send[a].innovation];
			cloneMutex.unlock();
			ans.mutateConnection(v.first, v.second, node->send[a].innovation, node->send[a].weight);
		}
	}
	ans.fitness = n.fitness;
}
