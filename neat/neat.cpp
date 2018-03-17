#include "Neat.h"
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>    // std::find
#include <iostream>
#include <vector>       // std::vector
#include <utility>
#include <array>
#include "Activation.h"
#include <thread>
#include <fstream>
using namespace std;

Neat::Neat(int numNetworks, int input, int output, double mutate, double lr, double(*activation)(double value), double(*activationDerivative)(double value)) : nodeMutate(mutate)
{
	threads = vector<std::thread>(8);
	speciesThreshold = .01;
	for (int i = output; i < input + output; i++) {
		for (int a = 0; a < output; a++) {
			pair<int, int> c = { i,a };
			connectionInnovation.push_back(c);
		}
	}

	network.reserve(numNetworks);

	for (int i = 0; i < numNetworks; i++) {
		network.push_back(Network(input, output, i, 0, lr, true, activation, activationDerivative));
	}

	createSpecies(0, network.size() % 5 + network.size() / 5);
	species[0].innovationDict = &connectionInnovation;
	for (int i = network.size() % 5 + (network.size() / 5); i + (network.size() / 5) <= network.size(); i += (network.size() / 5)) {
		createSpecies(i, i + (network.size() / 5));
	}

	{
		int sum = 0;
		for (int i = 0; i < species.size(); i++) {
			sum += species[i].network.size();
		}

		if (sum > numNetworks) {
			cout << "shit" << endl;
		}
	}
	for (int i = 0; i < species.size(); i++) {
		for (int a = 0; a < species[i].network.size(); a++) {
			species[i].mutateNetwork(*species[i].network[a]);
		}
	}

	{
		for (int i = 0; i < network.size(); i++) {
			if (network[i].networkId < 0) {
				cout << "shit 1" << endl;
			}
		}
	}

	mutatePopulation();
	mutatePopulation();
	mutatePopulation();

	speciateAll();

	{
		int sum = 0;
		for (int i = 0; i < species.size(); i++) {
			sum += species[i].network.size();
		}

		if (sum > numNetworks) {
			cout << "shit" << endl;
		}
	}

	{
		for (int i = 0; i < network.size(); i++) {
			if (network[i].networkId < 0) {
				cout << "shit 1" << endl;
			}
		}
	}
}

Network Neat::start(vector<pair<vector<double>, vector<double>>>& input, vector<pair<vector<double>, vector<double>>>& valid, int cutoff, double target, Network& bestNet)
{
	int strikes = cutoff;
	cout << isInput(network[0].getNode(3)) << " " << isOutput(network[0].getNode(3)) << endl;
	double bestFit = 0;

	trainNetworks(input, valid);

	for (int z = 0; strikes > 0 && bestFit < target; z++) {
		cout << "//////////////////////////////////////////////////////////////" << endl;
		cout << "/////////////////////" << endl;

		mateSpecies();

		{
			int sum = 0;
			for (int i = 0; i < species.size(); i++) {
				sum += species[i].network.size();
			}

			if (sum > 20) {
				cout << "shit" << endl;
			}
		}

		{
			for (int i = 0; i < network.size(); i++) {
				if (network[i].networkId < 0) {
					cout << "shit 1" << endl;
				}
			}
		}

		trainNetworks(input, valid);

		if (z % 5 == 0) {
			speciateAll();

			{
				int sum = 0;
				for (int i = 0; i < species.size(); i++) {
					sum += species[i].network.size();
				}

				if (sum > 20) {
					cout << "shit" << endl;
				}
			}

			{
				for (int i = 0; i < network.size(); i++) {
					if (network[i].networkId < 0) {
						cout << "shit 1" << endl;
					}
				}
			}
		}

		//determines the best
		int bestIndex = -1;
		for (int i = 0; i < network.size(); i++) {
			if (bestFit < network[i].fitness) {
				bestFit = network[i].fitness;
				bestIndex = i;
			}
		}

		//compares the best
		if (bestIndex != -1) {
			network[bestIndex].printNetwork();
			clone(network[bestIndex], bestNet, &connectionInnovation);
			bestNet.printNetwork();
			strikes = cutoff;

			ofstream myfile("bestnet.txt");
			myfile << bestNet.input.size() - 1 << endl;
			myfile << bestNet.output.size() << endl;
			myfile << bestNet.nodeList.size() - bestNet.input.size() - bestNet.output.size() << endl;
			for (int i = 0; i < bestNet.nodeList.size(); i++) {
				for (int a = 0; a < bestNet.nodeList[i].send.size(); a++) {
					myfile << bestNet.nodeList[i].id << " " << bestNet.nodeList[i].send[a].nodeTo->id << " " << bestNet.nodeList[i].send[a].weight << endl;
				}
			}
			myfile.flush();
			myfile.close();
		}
		else {
			strikes--;
			mutatePopulation();
			if (z % 5 != 0) {
				speciateAll();
			}
		}

		cout << "best" << endl;
		bestNet.printNetwork();
		cout << "epoch:" << z << " best: " << bestFit << endl;
		cout << endl;
	}

	return bestNet;
}

