#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>   // For pipe, fork
#include <sys/types.h>
#include <signal.h>
#include "account.h"

#define MAX_ACCOUNTS 100
#define MAX_PASSWORD_LENGTH 9
#define MAX_ID_LENGTH 17

// Worker and bank threads
pthread_t *workers;
pthread_t bank_thread;

account accounts[MAX_ACCOUNTS];
int num_accounts;

// Pipe for communication
int pipe_fd[2];  // [0] for reading, [1] for writing

pthread_mutex_t pipe_lock = PTHREAD_MUTEX_INITIALIZER;
int check_balance_counter = 0;  // Counter for "check balance" commands

typedef struct {
    char file[256];
    int start_index;
    int end_index;
} transaction_info;

// Function to log data to the pipe
void log_to_pipe(const char *message) {
    pthread_mutex_lock(&pipe_lock);
    write(pipe_fd[1], message, strlen(message));
    pthread_mutex_unlock(&pipe_lock);
}

// Function for the Auditor process
void auditor_process() {
    close(pipe_fd[1]);  // Close write-end of the pipe in the Auditor process

    FILE *ledger = fopen("ledger.txt", "w");
    if (!ledger) {
        perror("Error opening ledger.txt");
        exit(1);
    }

    char buffer[256];
    ssize_t bytes_read;
    while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the string
        fprintf(ledger, "%s", buffer);
    }

    fclose(ledger);
    close(pipe_fd[0]);
    exit(0);  // Exit Auditor process
}

// Modified "process_transaction" function
void* process_transaction(void* arg) {
    transaction_info* info = (transaction_info*)arg;

    FILE *file = fopen(info->file, "r");
    if (!file) {
        perror("Error opening file");
        pthread_exit(NULL);
    }

    // Seek to the starting index
    char buffer[256];
    for (int i = 0; i < info->start_index; i++) {
        fgets(buffer, sizeof(buffer), file);
    }

    char transaction_type, src_account[MAX_ID_LENGTH], password[MAX_PASSWORD_LENGTH];
    double amount;
    account *transaction_account = NULL;

    for (int i = info->start_index; i < info->end_index; i++) {
        fscanf(file, " %c", &transaction_type);
        if (transaction_type == 'C') {
            fscanf(file, "%s %s", src_account, password);
            transaction_account = find_account(src_account, password);

            if (transaction_account) {
                pthread_mutex_lock(&transaction_account->ac_lock);
                double balance = transaction_account->balance;
                pthread_mutex_unlock(&transaction_account->ac_lock);

                // Log every 500th "check balance" command
                pthread_mutex_lock(&pipe_lock);
                check_balance_counter++;
                if (check_balance_counter % 500 == 0) {
                    char log_message[256];
                    snprintf(log_message, sizeof(log_message), "Account: %s, Balance: %.2f\n", src_account, balance);
                    log_to_pipe(log_message);
                }
                pthread_mutex_unlock(&pipe_lock);
            }
        }
        // Handle other transaction types (D, W, T)...
    }

    fclose(file);
    pthread_exit(NULL);
}

// Modified "update_balance" function
void* update_balance(void* arg) {
    (void)arg;
    for (int i = 0; i < num_accounts; i++) {
        pthread_mutex_lock(&accounts[i].ac_lock);
        accounts[i].balance += accounts[i].transaction_tracter * accounts[i].reward_rate;
        pthread_mutex_unlock(&accounts[i].ac_lock);

        // Log final balance
        char log_message[256];
        snprintf(log_message, sizeof(log_message), "Final Balance for Account %s: %.2f\n", accounts[i].account_number, accounts[i].balance);
        log_to_pipe(log_message);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening input file");
        return 1;
    }

    // Load accounts and set up threads
    num_accounts = load_accounts(file);

    // Create pipe
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child process: Auditor
        auditor_process();
    } else {
        // Parent process: Duck Bank
        close(pipe_fd[0]);  // Close read-end of the pipe in Duck Bank process

        // Spawn worker threads
        workers = malloc(sizeof(pthread_t) * num_accounts);
        transaction_info **infos = malloc(num_accounts * sizeof(transaction_info *));
        int transaction_slice = get_total_transaction_count(file) / num_accounts;
        int remain = get_total_transaction_count(file) % num_accounts;

        for (int i = 0; i < num_accounts; i++) {
            infos[i] = malloc(sizeof(transaction_info));
            strcpy(infos[i]->file, argv[1]);
            infos[i]->start_index = i * transaction_slice;
            infos[i]->end_index = (i + 1) * transaction_slice + (i == num_accounts - 1 ? remain : 0);

            pthread_create(&workers[i], NULL, process_transaction, infos[i]);
        }

        // Join threads
        for (int i = 0; i < num_accounts; i++) {
            pthread_join(workers[i], NULL);
            free(infos[i]);
        }

        pthread_create(&bank_thread, NULL, update_balance, NULL);
        pthread_join(bank_thread, NULL);

        close(pipe_fd[1]);  // Close write-end of the pipe
        wait(NULL);         // Wait for Auditor process to finish

        save_balances_to_file("output.txt");
    }

    fclose(file);
    return 0;
}
