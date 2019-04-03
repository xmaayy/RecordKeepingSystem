// Database, using a growing array.
// This server should be called before the client.

#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "record.h"
#include "messages.h"

#define DEFAULTSIZE 9

RecordStore employee_records;

// Array to store pointers to employee records in.
Record** records = NULL;
// Index of the last element.
int last_record = 0;
// Capacity of the array.
int records_length;
// Message queue identifiers, obtained from msgget() calls.
int keeper_queue, admin_queue;
// Child handlers
pid_t children[2];

static volatile int running = 1;

/**
 * This makes sure that when we CTRL-C to end the program, we
 * close/shutdown the message queues. This only happens on the server
 * because the client shouldnt be closing the queues
 */
void intHandler(int dummy) {
    msgctl(keeper_queue, IPC_RMID, NULL);
    msgctl(admin_queue, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}

// Create a message queue for messages going from the record keeper -> the
// administrator.
int create_keeper_queue() {
    int id = msgget(KEEPERQUEUE, IPC_CREAT | 0666);
    if (id < 0) {
        perror("Message queue creation failed.");
        exit(EXIT_FAILURE);
    }
    printf("Server queue created with ID %d\n", id);
    return id;
}

// Create a message queue for messages going from the administrator -> the
// record keeper.
int create_admin_queue() {
    int id = msgget(ADMINQUEUE, IPC_CREAT | 0666);
        if (id < 0) {
            perror("Message queue creation failed.");
            exit(EXIT_FAILURE);
        }
        printf("Admin queue created with ID %d\n", id);
        return id;
}

int send_num(int num){
    cmd_message cmd;
    cmd.message_type = LEN;
    cmd.command = num;

    puts("Pushing Num to Queue");
    int status = msgsnd(keeper_queue, (void*)&cmd, sizeof(cmd.command), 0);
    if (status < 0) {
        perror("Num failed to send.");
        return 0;
    } else {
        puts("Num sent successfully.");
        return 1;
    }
}

int send_rec(Record r){
    static record_message msg;
    msg.message_type = DATA;
    msg.record = r;

    int status = msgsnd(keeper_queue, (void*)&msg, sizeof(msg.record), 0);
    if (status < 0) {
        perror("Record failed to send.");
        return 0;
    } else {
        puts("Record sent successfully.");
        return 1;
    }
}


/**
 * This function works by first sending all the records matching the passed
 * department name, then sending the number of records it just pushed to the
 * message queue. The initial push of data no effect because the client is
 * listening for the LEN type message that comes at the end of the function.
 */
int check(Record rec){
    // If there are no records we dont need to send anything
    if(employee_records.firstRec == NULL) send_num(0);

    ServerRecord* pos_rec = employee_records.firstRec;
    int found = 0;
    while(pos_rec != NULL){
        if(strcmp(pos_rec->record.deptName, rec.deptName) == 0){
            printf("Found matching employee entry");
            send_rec(pos_rec->record);
            printf("Pushed employee to queue");
            found++;
        }
        pos_rec = pos_rec->nextRec;
    }
    send_num(found);
    return 1;
}


/**
 * This function works by first sending all the records matching the passed
 * department name, then sending the number of records it just pushed to the
 * message queue. The initial push of data no effect because the client is
 * listening for the LEN type message that comes at the end of the function.
 */
int ret_by_name(Record rec){
    // If there are no records we dont need to send anything
    if(employee_records.firstRec == NULL) send_num(0);

    ServerRecord* pos_rec = employee_records.firstRec;
    int found = 0;
    while(pos_rec != NULL){
        if(strcmp(pos_rec->record.name, rec.name) == 0){
            printf("Found matching employee entry");
            send_rec(pos_rec->record);
            printf("Pushed employee to queue");
            found++;
        }
        pos_rec = pos_rec->nextRec;
    }
    send_num(found);
    return 1;
}


/**
 * This function works by first sending all the records matching the passed
 * department name, then sending the number of records it just pushed to the
 * message queue. The initial push of data no effect because the client is
 * listening for the LEN type message that comes at the end of the function.
 */
int ret_by_number(Record rec) {
    // If there are no records we dont need to send anything
    if(employee_records.firstRec == NULL) send_num(0);

    ServerRecord* pos_rec = employee_records.firstRec;
    int found = 0;
    while(pos_rec != NULL){
        if(pos_rec->record.employeeNum == rec.employeeNum){
            printf("Found matching employee entry");
            send_rec(pos_rec->record);
            printf("Pushed employee to queue");
            found++;
        }
        pos_rec = pos_rec->nextRec;
    }
    send_num(found);
    return 1;
}


/**
 * A breakoff point for the rest of the functions in this file. It takes
 * in a command and a record object and runs the appropriate function.
 */
void command_handler(Record rec, int cmd) {
    switch (cmd)
    {
        case 1:
            add_record(&employee_records, rec);
            break;
        case 2:
        case 3:
        case 4:
            ret_by_number(rec);
            break;
        case 5:
            ret_by_name(rec);
            break;
        case 6:
            check(rec);
            break;
        case 7:
            delete_record(&employee_records, rec.employeeNum);
            break;
        default:
            printf("Nonexistent command");
            break;
    }
    print_store(employee_records);
}


int main() {
    // Init the employee record store
    employee_records.firstRec = NULL;
    employee_records.lastRec = NULL;
    employee_records.size = 0;

    // We want to make sure we close the queues
    signal(SIGINT, intHandler);

    // Create the message queues.
    keeper_queue = create_keeper_queue();
    admin_queue = create_admin_queue();

    int status = 0;
    record_message msg;
    cmd_message cmd;
    while(1) {
        // Check for a command being sent in the queue
        puts("Listening For Server Commands");
        status = msgrcv(admin_queue, (void*)&cmd, sizeof(cmd.command), CMD, 0);
        if (status > 0) {
            printf("Recieved command %d\n", cmd.command);
        } else {
            printf("Errored out\n");
            exit(EXIT_FAILURE);
        }

        // Each command needs a record asociated with it
        puts("Listening for Associated Record");

        // Check for a record being sent in the queue
        status = msgrcv(admin_queue, (void*)&msg, sizeof(msg.record), DATA, 0);
        if (status > 0) {
            printf("Recieved Record For Command %d\n", cmd.command);
        }

        command_handler(msg.record, cmd.command);
    }

}

