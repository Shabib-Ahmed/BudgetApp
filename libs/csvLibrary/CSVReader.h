#ifndef CSV_READER_H
#define CSV_READER_H

#include <string>
#include <vector>

using CSVData = std::vector<std::vector<std::string>>;

class CSVReader {
public:
    static CSVData readCSV(const std::string& filename);
    static bool writeCSV(const std::string& filename, const CSVData& data);
};

#endif