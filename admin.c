// Interactive client.

#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

#include "record.h"
#include "messages.h"

int keeper_queue, admin_queue;

void remove_trailing_newline(char* str) {
    int n_index = strcspn(str, "\n");
    str[n_index] = '\0';
}

Record* prompt_for_record() {
    char name[12];
    char dept[12];
    char idString[12];
    char salaryString[12];
    int id;
    int salary;

    puts("Enter a name: ");
    fgets(name, 12, stdin);
    remove_trailing_newline(name);

    puts("Enter a department");
    fgets(dept, 12, stdin);
    remove_trailing_newline(dept);

    puts("Enter an ID");
    fgets(idString, 12, stdin);
    puts("Enter a salary");
    fgets(salaryString, 12, stdin);
    
    id = strtol(idString, NULL, 10);
    salary = strtol(salaryString, NULL, 10);

    return create_record(name, dept, id, salary);
}

void send_record(Record r) {
    record_message msg;
    msg.message_type = DATA;
    msg.record = r;
    puts("Attempting to send record: ");
    print_record(&r);
    int status = msgsnd(admin_queue, (void*)&msg, sizeof(Record), 0);
    if (status < 0) {
        perror("Message failed to send.");
    } else {
        puts("Message sent successfully.");
    }
}

// Get the message queue for messages going from the administrator -> the
// record keeper.
int get_admin_queue() {
    int id = msgget(ADMINQUEUE, IPC_CREAT | 0666);
        if (id < 0) {
            perror("Message queue creation failed.");
            exit(EXIT_FAILURE);
        }
        printf("Client queue obtained with ID %d\n", id);
        return id;
}

// Get a message queue for messages going from the record keeper -> the
// administrator.
int get_keeper_queue() {
    int id = msgget(KEEPERQUEUE, IPC_CREAT | 0666);
    if (id < 0) {
        perror("Message queue creation failed.");
        exit(EXIT_FAILURE);
    }
    printf("Server queue obtained with ID %d\n", id);
    return id;
}

int main() {
    keeper_queue = get_keeper_queue();
    admin_queue = get_admin_queue();

    Record* r = prompt_for_record();
    //print_record(r);

    Record conc = *r;
    send_record(conc);
}
