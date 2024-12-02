#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "account.h"

#define MAX_ACCOUNTS 100
#define MAX_PASSWORD_LENGTH 9
#define MAX_ID_LENGTH 17
#define TRANSACTION_LIMIT 5000

// Worker and bank threads
pthread_t *workers;
pthread_t bank_thread;

pthread_barrier_t start_barrier;
pthread_cond_t update_condition;
pthread_mutex_t update_mutex;
pthread_mutex_t transaction_mutex;
pthread_cond_t worker_condition;
int total_transactions = 0;
int transaction_count = 0;
int num_transactions;
bool bank_updating = false;

account accounts[MAX_ACCOUNTS];
int num_accounts;


typedef struct {
    char file[256];
    int start_index;
    int end_index;
} transaction_info;

// Helper function to load account information from the input file
int load_accounts(FILE *file) {
    int account_num;
    fscanf(file, "%d", &account_num);

    for (int i = 0; i < account_num; i++) {
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

    return account_num;
}

// Find account by account number and verify password
account *find_account_helper(const char *account_num, const char *password, bool check_password) {
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


account *find_account(const char *account_num, const char *password) {
	return find_account_helper(account_num, password, true);
}

int get_total_transaction_count(FILE *file) {
    char transaction_type;
    int count = 0;
	char temp_str1[256];
	char temp_str2[256];
	char temp_str3[256];
	double temp_double;
    while (fscanf(file, " %c", &transaction_type) == 1) {
        count++;
        if (transaction_type == 'D' || transaction_type == 'W') {
            fscanf(file, "%s %s %lf", temp_str1, temp_str2, &temp_double);
        } else if (transaction_type == 'T') {
            fscanf(file, "%s %s %s %lf", temp_str1, temp_str2, temp_str3, &temp_double);
        } else if (transaction_type == 'C') {
            fscanf(file, "%s %s", temp_str1, temp_str2);
        }
    }
    rewind(file);
    return count;
}

// Process transactions
void* process_transaction(void* arg) {
	printf("Initialized process_transaction\n");
	transaction_info* info = (transaction_info*) arg;

    FILE *file = fopen(info->file, "r");
    if (file == NULL) {
        perror("Error opening file");
        pthread_exit(NULL);
    }
    int start_index = info->start_index;
    int end_index = info->end_index;

    char transaction_type;
    char src_account[MAX_ID_LENGTH];
    char dest_account[MAX_ID_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    double amount;
    
    account *transaction_account = NULL;
	account *source_account = NULL;
	account *destination_account = NULL;
	
	int line_counter = 0;
	char buffer[256];
	while (line_counter < info->start_index && fgets(buffer, sizeof(buffer), file)) {
        line_counter++;  // increment the line_counter as we traverse the file
    }
    
    pthread_barrier_wait(&start_barrier);
    printf("waiting at start barrier");

	
	for(int i = start_index; i < end_index; i++) {
		printf("processing a transaction\n");
		pthread_mutex_lock(&update_mutex);
        while (bank_updating) {
            pthread_cond_wait(&worker_condition, &update_mutex);
        }
        pthread_mutex_unlock(&update_mutex);
		
		fscanf(file, " %c", &transaction_type);
		transaction_count++;
        switch (transaction_type) {
            case 'D':
                fscanf(file, "%s %s %lf", src_account, password, &amount);
                transaction_account = find_account(src_account, password);
                if (transaction_account) {
                	pthread_mutex_lock(&transaction_account->ac_lock);
                    transaction_account->balance += amount;
                    transaction_account->transaction_tracter += amount;
                    pthread_mutex_unlock(&transaction_account->ac_lock);
                    pthread_mutex_lock(&transaction_mutex);
                    total_transactions ++;
                    pthread_mutex_unlock(&transaction_mutex);
                }
                break;

            case 'W':
                fscanf(file, "%s %s %lf", src_account, password, &amount);
                transaction_account = find_account(src_account, password);
                if (transaction_account) {
                	pthread_mutex_lock(&transaction_account->ac_lock);
                    transaction_account->balance -= amount; // Allow overdrawing for this part
                    transaction_account->transaction_tracter += amount;
                    pthread_mutex_unlock(&transaction_account->ac_lock);
                    pthread_mutex_lock(&transaction_mutex);
                    total_transactions ++;
                    pthread_mutex_unlock(&transaction_mutex);
                }
                break;

            case 'T':
                fscanf(file, "%s %s %s %lf", src_account, password, dest_account, &amount);
                source_account = find_account(src_account, password);
                destination_account = find_account_helper(dest_account, "", false);
                if (source_account && destination_account) {
                
                	if(strcmp(source_account->account_number, destination_account->account_number) < 0){
                		pthread_mutex_lock(&source_account->ac_lock);
                		pthread_mutex_lock(&destination_account->ac_lock);
                	}
                	else{
                		pthread_mutex_lock(&destination_account->ac_lock);
                		pthread_mutex_lock(&source_account->ac_lock);
                	}
                    source_account->balance -= amount;
                    source_account->transaction_tracter += amount;
                    pthread_mutex_unlock(&source_account->ac_lock);
                    
                    destination_account->balance += amount;
                    pthread_mutex_unlock(&destination_account->ac_lock);
                    pthread_mutex_lock(&transaction_mutex);
                    total_transactions ++;
                    pthread_mutex_unlock(&transaction_mutex);
                    
                    //destination_account->transaction_tracter += amount; // Track transaction
                }
                break;

            case 'C':
                fscanf(file, "%s %s", src_account, password);
                transaction_account = find_account(src_account, password);
                if (transaction_account) {
                	pthread_mutex_lock(&transaction_account->ac_lock);
                    printf("Current Savings Balance for %s: %.2f\n", transaction_account->account_number, transaction_account->balance);
                    pthread_mutex_unlock(&transaction_account->ac_lock);
                }
                break;

            default:
                printf("Invalid transaction type: %c\n", transaction_type);
                break;
        }
        
        pthread_mutex_lock(&transaction_mutex);
        if (total_transactions >= TRANSACTION_LIMIT && !bank_updating) {
        	printf("Updating bank\n");
        	bank_updating = true;
            pthread_cond_signal(&update_condition);
            total_transactions = 0;
        }
        pthread_mutex_unlock(&transaction_mutex);
    }
    
    fclose(file);
    pthread_exit(NULL);
    return NULL;
}

void* update_balance(void* arg){
	(void)arg;
	
	pthread_barrier_wait(&start_barrier);
	while (1) {
		printf("In update_balance function while loop\n");
        pthread_mutex_lock(&update_mutex);
        pthread_cond_wait(&update_condition, &update_mutex);
        
        if (transaction_count >= num_transactions) {
        	pthread_mutex_unlock(&update_mutex);
            break;
        }
        
        // Update balance for all accounts
        for (int i = 0; i < num_accounts; i++) {
            pthread_mutex_lock(&accounts[i].ac_lock);
            accounts[i].balance += accounts[i].transaction_tracter * accounts[i].reward_rate;
            accounts[i].transaction_tracter = 0;
            pthread_mutex_unlock(&accounts[i].ac_lock);
            
            char filename[22];
            sprintf(filename, "Output/account%d.txt", i);
            FILE *account_file = fopen(filename, "a");
            if (account_file) {
                fprintf(account_file, "Current Balance: %20.2f\n", accounts[i].balance);
                fclose(account_file);
            }
        }

        // Signal workers to continue processing
        bank_updating = false;
		pthread_mutex_unlock(&update_mutex);
		pthread_cond_broadcast(&worker_condition);
    }
    return NULL;
}

// Save final account balances to file
void save_balances_to_file(const char *filename) {
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

    num_accounts = load_accounts(file); 
    
    int ret1 = pthread_barrier_init(&start_barrier, NULL, num_accounts + 1);
    int ret2 = pthread_cond_init(&worker_condition, NULL);
    int ret3 = pthread_cond_init(&update_condition, NULL);
    int ret4 = pthread_mutex_init(&update_mutex, NULL);
    
    if(ret1 != 0 || ret2 != 0 || ret3 != 0 || ret4 != 0){
    	printf("stupid\n");
    }
    
    printf("Iniitalized all primitives\n");
    
    num_transactions = get_total_transaction_count(file);
    int transaction_slice = num_transactions / num_accounts;
    int remain = num_transactions % num_accounts;
    int line_buffer = num_accounts * 5 + 1; //line buffer for account info provided in input file before transactions start
    
    workers = (pthread_t *)malloc(sizeof(pthread_t) * num_accounts);
    transaction_info infos[num_accounts];
    for (int i = 0; i < num_accounts; ++i)
    {
		strcpy(infos[i].file, argv[1]);
		infos[i].start_index = line_buffer + i * transaction_slice;
		infos[i].end_index = line_buffer + (i + 1) * transaction_slice;
	
		if (i == num_accounts - 1) {
			infos[i].end_index += remain;
		}
    
        pthread_create(&workers[i], NULL, (void*) process_transaction, (void*)&infos[i]);
    }

    for (int j = 0; j < num_accounts; ++j){
        pthread_join(workers[j], NULL);
    }       

	pthread_create(&bank_thread, NULL, update_balance, NULL);
    pthread_join(bank_thread, NULL);

    save_balances_to_file("output.txt");
    
    for (int i = 0; i < num_accounts; i++) {
    	pthread_mutex_destroy(&accounts[i].ac_lock);
	}
	
	pthread_barrier_destroy(&start_barrier);
	pthread_cond_destroy(&update_condition);
	pthread_mutex_destroy(&update_mutex);

    fclose(file);
    free(workers);
    return 0;
}