void Neat::mutatePopulation()
{
	int numNet = random(3, network.size() / 5 + 3); // rand() % ((network.size() - 3) / 5) + 3;
	for (int i = 0; i < numNet; i++) {
		int species = random(0, this->species.size() - 1); // int(rand() % (this->species.size()));

		this->species[species].mutateNetwork(this->species[species].getNetworkAt(random(0, this->species[species].network.size() - 1)));//rand() % this->species[species].network.size()));
	}
}

void Neat::trainNetworks(vector<pair<vector<double>, vector<double>>>& input, vector<pair<vector<double>, vector<double>>>& valid)
{
	threads.clear();
	for (int i = 0; i < species.size(); i++) {
		threads.push_back(thread(&Species::trainNetworks, &species[i], input, valid));
	}

	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}

void Neat::mateSpecies()
{
	threads.clear();

	for (int i = 0; i < species.size(); i++) {
		threads.push_back(thread(&Species::mateSpecies, &species[i]));
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
}

void Neat::speciateAll()
{
	for (int a = 0; a < species.size(); a++) {
		for (int i = 0; i < species[a].network.size(); i++) {
			speciate(*species[a].network[i], &species[a]);
			{
				int sum = 0;
				for (int i = 0; i < species.size(); i++) {
					sum += species[i].network.size();
				}

				if (sum != 20) {
					cout << "shit " << network.size() << endl;
				}
			}
			cout << endl;
		}
	}

	checkSpecies();
}

void Neat::checkSpecies()
{
	for (int i = 0; i < species.size(); i++) {
		vector<double> values;
		for (int a = 0; a < species.size(); a++) {
			if (a == i) {
				values.push_back(100.0);
				continue;
			}

			values.push_back(compareGenome(species[i].avgNode(), species[i].commonInnovation, species[a].avgNode(), species[a].commonInnovation));
		}

		double lValue = 1000.0;
		for (int a = 0; a < values.size(); a++) {
			if (values[a] < lValue) {
				lValue = values[a];
			}
		}

		if (lValue < speciesThreshold || species[i].network.size() < 2) { //switched direction if sign because %dif < difthreshold for it to be the same
			removeSpecies(species[i].id); //could say continue if similar so that the smaller does the hard workbut might screw with eliminating empties
			i--;
		}
	}
}

void Neat::speciate(Network& network, Species* s)
{
	cout << "stat spec " << network.species << endl;
	if (s != nullptr) {
		cout << s->id << endl;
	}
	vector<double> values;

	for (int i = 0; i < species.size(); i++) {
		values.push_back(compareGenome(network.nodeList.size(), network.innovation, species[i].avgNode(), species[i].commonInnovation));
	}

	//this should be faster than sorting the whole thing (it also retains position information)
	int bestSpec = -1;
	double lValue = 1000.0;
	for (int i = 0; i < values.size(); i++) {
		if (values[i] < lValue) {
			bestSpec = species[i].id;
			lValue = values[i];
		}
	}

	//s := n.getSpecies(network.species)
	if (lValue > speciesThreshold) { //&& s != nil && len(s.network) > 2 { //i flipped this sign i think it works better %different > differentThreshold
									 //finds the position
		cout << "1" << endl;
		int networkIndex = 0;
		for (int i = 0; i < this->network.size(); i++) {
			if (this->network[i].networkId == network.networkId) {
				networkIndex = i;
			}
		}
		if (s != nullptr) {
			s->removeNetwork(network.networkId);
		}
		int lastSpec = network.species;
		vector<Network*> pass;
		if (s != nullptr) {
			cout << s->id << endl;
		}
		pass.push_back(&this->network[networkIndex]);
		if (s != nullptr) {
			cout << s->id << endl;
		}
		Species& newSpec = createSpecies(pass);
		if (s != nullptr) {
			cout << s->id << " ns " << newSpec.id << endl;
		}
		//TODO: there will be a problem when og species can't be found

		if (s != nullptr) {
			cout << "2" << endl;
			//removes current and checks to see if the rest need to be speciated
			cout << "size b: " << s->network.size() << " " << s->id << endl;
			s->removeNetwork(network.networkId);
			cout << "size a: " << s->network.size() << endl;
			if (s != nullptr) {
				cout << s->id << endl;
			}
			for (int i = 0; i < s->network.size(); i++) {
				if (s->network[i]->networkId != network.networkId) { // && s->network[i]->species == s->id) {
																	 //	compareGenome(len(s.network[i].nodeList), s.network[i].innovation, s.avgNode(), s.commonInnovation) > compareGenome(len(s.network[i].nodeList), s.network[i].innovation, newSpec.avgNode(), newSpec.commonInnovation) {
					if (compareGenome(s->network[i]->nodeList.size(), s->network[i]->innovation, s->avgNode(), s->commonInnovation) > compareGenome(s->network[i]->nodeList.size(), s->network[i]->innovation, newSpec.avgNode(), newSpec.commonInnovation)) {
						cout << "3" << endl;
						newSpec.addNetwork(*s->network[i]);
						s->removeNetwork(s->network[i]->networkId);
						i--;
					}
				}
			}
		}

		/* THIS IS FOR CHECKING ALL NETWORKS//cannot have more than one species removed at the same time
		for (int i = 0; i < this->network.size(); i++) {
		Network& comp = this->network[i];
		if (comp.networkId != network.networkId && (s != nullptr && comp.species == lastSpec)) { // && comp.species == s->id) {
		Species& cs = getSpecies(comp.species);												 //	compareGenome(len(s.network[i].nodeList), s.network[i].innovation, s.avgNode(), s.commonInnovation) > compareGenome(len(s.network[i].nodeList), s.network[i].innovation, newSpec.avgNode(), newSpec.commonInnovation) {
		if (compareGenome(comp.nodeList.size(), comp.innovation, cs.avgNode(), cs.commonInnovation) > compareGenome(comp.nodeList.size(), comp.innovation, newSpec.avgNode(), newSpec.commonInnovation)) {
		newSpec.addNetwork(comp);
		cs.removeNetwork(comp.networkId);
		i--;
		}
		}
		}*/
		//checks to see if new species meets size requirement
		if (newSpec.network.size() < 2) {
			//reassign creator to next best in order to prevent a loop
			//newSpec.removeNetwork(network.networkId);
			cout << "4" << endl;
			if (s != nullptr) {
				cout << s->id << endl;
			}
			getSpecies(bestSpec).addNetwork(network); //could be problem because index changes when make new species (maybe because should be added to the end)
			cout << "stat spec " << network.species << " size " << getSpecies(bestSpec).network.size() << endl;
			removeSpecies(newSpec.id);
			cout << "stat spec " << network.species << endl;

		}
	}
	else if (network.species != bestSpec) {
		cout << "5" << endl;
		getSpecies(bestSpec).addNetwork(network);

		if (s != nullptr) {
			cout << "6" << endl;
			s->removeNetwork(network.networkId);
		}
	}
}

double Neat::compareGenome(int node, vector<int>& innovation, int nodeA, vector<int>& innovationA)
{
	vector<int>* larger;
	vector<int>*smaller;

	if (innovation.size() > innovationA.size()) {
		larger = &innovation;
		smaller = &innovationA;
	}
	else {
		larger = &innovationA;
		smaller = &innovation;
	}

	int missing = 0;
	if (smaller->size() == 0) {
		missing = larger->size();
	}
	else {
		for (int b = 0; b < larger->size(); b++) {
			if (find(smaller->begin(), smaller->end(), (*larger)[b]) == smaller->end()) {
				missing++;
			}
		}
	}
	return (missing / (double)larger->size()) + (abs(node - nodeA) / (double)(node + nodeA) / 2);

	//return missing + abs(node - nodeA) / (smaller->size() + ((node + nodeA) / 2));
}

void Neat::printNeat() {
	cout << endl;
	for (int i = 0; i < species.size(); i++) {
		cout << "Spec Id: " << species[i].id << " network size: " << species[i].network.size() << " " << endl;
		for (int a = 0; a < species[i].commonInnovation.size(); a++) {
			cout << species[i].commonInnovation[a] << " ";
		}
		cout << endl;
		for (int a = 0; a < species[i].network.size(); a++) {
			species[i].network[a]->printNetwork();
		}
	}
	cout << endl;
}
/*int * Neat::getInnovation(int num)
{
return nullptr;
}*/

/*int Neat::findInnovation(int search[2])
{
return 0;
}*/

Species& Neat::getSpecies(int id)
{
	for (int i = 0; i < species.size(); i++) {
		if (species[i].id == id) {
			return species[i];
		}
	}
	//TODO: need default return
}

Species& Neat::createSpecies(vector<Network*>& possible)
{
	for (int i = 0; i < possible.size(); i++) {
		possible[i]->species = speciesId;
	}

	species.push_back(Species(speciesId, possible, nodeMutate));

	speciesId++;

	return species.back();
}

Species & Neat::createSpecies(int startIndex, int endIndex)
{
	vector<Network*> possible;
	for (int i = startIndex; i < endIndex; i++) {
		network[i].species = speciesId;
		possible.push_back(&network[i]);
	}

	species.push_back(Species(speciesId, possible, nodeMutate));

	speciesId++;

	return species.back();
}

void Neat::removeSpecies(int id)
{
	for (int i = 0; i < species.size(); i++) {
		if (species[i].id == id) {
			vector<Network*> currentSpecies = species[i].network;

			species.erase(species.begin() + i);
			for (int a = 0; a < currentSpecies.size(); a++) {
				if (currentSpecies[a]->species == id) {
					cout << "speciating: " << currentSpecies[a]->networkId << endl;
					speciate(*currentSpecies[a], nullptr);
				}
			}
		}
	}
}
