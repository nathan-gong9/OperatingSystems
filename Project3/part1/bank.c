#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "account.h"

#define MAX_ACCOUNTS 100
#define MAX_PASSWORD_LENGTH 9
#define MAX_ID_LENGTH 17

// Helper function to load account information from the input file
int load_accounts(FILE *file, account accounts[]) {
    int num_accounts;
    fscanf(file, "%d", &num_accounts);

    for (int i = 0; i < num_accounts; i++) {
        char buffer[20];
        int index_number;
        fscanf(file, "%s %d", buffer, &index_number); // Skip "index" line
        fscanf(file, "%s", accounts[i].account_number); // account number
        fscanf(file, "%s", accounts[i].password); // Password
        fscanf(file, "%lf", &accounts[i].balance); // Initial balance
        fscanf(file, "%lf", &accounts[i].reward_rate); // Reward rate
        accounts[i].transaction_tracter = 0.0; // Initialize transaction tracker
        // Initialize the mutex lock for thread safety (even though not used in Part 1)
        pthread_mutex_init(&accounts[i].ac_lock, NULL);
    }

    return num_accounts;
}

// Find account by account number and verify password
account *find_account_helper(account accounts[], int num_accounts, const char *account_num, const char *password, bool check_password) {
    for (int i = 0; i < num_accounts; i++) {
        if (strcmp(accounts[i].account_number, account_num) == 0) {
        	if(!check_password){
        		return &accounts[i];
        	}
        	else{
				if(strcmp(accounts[i].password, password) == 0){
					//printf("Correct password: %s versus %s\n", password, accounts[i].password);
					return &accounts[i];
				}
			}
        }
    }
    return NULL; 
}


account *find_account(account accounts[], int num_accounts, const char *account_num, const char *password) {
	return find_account_helper(accounts, num_accounts, account_num, password, true);
}

// Process transactions
void process_transactions(FILE *file, account accounts[], int num_accounts) {
    char transaction_type;
    char src_account[MAX_ID_LENGTH];
    char dest_account[MAX_ID_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    //int invalid_count = 0;
    //int transaction_count = 0;
    //int total_count = 0;
    double amount;

    while (fscanf(file, " %c", &transaction_type) == 1) {
    	//total_count++;
        switch (transaction_type) {
            case 'D':
                fscanf(file, "%s %s %lf", src_account, password, &amount);
                account *deposit_account = find_account(accounts, num_accounts, src_account, password);
                if (deposit_account) {
                    deposit_account->balance += amount;
                    deposit_account->transaction_tracter += amount;
                    //transaction_count++;
                }
                else{
                	//invalid_count++;
                }
                break;

            case 'W':
                fscanf(file, "%s %s %lf", src_account, password, &amount);
                account *withdraw_account = find_account(accounts, num_accounts, src_account, password);
                if (withdraw_account) {
                    withdraw_account->balance -= amount; // Allow overdrawing for this part
                    withdraw_account->transaction_tracter += amount;
                    //transaction_count++;
                }
                else{
                	//invalid_count++;
                }
                break;

            case 'T':
                fscanf(file, "%s %s %s %lf", src_account, password, dest_account, &amount);
                account *source_account = find_account(accounts, num_accounts, src_account, password);
                account *destination_account = find_account_helper(accounts, num_accounts, dest_account, "", false);
                if (source_account && destination_account) {
                    source_account->balance -= amount;
                    destination_account->balance += amount;
                    source_account->transaction_tracter += amount;
                    //destination_account->transaction_tracter += amount; // Track transaction
                    //transaction_count++;
                }
                else{
                	//invalid_count++;
                }
                break;

            case 'C':
                fscanf(file, "%s %s", src_account, password);
                account *check_account = find_account(accounts, num_accounts, src_account, password);
                if (check_account) {
                    	printf("Current Savings Balance for %s: %.2f\n", check_account->account_number, check_account->balance);
                    //transaction_count++;
                }
                else{
                	//invalid_count++;
                }
                break;

            default:
                printf("Invalid transaction type: %c\n", transaction_type);
                break;
        }
    }
    //printf("Total transaction count: %d\n", total_count);
    //printf("Transaction count: %d\n", transaction_count);
    //printf("Invalid transaction count: %d\n", invalid_count);
}

// Save final account balances to file
void save_balances_to_file(const char *filename, account accounts[], int num_accounts) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    for (int i = 0; i < num_accounts; i++) {
        // Print the account index and balance in the desired format
        fprintf(file, "%d balance:\t%.2f\n", i, accounts[i].balance);
        fprintf(file, "\n");
    }

    fclose(file);
}

void calculate_rewards(account accounts[], int num_accounts){
	for(int i = 0; i < num_accounts; i++){
		accounts[i].balance += accounts[i].transaction_tracter * accounts[i].reward_rate;
	}
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening input file");
        return 1;
    }

    account accounts[MAX_ACCOUNTS];
    int num_accounts = load_accounts(file, accounts); 

    process_transactions(file, accounts, num_accounts); 
    calculate_rewards(accounts, num_accounts);
    save_balances_to_file("output.txt", accounts, num_accounts);
    
    for (int i = 0; i < num_accounts; i++) {
    	pthread_mutex_destroy(&accounts[i].ac_lock);
	}

    fclose(file);
    return 0;
}
