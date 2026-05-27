#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

// Global Constants
const double MINIMUM_BALANCE = 1000.00;
const string ACCOUNTS_FILE = "accounts.txt";
const string TRANSACTIONS_FILE = "transactions.txt";

// ==========================================
// 1. INPUT VALIDATION UTILITIES
// ==========================================
class Validator {
public:
    static int getValidInteger(string prompt) {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cout << "❌ Invalid input. Please enter a valid number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    static double getValidDouble(string prompt) {
        double value;
        while (true) {
            cout << prompt;
            if (cin >> value && value >= 0) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cout << "❌ Invalid amount. Please enter a positive decimal number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    static string getValidString(string prompt) {
        string str;
        cout << prompt;
        getline(cin, str);
        while (str.empty()) {
            cout << "❌ Field cannot be empty. Try again: ";
            getline(cin, str);
        }
        return str;
    }
};

// ==========================================
// 2. TRANSACTION CLASS
// ==========================================
class Transaction {
private:
    int accountNumber;
    string type; // "Deposit" or "Withdrawal"
    double amount;

public:
    Transaction(int accNum, string t, double amt) : accountNumber(accNum), type(t), amount(amt) {}

    void logToFile() const {
        ofstream fout(TRANSACTIONS_FILE, ios::app);
        if (fout) {
            fout << accountNumber << "," << type << "," << amount << "\n";
        }
    }

    static void displayHistory(int accNum) {
        ifstream fin(TRANSACTIONS_FILE);
        if (!fin) return;

        string line;
        int num;
        string type;
        double amt;
        bool found = false;

        cout << "\n=== TRANSACTION HISTORY FOR ACCOUNT #" << accNum << " ===\n";
        cout << left << setw(15) << "Type" << setw(15) << "Amount" << "\n";
        cout << "-----------------------------------\n";

        while (fin >> num) {
            fin.ignore(); // skip comma
            getline(fin, type, ',');
            fin >> amt;
            fin.ignore(); // skip newline

            if (num == accNum) {
                cout << left << setw(15) << type << "$" << setw(14) << fixed << setprecision(2) << amt << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No transactions recorded yet.\n";
        }
        cout << "-----------------------------------\n";
    }
};

// ==========================================
// 3. ACCOUNT CLASS
// ==========================================
class Account {
private:
    int accountNumber;
    string name;
    string accountType; // "Savings" or "Current"
    double balance;

public:
    Account() : accountNumber(0), balance(0.0) {}
    
    Account(int accNum, string n, string type, double bal) {
        accountNumber = accNum;
        name = n;
        accountType = type;
        balance = bal;
    }

    // Getters
    int getAccountNumber() const { return accountNumber; }
    string getName() const { return name; }
    double getBalance() const { return balance; }

    // Setters / Operations
    void deposit(double amount) {
        balance += amount;
        Transaction t(accountNumber, "Deposit", amount);
        t.logToFile();
    }

    bool withdraw(double amount) {
        if (balance - amount < MINIMUM_BALANCE) {
            cout << "❌ Transaction Denied! Maintenance of minimum balance ($" << MINIMUM_BALANCE << ") required.\n";
            return false;
        }
        balance -= amount;
        Transaction t(accountNumber, "Withdrawal", amount);
        t.logToFile();
        return true;
    }

    // Display Account Card
    void display() const {
        cout << left << setw(12) << accountNumber 
             << setw(20) << name 
             << setw(12) << accountType 
             << "$" << fixed << setprecision(2) << balance << "\n";
    }

    // File I/O Helpers (Comma Separated Values)
    void writeToFile(ofstream &fout) const {
        fout << accountNumber << "," << name << "," << accountType << "," << balance << "\n";
    }

    void readFromFile(ifstream &fin) {
        string accNumStr, balStr;
        getline(fin, accNumStr, ',');
        if (accNumStr.empty()) return;
        
        accountNumber = stoi(accNumStr);
        getline(fin, name, ',');
        getline(fin, accountType, ',');
        getline(fin, balStr);
        balance = stod(balStr);
    }
};

// ==========================================
// 4. BANK MANAGER CLASS (File Orchestration)
// ==========================================
class BankManager {
private:
    vector<Account> accounts;

    void loadAllAccounts() {
        accounts.clear();
        ifstream fin(ACCOUNTS_FILE);
        if (!fin) return; // File doesn't exist yet

        while (fin.peek() != EOF) {
            Account acc;
            acc.readFromFile(fin);
            if (acc.getAccountNumber() != 0) {
                accounts.push_back(acc);
            }
        }
        fin.close();
    }

    void saveAllAccounts() {
        ofstream fout(ACCOUNTS_FILE, ios::trunc);
        for (const auto &acc : accounts) {
            acc.writeToFile(fout);
        }
        fout.close();
    }

public:
    BankManager() {
        loadAllAccounts();
    }

