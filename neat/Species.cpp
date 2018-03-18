#include "stdafx.h"
#include "Species.h"
#include <algorithm>
#include <ctime> //for time function
#include <cstdlib>// for srand function.
#include "Activation.h"
#include <iostream>
#include <mutex>
using namespace std;

vector<pair<int, int>>* Species::innovationDict;

Species::Species(int id, vector<Network*> networks, double mutate)
{
	this->id = id;
	this->mutate = mutate;
	this->network = networks;

	updateStereotype();
}
Species::Species() {}
//these CI methods are for common innovation
void Species::addCI(int a)
{
	for (int i = 0; i < commonInnovation.size(); i++) {
		if (commonInnovation[i] == a) {
			return;
		}
	}

	commonInnovation.push_back(a);
}
void Species::removeCI(int a)
{
	for (int i = 0; i < commonInnovation.size(); i++) {
		if (commonInnovation[i] == a) {
			commonInnovation.erase(commonInnovation.begin() + i);
		}
	}
}

int& Species::getInovOcc(int i)
{
	if (i >= connectionInnovation.size()) {
		connectionInnovation.reserve(connectionInnovation.capacity() + i);
	}
	while (i >= connectionInnovation.size()) {
		connectionInnovation.push_back(0);
	}
	return (connectionInnovation)[i];
}

int& Species::incrementInov(int i)
{
	int& ans = getInovOcc(i);
	(ans)++;

	if ((double)ans / network.size() >= .5) {
		addCI(i);
	}

	return ans;
}

int& Species::reduceInov(int i)
{
	int& ans = getInovOcc(i);
	(ans)--;

	if (ans < 0) {
		ans = 0;
	}

	if ((double)ans / network.size() < .5) {
		removeCI(i);
	}

	return ans;
}

void Species::checkCI()
{
	commonInnovation.clear();

	for (int i = 0; i < connectionInnovation.size(); i++) {
		if ((double)connectionInnovation[i] / network.size() >= .5) {
			addCI(i);
		}
	}
}

/*mutex mRef;
pair<int, int> Species::getInnovationRef(int num)
{
	lock_guard<mutex> m(mRef);
	return (*innovationDict)[num];
}

mutex mx;
int Species::createNewInnovation(int a, int b)
{
	lock_guard<mutex> lg(mx);
	pair<int, int> c = { a, b };
	(*innovationDict).push_back(c);
	return innovationDict->size() - 1;
}*/

void Species::sortInnovation()
{
	for (int i = 0; i < network.size(); i++) {
		sort(network[i]->innovation.begin(), network[i]->innovation.end());
	}
}

Network& Species::getNetworkAt(int a)
{
	return *network[a];
}

void Species::removeNetwork(int id)
{
	for (int i = 0; i < network.size(); i++) {
		if (network[i]->networkId == id) {
			vector<int> inn = network[i]->innovation;
			network.erase(network.begin() + i);

			for (int a = 0; a < inn.size(); a++) {
				reduceInov(inn[a]);
			}

			checkCI();
		}
	}
}

Network& Species::getNetwork(int id)
{
	for (int i = 0; i < network.size(); i++) {
		if (network[i]->networkId == id) {
			return *network[i];
		}
	}
}

void Species::addNetwork(Network& n)
{
	n.species = id;
	network.push_back(&n);

	for (int i = 0; i < n.innovation.size(); i++) {
		incrementInov(n.innovation[i]);
	}

	checkCI();
}

void Species::updateStereotype()
{
	int numNodes = 0;

	connectionInnovation.clear();
	commonInnovation.clear();

	for (int i = 0; i < network.size(); i++) {
		numNodes += network[i]->nodeList.size();
		for (int a = 0; a < network[i]->innovation.size(); a++) {
			incrementInov(network[i]->innovation[a]);
		}
	}
}

