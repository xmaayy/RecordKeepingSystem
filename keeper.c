// Database, using a growing array.
// This server should be called before the client.

#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

#include "record.h"
#include "messages.h"

#define DEFAULTSIZE 9

// Array to store pointers to employee records in.
Record** records = NULL;
int record_index = 0;
int records_max;
// Message queue identifiers, obtained from msgget() calls.
int keeper_queue, admin_queue;


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
    records_max = DEFAULTSIZE;
}

int array_length(Record* r) {
    return (int) sizeof(r)/sizeof(Record);
}

// Look for a message in the queue that contains a record. If there is one,
// fetch it and add the record it contains to the database.
int add_record() {
    record_message msg;
    int len = msgrcv(admin_queue, &msg, sizeof(Record), DATA, 0);
    Record data = msg.record;
    // This is super inefficient and redundant but meh.
    // We need to get this data copied somewhere permanent, this is a simple
    // dumb solution.
    insert_record(create_record(data.name, data.deptName, data.employeeNum, 
                                data.salary));
}

// Place a new record into the database.
int insert_record(Record* r) {
    if (record_index <= records_max) {
        records[record_index] = r;
        record_index++;
        return 1;
    } else {
        // TODO: Have the array dynamically resize on demand.
        perror("Could not insert new record; array is full.");
        return -1;
    }
}

// Create a new record and return a pointer to it.
Record* create_record(char* name, char* dept, int id, int salary) {
    Record* r = malloc((size_t) sizeof(Record*));
    strcpy(r->name, name);
    strcpy(r->deptName, dept);
    r->employeeNum = id;
    r->salary = salary;
    return r;
}

// Fetch a record by its employee ID.
Record* fetch_by_ID(int ID) {
    for(int i=0; i<records_max; i++) {
        if (records[i]->employeeNum == ID) {
            return records[i];
        }
    }
    return NULL;
}

// Create a message queue for messages going from the record keeper -> the
// administrator.
int create_keeper_queue() {
    int id = msgget(KEEPERQUEUE, IPC_CREAT | 0666);
    if (id < 0) {
        perror("Message queue creation failed.");
        exit(EXIT_FAILURE);
    }
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
        return id;
}

int main() {
    // Allocate memory for an array of records.
    allocate_memory(DEFAULTSIZE);
    // Create the message queues.
    keeper_queue = create_keeper_queue();
    admin_queue = create_admin_queue();


    for (int i=0; i<=records_max; i++) {
        Record* foo = create_record("John", "Accting", i, 20000);
        insert_record(foo);
        print_record(records[i]);
    }
}