    void createAccount() {
        cout << "\n--- Create New Account ---\n";
        int accNum = Validator::getValidInteger("Enter Unique Account Number: ");
        
        // Check if account number already exists
        for (const auto& acc : accounts) {
            if (acc.getAccountNumber() == accNum) {
                cout << "❌ Error: Account number already exists!\n";
                return;
            }
        }

        string name = Validator::getValidString("Enter Account Holder Name: ");
        
        int typeChoice = Validator::getValidInteger("Select Account Type (1. Savings, 2. Current): ");
        string type = (typeChoice == 2) ? "Current" : "Savings";

        double initialBal = Validator::getValidDouble("Enter Initial Deposit: ");
        if (initialBal < MINIMUM_BALANCE) {
            cout << "❌ Error: Initial deposit must be at least $" << MINIMUM_BALANCE << "\n";
            return;
        }

        Account newAcc(accNum, name, type, initialBal);
        accounts.push_back(newAcc);
        saveAllAccounts();
        
        // Log the initial deposit transaction
        Transaction t(accNum, "Init_Deposit", initialBal);
        t.logToFile();

        cout << "✅ Account Created Successfully!\n";
    }

    void viewAllAccounts() {
        loadAllAccounts();
        if (accounts.empty()) {
            cout << "\n📭 No accounts found in the system.\n";
            return;
        }

        cout << "\n=======================================================\n";
        cout << left << setw(12) << "Acc No." << setw(20) << "Name" << setw(12) << "Type" << "Balance\n";
        cout << "=======================================================\n";
        for (const auto &acc : accounts) {
            acc.display();
        }
        cout << "=======================================================\n";
    }

    void searchAccount() {
        cout << "\nSearch by: 1. Account Number  2. Name: ";
        int choice = Validator::getValidInteger("");
        bool found = false;

        if (choice == 1) {
            int accNum = Validator::getValidInteger("Enter Account Number: ");
            for (const auto &acc : accounts) {
                if (acc.getAccountNumber() == accNum) {
                    cout << "\n🔍 Account Found:\n";
                    acc.display();
                    Transaction::displayHistory(accNum);
                    found = true;
                    break;
                }
            }
        } else {
            string name = Validator::getValidString("Enter Holder Name: ");
            // Transform to lowercase for case-insensitive search
            string lowerName = name;
            transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

            for (const auto &acc : accounts) {
                string currentName = acc.getName();
                transform(currentName.begin(), currentName.end(), currentName.begin(), ::tolower);

                if (currentName.find(lowerName) != string::npos) {
                    if (!found) cout << "\n🔍 Matching Account(s) Found:\n";
                    acc.display();
                    found = true;
                }
            }
        }

        if (!found) cout << "❌ Account not found.\n";
    }

    void handleDeposit() {
        int accNum = Validator::getValidInteger("\nEnter Account Number for Deposit: ");
        for (auto &acc : accounts) {
            if (acc.getAccountNumber() == accNum) {
                double amount = Validator::getValidDouble("Enter Amount to Deposit: ");
                acc.deposit(amount);
                saveAllAccounts();
                cout << "✅ Deposit Successful! New Balance: $" << acc.getBalance() << "\n";
                return;
            }
        }
        cout << "❌ Account not found.\n";
    }

    void handleWithdrawal() {
        int accNum = Validator::getValidInteger("\nEnter Account Number for Withdrawal: ");
        for (auto &acc : accounts) {
            if (acc.getAccountNumber() == accNum) {
                double amount = Validator::getValidDouble("Enter Amount to Withdraw: ");
                if (acc.withdraw(amount)) {
                    saveAllAccounts();
                    cout << "✅ Withdrawal Successful! New Balance: $" << acc.getBalance() << "\n";
                }
                return;
            }
        }
        cout << "❌ Account not found.\n";
    }

    void deleteAccount() {
        int accNum = Validator::getValidInteger("\nEnter Account Number to DELETE: ");
        auto it = remove_if(accounts.begin(), accounts.end(), [accNum](const Account &acc) {
            return acc.getAccountNumber() == accNum;
        });

        if (it != accounts.end()) {
            accounts.erase(it, accounts.end());
            saveAllAccounts();
            cout << "🗑️ Account deleted successfully from records.\n";
        } else {
            cout << "❌ Account not found.\n";
        }
    }
};

// ==========================================
// 5. MAIN MENU SYSTEM
// ==========================================
int main() {
    BankManager bank;
    int choice;

    do {
        cout << "\n====================================\n";
        cout << "    BANK MANAGEMENT SYSTEM MENU     \n";
        cout << "====================================\n";
        cout << "1. Create New Account\n";
        cout << "2. View All Accounts\n";
        cout << "3. Search Account & Statement\n";
        cout << "4. Deposit Money\n";
        cout << "5. Withdraw Money\n";
        cout << "6. Delete Account\n";
        cout << "7. Exit Application\n";
        cout << "====================================\n";
        
        choice = Validator::getValidInteger("Enter your choice (1-7): ");

        switch (choice) {
            case 1: bank.createAccount(); break;
            case 2: bank.viewAllAccounts(); break;
            case 3: bank.searchAccount(); break;
            case 4: bank.handleDeposit(); break;
            case 5: bank.handleWithdrawal(); break;
            case 6: bank.deleteAccount(); break;
            case 7: cout << "\nThank you for using our Banking System. Goodbye!\n"; break;
            default: cout << "❌ Invalid Choice! Please select between 1 and 7.\n";
        }
    } while (choice != 7);

    return 0;
}