void Species::mutateNetwork(Network& network)
{
	int nodeRange = network.nodeList.size();
	int startNodeRange = network.input.size() + network.output.size();
	//finds or adds innovation numbers and returns the innovation
	auto addConnectionInnovation = [&](int numFrom, int numTo) {
		//checks to see if preexisting innovation
		int maxPos = innovationDict->size()-1;
		for (int i = 0; i < maxPos; i++) {
			pair<int, int> pos = safeRead(*innovationDict, i);
			if (pos.second == numTo && pos.first == numFrom) {
				incrementInov(i);

				return i;
			}
		}

		//checks to see if needs to grow
		int num = safeWrite(*innovationDict, numFrom, numTo);

		incrementInov(num);

		return num;
	};

	//mutates a node
	auto nodeMutate = [&]() {
		int  firstNode = -1;
		int secondNode = -1;
		bool ans = false;

		//picks a node
		while (!ans) {
			firstNode = random(0, nodeRange - 1); // int(rand() % nodeRange);

			if (network.getNode(firstNode).send.size() > 0) {
				ans = true;
			}
		}

		Connection& c = network.getNode(firstNode).send[random(0, network.getNode(firstNode).send.size() - 1)];
		//picks a random connection from firstNode and gets the id
		secondNode = c.nodeTo->id; //int(r.Int63n(int64(nodeRange)));
		reduceInov(c.innovation);

		network.mutateNode(firstNode, secondNode, addConnectionInnovation(firstNode, network.getNextNodeId()), addConnectionInnovation(network.getNextNodeId(), secondNode));
	};

	int r = random(0, 100);
	//randomly picks if node or connection mutate
	if (r <= mutate * 100 || startNodeRange == network.nodeList.size()) {
		nodeMutate();
	}
	else if (r >= 90) { //TODO: customize
		int numNode = .1*network.nodeList.size();

		for (int i = 0; i < numNode; i++) {
			int rand = random(1, 9);
			Node& n = network.nodeList[random(startNodeRange, nodeRange - 1)];

			if (i <= 3) {
				n.activation = &tanh;
				n.activationDerivative = &tanhDerivative;
			}
			else if (i <= 6) {
				n.activation = &sigmoid;
				n.activationDerivative = &sigmoidDerivative;
			}
			else {
				n.activation = &lRelu;
				n.activationDerivative = &lReluDerivative;
			}
		}
	}
	else {
		int firstNode;
		int secondNode;
		//ans := true
		int attempts = 0; //attempts at finding nodes

						  //find 2 unconnected nodes
						  //for ans && attempts <= 10 {
		while (attempts <= 10) {
			firstNode = random(0, nodeRange - 1);
			secondNode = random(0, nodeRange - 1);

			if (firstNode == secondNode || isOutput(network.getNode(firstNode)) || isInput(network.getNode(secondNode))) {
				continue;
			}

			if (network.getNode(firstNode).connectsTo(secondNode) || network.getNode(secondNode).connectsTo(firstNode) || network.checkCircleMaster(network.getNode(firstNode), secondNode)) {
				attempts++;
				continue;
			}

			break;
		}

		//muateNode if attempts exceeded else add the connection
		if (attempts > 10) {
			nodeMutate();
		}
		else {
			network.mutateConnection(firstNode, secondNode, addConnectionInnovation(firstNode, secondNode));
		}
	}
}

