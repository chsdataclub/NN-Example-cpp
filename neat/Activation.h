#pragma once

double tanh(double value);
double tanhDerivative(double value);

double sigmoid(double value);
double sigmoidDerivative(double value);

double lRelu(double value);
double lReluDerivative(double value);

int random(int f, int t);
double random(double f, double t);
void randInit();