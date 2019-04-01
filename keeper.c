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

void intHandler(int dummy) {
    if(children[1] != 0){
       printf("Kill signal recieved, shutting down children\n");
        kill(SIGINT, children[0]);
        kill(SIGINT, children[1]);
        running = 0;
    } else {
        exit(EXIT_SUCCESS);
    }

}


// Allocate an array to store records in.
// If the array already exists, reallocate instead.
// Size represents the number of records the new array should be able to hold
void allocate_memory(int size) {
    // realloc() tries to reallocate an allocated block of memory.
    // If the supplied pointer is null, it will try to create a new block.
    // Thus, it can be used both to create the database initially, and to
    // resize it.
    records = (Record**) realloc(records, size * sizeof(Record*));
    if (records == NULL) {
        perror("Memory allocation failed.");
        exit(EXIT_FAILURE);
    }
    records_length = DEFAULTSIZE;
}

int array_length(Record* r) {
    return (int) sizeof(r)/sizeof(Record);
}

// Place a new record into the database.
int insert_item(Record* r) {
    if (last_record <= records_length) {
        records[last_record] = r;
        last_record++;
        return 1;
    } else {
        perror("Could not insert new record; array is full.");
        return -1;
    }
}

// Look for a message in the queue that contains a record. If there is one,
// fetch it and add the record it contains to the database.
int add_rec(Record rec) {
    // This is super inefficient and redundant but meh.
    // We need to get this data copied somewhere permanent, this is a simple
    // dumb solution.
    return add_record(&employee_records, rec);
}

// Fetch a record by its employee ID.
Record* fetch_by_ID(int ID) {
    for(int i=0; i<records_length; i++) {
        if (records[i]->employeeNum == ID) {
            return records[i];
        }
    }
    return NULL;
}

// Fetch a record by the employee's name.
// The name must be an exact match.
Record* fetch_by_name(char* name) {
    for(int i=0; i<records_length; i++) {
        if (records[i]->name == name) {
            return records[i];
        }
    }
    return NULL;
}

// Fetch all records containing employees in a given department.
// The department name must be an exact match.
// Returns an array of Record*s
Record** fetch_by_department(char* dept) {
    //Record* matches[records_length];
    // TODO: Need to clean up this array! MEMORY LEAK!
    Record** matches = malloc(records_length*sizeof(Record*));
    int matches_index = 0;
    for(int i=0; i<records_length; i++) {
        if (records[i]->deptName == dept) {
            // Add this record to the list.
            matches[matches_index] = records[i];
            matches_index++;
        }
    }
    return matches;
}

// Delete a record with a given employee number from the database.
int del_rec(Record rec) {
    int success = delete_record(&employee_records, rec.employeeNum);
    if(success == 1){
        // TODO: GOOD
    } else {
        printf("No record with employee ID %d found", rec.employeeNum);
        return -1;
    }
    return success;
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


void command_handler(Record rec, int cmd) {
    switch (cmd)
    {
        case 1:
            add_record(&employee_records, rec);
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

    // Allocate memory for an array of records.
    allocate_memory(DEFAULTSIZE);

    // We want to make sure the children die with the parent
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