void Species::mateNetwork(vector<Node>& nB, vector<Node>& nA, bool bBetter, Network& ans)
{
	int in = network[0]->input.size() - 1;
	int out = network[0]->output.size();
	ans = Network(in, out, -1, id, network[0]->learningRate, false, ans.activation, ans.activationDerivative);

	vector<Node>* numNode;
	vector<Node>* l;
	if (nB.size() > nA.size()) {
		numNode = &nB;
		l = &nA;
	}
	else {
		l = &nB;
		numNode = &nA;
	}

	//create the nodes
	for (int i = (in + out + 1); i < numNode->size(); i++) { //this should be ok
		if (bBetter & i < nB.size() || i >= nA.size()) {
			ans.createNode(100, nB[i].activation, nB[i].activationDerivative);
		}
		else {
			ans.createNode(100, nA[i].activation, nA[i].activationDerivative);
		}
	}

	//add nA innovation
	for (int i = 0; i < numNode->size(); i++) {
		for (int a = 0; a < (*numNode)[i].send.size(); a++) {
			pair<int, int> ina = safeRead(*innovationDict, (*numNode)[i].send[a].innovation);
			ans.mutateConnection(ina.first, ina.second, (*numNode)[i].send[a].innovation);
		}
	}

	//add unique (*l) innovation
	for (int i = 0; i < l->size(); i++) {
		for (int a = 0; a < (*l)[i].send.size(); a++) {
			pair<int, int> inb = safeRead(*innovationDict, (*l)[i].send[a].innovation);
			int firstNode = inb.first;
			int secondNode = inb.second;

			//checks to make sure their is no conflict in possible innovations
			if (!(ans.containsInnovation((*l)[i].send[a].innovation) || ans.getNode(firstNode).connectsTo(secondNode) || ans.getNode(secondNode).connectsTo(firstNode) || ans.checkCircleMaster(ans.getNode(firstNode), secondNode))) {
				ans.mutateConnection(firstNode, secondNode, (*l)[i].send[a].innovation);
			}
		}
	}
}

void Species::trainNetworks(vector<pair<vector<double>, vector<double>>>& trainingSet, vector<pair<vector<double>, vector<double>>>& valid)
{
	for (int i = 0; i < network.size(); i++) {
		network[i]->trainset(trainingSet, valid, 10000); //I have capped the number of interations intentionaly to control training time
	}
}

