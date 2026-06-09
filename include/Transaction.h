#pragma once
#include <string>

struct Transaction {
    int year;
    int month;
    int day; 
    std::string Description; 
    std::string Category; 
    std::string transactionHash;
    double amount;
};