#ifndef UTILS_H
#define UTILS_H
#include <random>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <map>


// Function to generate a random float between a and b
float randomFloat(float a, float b);

// Distance
float euclideanDistance(const std::vector<float>& v1, const std::vector<float>& v2);

void printMatrix(const std::vector<std::vector<float>>& matrix);

std::map<std::string, std::string> readConfig(const std::string& filePath);

#endif // UTILS_H
