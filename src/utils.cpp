#include "utils.h"


float randomFloat(float a, float b) {
    if(a > b) {
        std::cerr << "in interval linf > lsup" << std::endl;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution(a, b);
    return distribution(gen);
}

float euclideanDistance(const std::vector<float>& v1, const std::vector<float>& v2) {
    float sum = 0.0f;
    for (std::size_t i = 0; i < v1.size(); ++i) {
        sum += std::pow(v1[i] - v2[i], 2);
    }
    return std::sqrt(sum);
}

void printMatrix(const std::vector<std::vector<float>>& matrix) {
    for (const auto& row : matrix) {
        for (float element : row) {
            std::cout << element << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::map<std::string, std::string> readConfig(const std::string& filePath) {
    std::map<std::string, std::string> config;
    std::ifstream file(filePath);
    if (!file.is_open()) return config;

    std::regex pattern(R"(^\s*([^\s#]+)\s*=\s*([^#]+?)\s*$)");
    std::smatch match;
    std::string line;

    while (std::getline(file, line)) {
        // Remove anything after '#' as it's considered a comment
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos); // Keep only part before the comment
        }

        // Trim any trailing whitespace that might have remained after removing comment
        line = std::regex_replace(line, std::regex(R"(\s+$)"), "");

        if (std::regex_match(line, match, pattern)) {
            config[match[1]] = match[2];
        }
    }

    return config;
}