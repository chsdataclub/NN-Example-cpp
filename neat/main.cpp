#include "stdafx.h"
#include <string>
#include <iostream>
#include "Neat.h"
#include "Activation.h"
#include <fstream>
using namespace std;

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
				value = stod(line.string::substr(line.find(',')+1));
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

			if (rand()/(double)RAND_MAX > .1) {
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
			dataset[a].first[i] = (dataset[a].first[i] - min) / (max - min);
		}
	}

	randInit();

	for (int i = 0; i < 100; i++) {
		cout << random(-1.0, 1.0) << endl;
	}

	Network winner(9, 1, 0, 0, .1, false , &sigmoid, &sigmoidDerivative);
	Neat neat = Neat(20, 9, 1, .3, .1, &sigmoid, &sigmoidDerivative);

	//neg 1 indicates sell
	neat.start(dataset, valid, 100, 10000, winner);
	//neat.printNeat()

	cout << endl;

	//printNetwork(&winner);
	cout << "best " << winner.fitness << "error " << 1 / winner.fitness << endl;
	//cout << "result " << winner.process(dataset[0].first)[0] << winner.process(dataset[100].first)[0] << winner.process(dataset[150].first)[0] << winner.process(dataset[3].first)[250] << endl; //1 1 0 0
	for (int i = 0; i < valid.size(); i++) {
		cout << winner.process(valid[i].first)[0] << " vs " << valid[i].second[0] << " dif " << (winner.process(valid[i].first)[0]-valid[i].second[0]) << endl;
	}
	cout << "done";

	ofstream myfile("bestnet.txt");
	myfile << winner.input.size()-1 << endl;
	myfile << winner.output.size() << endl;
	myfile << winner.nodeList.size()-winner.input.size()-winner.output.size() << endl;
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
