#pragma once

class Node;

class Connection {
public:
	double weight;
	double nextWeight;
	Node* nodeTo;
	Node* nodeFrom;
	int innovation;
	double momentum;
	double velocity;
	double beta;
	double betaA;
	double epsilon;

	Connection(Node* fromNode, Node* toNode, int inNumber);

	void randWeight();
	void notifyValue();
	void notifyInfluence();

	double setWeight(double v);
};