#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "account.h"

#define MAX_ACCOUNTS 100
#define MAX_PASSWORD_LENGTH 9
#define MAX_ID_LENGTH 17

pthread_t *workers;
pthread_t bank_thread;

account accounts[MAX_ACCOUNTS];
int num_accounts;

typedef struct {
    char file[256];
    int start_index;
    int end_index;
} transaction_info;

int load_accounts(FILE *file) {
    int account_num;
    fscanf(file, "%d", &account_num);

    for (int i = 0; i < account_num; i++) {
        char buffer[20];
        int index_number;
        fscanf(file, "%s %d", buffer, &index_number);
        fscanf(file, "%s", accounts[i].account_number);
        fscanf(file, "%s", accounts[i].password);
        fscanf(file, "%lf", &accounts[i].balance);
        fscanf(file, "%lf", &accounts[i].reward_rate);
        accounts[i].transaction_tracter = 0.0;
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
        line_counter++;
    }
	
	
	for(int i = start_index; i < end_index; i++) {
		fscanf(file, " %c", &transaction_type);
        switch (transaction_type) {
            case 'D':
                fscanf(file, "%s %s %lf", src_account, password, &amount);
                transaction_account = find_account(src_account, password);
                if (transaction_account) {
                	pthread_mutex_lock(&transaction_account->ac_lock);
                    transaction_account->balance += amount;
                    transaction_account->transaction_tracter += amount;
                    pthread_mutex_unlock(&transaction_account->ac_lock);
                }
                break;

            case 'W':
                fscanf(file, "%s %s %lf", src_account, password, &amount);
                transaction_account = find_account(src_account, password);
                if (transaction_account) {
                	pthread_mutex_lock(&transaction_account->ac_lock);
                    transaction_account->balance -= amount;
                    transaction_account->transaction_tracter += amount;
                    pthread_mutex_unlock(&transaction_account->ac_lock);
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
                }
                break;

            case 'C':
                fscanf(file, "%s %s", src_account, password);
                transaction_account = find_account(src_account, password);
                if (transaction_account) {
                	pthread_mutex_lock(&transaction_account->ac_lock);
                    printf("Current Balance for %s: %.2f\n", transaction_account->account_number, transaction_account->balance);
                    pthread_mutex_unlock(&transaction_account->ac_lock);
                }
                break;

            default:
                printf("Invalid transaction type: %c\n", transaction_type);
                break;
        }
    }
    
    fclose(file);
    pthread_exit(NULL);
}

void* update_balance(void* arg){
	(void)arg;
	int* update_count = malloc(sizeof(int));
	for(int i = 0; i < num_accounts; i++){
		pthread_mutex_lock(&accounts[i].ac_lock);
		accounts[i].balance += accounts[i].transaction_tracter * accounts[i].reward_rate;
		pthread_mutex_unlock(&accounts[i].ac_lock);
	}
	update_count += 1;
	pthread_exit(update_count);
}

// Save final account balances to file
void save_balances_to_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    for (int i = 0; i < num_accounts; i++) {
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
    
    int num_transactions = get_total_transaction_count(file);
    int transaction_slice = num_transactions / num_accounts;
    int remain = num_transactions % num_accounts;
    int line_buffer = num_accounts * 5 + 1;
    
    workers = (pthread_t *)malloc(sizeof(pthread_t) * num_accounts);
    transaction_info **infos = malloc(num_accounts * sizeof(transaction_info *));
    for (int i = 0; i < num_accounts; ++i)
    {
    	infos[i] = malloc(sizeof(transaction_info));
		strcpy(infos[i]->file, argv[1]);
		infos[i]->start_index = line_buffer + i * transaction_slice;
		infos[i]->end_index = line_buffer + (i + 1) * transaction_slice;
	
		if (i == num_accounts - 1) {
			infos[i]->end_index += remain;
		}
    
        pthread_create(&workers[i], NULL, (void*) process_transaction, (void*)(infos[i]));
    }

    for (int j = 0; j < num_accounts; ++j){
        pthread_join(workers[j], NULL);
        free(infos[j]);
    }
    
    int* bank_result;

	pthread_create(&bank_thread, NULL, update_balance, NULL);
    pthread_join(bank_thread, (void**)&bank_result);
    printf("Updated accounts %d times\n", bank_result);
    free(bank_result);

    save_balances_to_file("output.txt");
    
    for (int i = 0; i < num_accounts; i++) {
    	pthread_mutex_destroy(&accounts[i].ac_lock);
	}

    fclose(file);
    free(workers);
    free(infos);
    return 0;
}
