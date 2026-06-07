#pragma once
#include <vector>
#include <string>
#include "Transaction.h"
#include "csvLibrary/CSVReader.h" 

class BudgetManager {
private:
    std::vector<Transaction> transactions;
    std::string currentFilename;

public:
    // Loads raw data csv
    bool loadRawDataFile(const std::string& filename);
    // Loads from Local Budget csv
    bool loadBudgetFile(const std::string& filename);
    //add transaction to the dataset
    void addTransaction(const Transaction& t);
    //saves to Local Budget csv
    bool saveBudgetFile();
};