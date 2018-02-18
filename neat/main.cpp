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
	//control = make([][][]float64, 0, 100)
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
			vector<double> in = { data[i], sumtw / 12, sumts / 26, sumt / 20, ematw, emats, macd, emani - macd, data[i] - ((sumt / 20) + 2 * dev) };
			vector<double> o = { -1 };
			p.first = in;
			p.second = o;
			dataset.push_back(p);

			if (i > 35) {
				double val = -1.0;
				if (data[i] > 0) {
					val = 1.0;
				}
				dataset[count - 1].second[0] = val;
			}

			count++;

			sumtw += data[i];
			sumts += data[i];
			sumt += data[i];
		}
	}


	randInit();

	Network winner(0, 0, 0, 0, 0.0, false);
	Neat neat = Neat(100, 9, 1, .3, .1);

	winner = neat.start(dataset, 100, 10000);
	//neat.printNeat()

	cout << endl;

	//printNetwork(&winner);
	cout << "best " << winner.fitness << "error" << 1 / winner.fitness << endl;
	//cout << "result " << winner.process(data[0].first) << winner.process(data[1].first) << winner.process(data[2].first) << winner.process(data[3].first) << endl; //1 1 0 0
	cout << "done";
	system("pause");
	return 0;
}
