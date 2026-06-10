#include "BudgetManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <functional>

bool parseDate(const std::string& dateStr, int& year, int& month, int& day) {
    if (dateStr.empty()) return false;
    
    std::stringstream ss(dateStr);
    std::string m, d, y;

    if (std::getline(ss, m, '/') && std::getline(ss, d, '/') && std::getline(ss, y, '/')) {
        try {
            month = std::stoi(m);
            day = std::stoi(d);
            year = std::stoi(y);
            return true;
        } catch (...) {
            return false; 
        }
    }
    return false;
}

std::string generateHash(int y, int m, int d, const std::string& desc, double amt) {
    std::stringstream ss;
    ss << y << "-" << m << "-" << d << "|" << desc << "|" << std::fixed << std::setprecision(2) << amt;
    
    std::hash<std::string> hasher;
    size_t hashValue = hasher(ss.str());
    
    return std::to_string(hashValue);
}

bool BudgetManager::loadRawDataFile(const std::string& filename) {
    CSVReader reader;
    CSVData rawData = reader.readCSV(filename);

    if (rawData.empty()) {
        std::cerr << "Error: No data loaded from raw file: " << filename << "\n";
        return false;
    }

    std::unordered_set<std::string> existingHashes;
    for (const auto& t : transactions) {
        existingHashes.insert(t.transactionHash);
    }

    for (size_t i = 1; i < rawData.size(); ++i) {
        const auto& row = rawData[i];

        if (row.size() < 6) continue; 

        std::string rawDate = row[0];
        std::string desc = row[2];
        std::string cat = row[3];
        std::string rawAmount = row[5];

        int year = 0, month = 0, day = 0;
        if (!parseDate(rawDate, year, month, day)) {
            continue; 
        }

        double amount = 0.0;
        try {
            amount = std::stod(rawAmount);
        } catch (...) {
            continue;
        }

        std::string hash = generateHash(year, month, day, desc, amount);

        if (existingHashes.find(hash) != existingHashes.end()) {
            continue;
        }

        Transaction t{year, month, day, desc, cat, hash, amount};
        transactions.push_back(t);
        existingHashes.insert(hash); 
    }

    return true;
}

bool BudgetManager::loadBudgetFile(const std::string& filename) {
    currentFilename = filename; 

    CSVReader reader;
    CSVData localData = reader.readCSV(filename);

    if (localData.empty()) {
        return false; 
    }

    transactions.clear(); 

    for (size_t i = 1; i < localData.size(); ++i) {
        const auto& row = localData[i];
        if (row.size() < 7) continue;

        Transaction t;
        t.year = std::stoi(row[0]);
        t.month = std::stoi(row[1]);
        t.day = std::stoi(row[2]);
        t.Description = row[3];
        t.Category = row[4];
        t.transactionHash = row[5];
        t.amount = std::stod(row[6]);

        transactions.push_back(t);
    }
    return true;
}

void BudgetManager::addTransaction(const Transaction& t) {
    for (const auto& existing : transactions) {
        if (existing.transactionHash == t.transactionHash) {
            std::cout << "Transaction already exists!\n";
            return;
        }
    }
    transactions.push_back(t);
}

bool BudgetManager::saveBudgetFile() {
    if (currentFilename.empty()) return false;

    CSVData outputData;

    outputData.push_back({"Year", "Month", "Day", "Description", "Category", "Hash", "Amount"});

    for (const auto& t : transactions) {
        std::vector<std::string> row = {
            std::to_string(t.year),
            std::to_string(t.month),
            std::to_string(t.day),
            t.Description,
            t.Category,
            t.transactionHash,
            std::to_string(t.amount)
        };
        outputData.push_back(row);
    }

    return CSVReader::writeCSV(currentFilename, outputData);
}

std::vector<Transaction> BudgetManager::getTransactionsMonth(int month, int year) const {
    std::vector<Transaction> filteredTransactions;
    for (const auto& t : transactions) {
        if (t.month == month && t.year == year) {
            filteredTransactions.push_back(t);
        }
    }
    return filteredTransactions;
}

std::vector<Transaction> BudgetManager::getTransactionsYear(int year) const {
    std::vector<Transaction> filteredTransactions;
    for (const auto& t : transactions) {
        if (t.year == year) {
            filteredTransactions.push_back(t);
        }
    }
    return filteredTransactions;
}

bool BudgetManager::changeTransactionCategory(const std::string& hash, const std::string& newCategory) {
    for (auto& t : transactions) {
        if (t.transactionHash == hash) {
            t.Category = newCategory; 
            return true;              
        }
    }
    return false; 
}