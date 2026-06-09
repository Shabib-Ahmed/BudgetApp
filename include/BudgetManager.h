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
    bool loadRawDataFile(const std::string& filename);
    bool loadBudgetFile(const std::string& filename);
    bool saveBudgetFile();
    
    void addTransaction(const Transaction& t);
    
    std::vector<Transaction> getTransactionsMonth(int month, int year) const;
    std::vector<Transaction> getTransactionsYear(int year) const;
    
    bool changeTransactionCategory(const std::string& hash, const std::string& newCategory);
};