void Species::mateSpecies()
{
	{
		for (int i = 0; i < network.size(); i++) {
			if (network[i]->networkId < 0) {
				cout << "shittttt" << endl;
			}
		}
	}
	adjustFitness();

	{
		for (int i = 0; i < network.size(); i++) {
			if (network[i]->networkId < 0) {
				cout << "shittttt" << endl;
			}
		}
	}
	//sorts by adjusted fitness
	vector<Network*> sortedNetwork;
	sortedNetwork.reserve((network.size() * 85 / 100));
	double lastValue = 1000.0;
	double sumFitness = 0.0;
	for (int i = 0; i < sortedNetwork.capacity(); i++) {
		double localMax = 0.0;
		int localIndex = 0;
		for (int a = 0; a < network.size(); a++) {
			if (getNetworkAt(a).adjustedFitness > localMax && getNetworkAt(a).adjustedFitness <= lastValue) {
				bool good = true;
				for (int b = i - 1; b >= 0; b--) {
					if (getNetworkAt(a).networkId == sortedNetwork[b]->networkId) {
						good = false;
						break;
					}

					if (sortedNetwork[b]->adjustedFitness != getNetworkAt(a).adjustedFitness) {
						break;
					}
				}

				if (good) {
					localMax = network[a]->adjustedFitness;
					localIndex = a;
				}
			}
		}

		sortedNetwork.push_back(&getNetworkAt(localIndex));
		sumFitness += sortedNetwork[i]->adjustedFitness;
		lastValue = sortedNetwork[i]->adjustedFitness;
	}
	{
		for (int i = 0; i < network.size(); i++) {
			if (network[i]->networkId < 0) {
				cout << "shittttt" << endl;
			}
		}

		for (int i = 0; i < sortedNetwork.size(); i++) {
			if (sortedNetwork[i]->networkId < 0) {
				cout << "shiiiiiiittttttttttt" << endl;
			}
		}
	}
	int count = 0;
	//mates networks
	for (int i = 0; i < sortedNetwork.size(); i++) {
		int numKids = int(sortedNetwork[i]->adjustedFitness / sumFitness * network.size());
		int numMade = numKids;
		vector<Node> currentIn = sortedNetwork[i]->nodeList;
		int currentNL = sortedNetwork[i]->nodeList.size();
		for (int a = 1; count < network.size() && a + i < sortedNetwork.size(); a++) {
			int id = sortedNetwork[i]->networkId;
			mateNetwork(currentIn, sortedNetwork[i + a]->nodeList, sortedNetwork[i]->fitness > sortedNetwork[i + a]->fitness, *sortedNetwork[i]);
			sortedNetwork[i]->networkId = id;
			count++;
			numMade--;
			{
				for (int i = 0; i < network.size(); i++) {
					if (network[i]->networkId < 0) {
						cout << "shittttt" << endl;
					}
				}

				for (int i = 0; i < sortedNetwork.size(); i++) {
					if (sortedNetwork[i]->networkId < 0) {
						cout << "shiiiiiiittttttttttt" << endl;
					}
				}
			}
		}
		{
			for (int i = 0; i < network.size(); i++) {
				if (network[i]->networkId < 0) {
					cout << "shittttt" << endl;
				}
			}

			for (int i = 0; i < sortedNetwork.size(); i++) {
				if (sortedNetwork[i]->networkId < 0) {
					cout << "shiiiiiiittttttttttt" << endl;
				}
			}
		}
	}

	{
		for (int i = 0; i < network.size(); i++) {
			if (network[i]->networkId < 0) {
				cout << "shittttt" << endl;
			}
		}

		for (int i = 0; i < sortedNetwork.size(); i++) {
			if (sortedNetwork[i]->networkId < 0) {
				cout << "shiiiiiiittttttttttt" << endl;
			}
		}
	}

	//mutates for remainder of spots available
	for (int i = 0; count < network.size(); i++) {
		int id = network[count]->networkId;
		{
			for (int i = 0; i < network.size(); i++) {
				if (network[i]->networkId < 0) {
					cout << "shittttt" << endl;
				}
			}

			for (int i = 0; i < sortedNetwork.size(); i++) {
				if (sortedNetwork[i]->networkId < 0) {
					cout << "shiiiiiiittttttttttt" << endl;
				}
			}
		}
		clone(*sortedNetwork[i], *network[count], innovationDict);
		{
			for (int i = 0; i < network.size(); i++) {
				if (network[i]->networkId < 0) {
					cout << "shittttt" << endl;
				}
			}

			for (int i = 0; i < sortedNetwork.size(); i++) {
				if (sortedNetwork[i]->networkId < 0) {
					cout << "shiiiiiiittttttttttt" << endl;
				}
			}
		}
		mutateNetwork(*network[count]);
		{
			for (int i = 0; i < network.size(); i++) {
				if (network[i]->networkId < 0) {
					cout << "shittttt" << endl;
				}
			}

			for (int i = 0; i < sortedNetwork.size(); i++) {
				if (sortedNetwork[i]->networkId < 0) {
					cout << "shiiiiiiittttttttttt" << endl;
				}
			}
		}
		network[count]->networkId = id;
		count++;

		if (i == sortedNetwork.size() - 1) {
			i--; //this can lead to mutating the same network as last time (stacking mutations) but i don't think it is a big deal
		}
		{
			for (int i = 0; i < network.size(); i++) {
				if (network[i]->networkId < 0) {
					cout << "shittttt" << endl;
				}
			}

			for (int i = 0; i < sortedNetwork.size(); i++) {
				if (sortedNetwork[i]->networkId < 0) {
					cout << "shiiiiiiittttttttttt" << endl;
				}
			}
		}
	}

	{
		for (int i = 0; i < network.size(); i++) {
			if (network[i]->networkId < 0) {
				cout << "shittttt" << endl;
			}
		}

		for (int i = 0; i < sortedNetwork.size(); i++) {
			if (sortedNetwork[i]->networkId < 0) {
				cout << "shiiiiiiittttttttttt" << endl;
			}
		}
	}
	updateStereotype();
}

void Species::adjustFitness()
{
	for (int i = 0; i < network.size(); i++) {
		network[i]->adjustedFitness = network[i]->fitness / network.size();
	}
}

int Species::avgNode()
{
	if (network.size() == 0) {
		return 0;
	}
	int sum = 0;
	for (int i = 0; i < network.size(); i++) {
		sum += network[i]->nodeList.size();
	}

	return sum / network.size();
}
