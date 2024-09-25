#ifndef UTILS_H
#define UTILS_H
#include <random>
#include <iostream>

// Function to generate a random float between a and b
float randomFloat(float a, float b);

// Distance
float euclideanDistance(const std::vector<float>& v1, const std::vector<float>& v2);

void printMatrix(const std::vector<std::vector<float>>& matrix);
#endif // UTILS_H
