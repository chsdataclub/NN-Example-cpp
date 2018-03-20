#include "stdafx.h"
#include <string>
#include <iostream>
#include "Neat.h"
#include "Activation.h"
#include <fstream>
#include <sstream>
using namespace std;

void split(const std::string &s, char delim, vector<string> &result) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		result.push_back(item);
	}
}

int main()
{
	vector<pair<vector<double>, vector<double>>> dataset;
	vector<pair<vector<double>, vector<double>>> valid;
	{
		double sumts = 0.0;
		double sumtw = 0.0;

		double count = 0;

		vector<double> data;

		double last = 0.0;
		string line = "";
		ifstream myfile("krakenUSDDay.csv");
		for (int i = 0; getline(myfile, line); i++) {
			double value = -1;
			try {
				value = stod(line.string::substr(line.find(',') + 1));
			}
			catch (...) {
				cout << "caught" << endl;
			}
			if (value != -1 && i >= 2) {
				data.push_back((value - last) / last);
			}
			else if (value != -1) {
				last = value;
			}
		}

		double multtw = 2 / 13.0;
		double multts = 2 / 27.0;
		double multni = 2 / 10.0;

		double sumDev = 0.0;
		double sumt = 0.0;

		for (int i = 0; i < 26; i++) {
			if (i < 20) {
				if (i < 12) {
					sumtw += data[i];
				}
				sumt += data[i];
			}
			sumts += data[i];
		}

		double lastematw = sumtw / 12;
		double lastemats = sumts / 26;
		double lastemani = 0.0;
		for (int i = 26; i < 35; i++) {
			sumtw -= data[i - 11];
			sumts -= data[i - 25];
			sumt -= data[i - 19];

			double ematw = data[i] * multtw + lastematw * (1 - multtw);
			lastematw = ematw;
			double emats = data[i] * multts + lastemats * (1 - multts);
			lastemats = emats;
			double macd = ematw - emats;
			double emani = macd * multni + lastemani * (1 - multni);
			lastemani = emani;
		}

		srand(time(NULL));
		for (int i = 35; i < data.size() - 1; i++) {
			sumtw -= data[i - 11];
			sumts -= data[i - 25];
			sumt -= data[i - 19];
			double sumDev = 0;
			for (int i = 0; i < 20; i++) {
				sumDev += abs(data[i] - (sumt / 20));
			}
			double dev = sumDev / 20;

			double ematw = data[i] * multtw + lastematw * (1 - multtw);
			lastematw = ematw;
			double emats = data[i] * multts + lastemats * (1 - multts);
			lastemats = emats;
			double macd = ematw - emats;
			double emani = macd * multni + lastemani * (1 - multni);
			lastemani = emani;

			pair<vector<double>, vector<double>> p;
			//the value will always be positive and when grows > 1
			vector<double> in = { data[i], sumtw / 12, sumts / 26, sumt / 20, ematw, emats, macd, emani - macd, data[i] - ((sumt / 20) + 2 * dev) };
			vector<double> o = { -2 };
			p.first = in;
			p.second = o;

			if (i > 35) {
				double val = 0;
				if (data[i] > 1) {
					val = 1.0;
				}

				if (dataset.size() > 0 && dataset.back().second[0] == -2) {
					dataset.back().second[0] = val;
				}
				else {
					valid.back().second[0] = val;
				}
			}

			if (rand() / (double)RAND_MAX > .1) {
				dataset.push_back(p);
			}
			else {
				valid.push_back(p);
			}

			count++;

			sumtw += data[i];
			sumts += data[i];
			sumt += data[i];
		}
	}

	
	for (int i = 0; i < dataset[0].first.size(); i++) {
		double max = -1000000;
		double min = 100000000;
		{
			for (int a = 0; a < dataset.size(); a++) {
				double val = dataset[a].first[i];
				if (val > max) {
					max = val;
				}
				else if (val < min) {
					min = val;
				}
			}

			for (int a = 0; a < dataset.size(); a++) {
				double val = dataset[a].first[i];
				if (val > max) {
					max = val;
				}
				else if (val < min) {
					min = val;
				}
			}
		}

		for (int a = 0; a < dataset.size(); a++) {
			dataset[a].first[i] = (dataset[a].first[i] - min) / (max - min);
		}

		for (int a = 0; a < valid.size(); a++) {
			valid[a].first[i] = (valid[a].first[i] - min) / (max - min);
		}
	}

	for (int i = 0; i < valid.size(); i++) {
		if (valid[i].second[0] == -2) {
			valid.erase(valid.begin() + i);
		}
	}

	for (int i = 0; i < dataset.size(); i++) {
		if (dataset[i].second[0] == -2) {
			dataset.erase(dataset.begin() + i);
		}
	}

	randInit();

	for (int i = 0; i < 100; i++) {
		cout << random(-1.0, 1.0) << endl;
	}

	Network winner(9, 1, 0, 0, .1, false, &sigmoid, &sigmoidDerivative);
	winner.nodeList.clear();
	winner.output.clear();
	winner.innovation.clear();

	//Neat neat = Neat(250, 9, 1, .3, .1, &sigmoid, &sigmoidDerivative);

	//0 indicates sell
	//neat.start(dataset, valid, 100, 10000, winner);

	ifstream net("bestnet.txt");
	string line;
	for (int i = 0; getline(net, line); i++) {
		vector<string> in;
		split(line, ' ', in);
		if (i == 0) {
			winner = Network(stoi(in[0]), stoi(in[1]), 0, 0, .1, false, stringtoAct(in[2]), stringtoDeriv(in[2]));
		}
		else if (in.size() == 3) {
			winner.mutateConnection(stoi(in[0]), stoi(in[1]), 0, stod(in[2]));
		}
		else if (in.size() == 2) {
			winner.createNode(100, stringtoAct(in[1]), stringtoDeriv(in[1]));
		}
	}
	winner.printNetwork();
	//neat.printNeat()
	cout << endl;

	winner.trainset(dataset, valid, 100000);

	//printNetwork(&winner);
	cout << "best " << winner.fitness << " error " << 1 / winner.fitness << endl;
	//cout << "result " << winner.process(dataset[0].first)[0] << winner.process(dataset[100].first)[0] << winner.process(dataset[150].first)[0] << winner.process(dataset[3].first)[250] << endl; //1 1 0 0
	for (int i = 0; i < valid.size(); i++) {
		cout << winner.process(valid[i].first)[0] << " vs " << valid[i].second[0] << " dif " << (winner.process(valid[i].first)[0] - valid[i].second[0]) << endl;
	}
	cout << "done";

	ofstream myfile("bestnet.txt");
	myfile << winner.input.size() - 1 << " " << winner.output.size() << " " << acttoString(winner.nodeList[0].activation).c_str() << endl;
	for (int i = 0; i < winner.nodeList.size(); i++) {
		Node& n = winner.nodeList[i];
		myfile << n.id << " " << acttoString(n.activation).c_str() << endl;
	}
	for (int i = 0; i < winner.nodeList.size(); i++) {
		for (int a = 0; a < winner.nodeList[i].send.size(); a++) {
			myfile << winner.nodeList[i].id << " " << winner.nodeList[i].send[a].nodeTo->id << " " << winner.nodeList[i].send[a].weight << endl;
		}
	}
	myfile.flush();
	myfile.close();
	system("pause");
	return 0;
}
