#include <math.h>
#include <random>
#include <ctime>
#include "Activation.h"
#include <mutex>
using namespace std;

typedef std::mt19937 MyRNG;  // the Mersenne Twister with a popular choice of parameters
uint32_t seed_val;           // populate somehow

MyRNG rng;                   // e.g. keep one global instance (per thread)

double tanh(double value) {
	return (pow(2.71, value) - pow(2.71, -1 * value)) / (pow(2.71, value) + pow(2.71, -1 * value));
}
double tanhDerivative(double value) {
	return 1 - pow(tanh(value), 2);
}
double sigmoid(double value) {
	return 1 / (1 + (1 / pow(2.71, value)));
}
double sigmoidDerivative(double value) {
	return sigmoid(value) * (1 - sigmoid(value));
}
double lRelu(double value)
{
	if (value >= 0) {
		return value;
	}
	else {
		return .01*value;
	}
}
double lReluDerivative(double value)
{
	if (value < 0) {
		return .01;
	}
	else {
		return 1;
	}
}
void randInit() {
	rng.seed(time(NULL));
}

//these are the new add and read methods
mutex m;
pair<int, int> safeRead(vector<pair<int, int>>& connectionInnovation, int a) {
	lock_guard<mutex> fa(m);
	return connectionInnovation[a];
}
int safeWrite(vector<pair<int, int>>& connectionInnovation, int a, int b) {
	lock_guard<mutex> w(m);
	connectionInnovation.push_back(pair<int, int>(a, b));
	return connectionInnovation.size() - 1;
}

//generates number [f,t]
int random(int f, int t)
{
	std::uniform_int_distribution<uint32_t> uint_dist10(f, t);

	return uint_dist10(rng);
}

double random(double f, double t)
{
	int a = f * RAND_MAX;
	int b = t * RAND_MAX;
	std::uniform_int_distribution<int32_t> uint_dist10(a, b);

	return uint_dist10(rng) / (double)RAND_MAX;
}

