#include <iostream>
#include "csvLibrary/CSVReader.h"

int main() {
    CSVData myData = CSVReader::readCSV("data.csv");

    if (myData.empty()) {
        return 1;
    }

    std::cout << "--- Printing Data" << std::endl;

    for (const auto& row : myData) {
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << row[i];
            if (i < row.size() - 1) {
                std::cout << " \t| ";
            }
        }
        std::cout << std::endl;
    }

    return 0;
}