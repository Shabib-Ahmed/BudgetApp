#include "CSVReader.h"
#include <iostream>
#include <fstream>
#include <sstream>

CSVData CSVReader::readCSV(const std::string& filename) {
    CSVData data;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return data;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> row;

        while (std::getline(ss, cell, ',')) {
            row.push_back(cell);
        }
        data.push_back(row);
    }

    file.close();
    return data;
}

bool CSVReader::writeCSV(const std::string& filename, const CSVData& data) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << " for writing." << std::endl;
        return false;
    }

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& row = data[i];
        
        for (size_t j = 0; j < row.size(); ++j) {
            std::string cell = row[j];

            if (cell.find(',') != std::string::npos) {
                cell = "\"" + cell + "\"";
            }

            file << cell;

            if (j < row.size() - 1) {
                file << ",";
            }
        }
        
        file << "\n";
    }

    file.close();
    return true;
}