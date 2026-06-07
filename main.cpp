#include <iostream>
#include "include/BudgetManager.h" 

int main() {
    std::cout << "   Budgeting App: Data Intake Test       \n";

    BudgetManager manager;

    std::string rawDataFile = "data.csv";
    std::cout << "[Step 1] Loading raw bank data from: " << rawDataFile << "...\n";
    
    if (manager.loadRawDataFile(rawDataFile)) {
        std::cout << "--> SUCCESS: Raw data parsed, filtered, and hashed smoothly!\n\n";
    } else {
        std::cerr << "--> ERROR: Failed to read or parse " << rawDataFile << "\n";
        std::cerr << "Make sure the file exists in your project working directory.\n";
        return 1;
    }

    std::string budgetDbFile = "local_budget.csv";
    std::cout << "[Step 2] Saving structured data to local database: " << budgetDbFile << "...\n";
    
    manager.loadBudgetFile(budgetDbFile); 

    if (manager.saveBudgetFile()) {
        std::cout << "--> SUCCESS: Clean budget file generated successfully!\n\n";
    } else {
        std::cerr << "--> ERROR: Failed to save data to " << budgetDbFile << "\n";
        return 1;
    }

    std::cout << "[Step 3] Verifying database integrity by reloading " << budgetDbFile << "...\n";
    if (manager.loadBudgetFile(budgetDbFile)) {
        std::cout << "--> SUCCESS: Database reloaded perfectly. Test complete!\n\n";
    } else {
        std::cerr << "--> ERROR: Could not read back the file we just saved!\n";
        return 1;
    }

    return 0;
}