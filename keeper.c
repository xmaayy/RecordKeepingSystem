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
// Index of the last element.
int last_record = 0;
// Capacity of the array.
int records_length;
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

// Remove a record at the specified index from the database.
int remove_item(int index) {
    // Since we keep track of the index of the last record in the array,
    // and the order of elements really doesn't matter, we can just put the last
    // record into the hole left by removing this one.
    records[index] = records[last_record];
    records[last_record] = NULL;
    last_record--;
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
    insert_item(create_record(data.name, data.deptName, data.employeeNum, 
                                data.salary));
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
int delete_record(int ID) {
    for(int i=0; i<records_length; i++) {
        if (records[i]->employeeNum == ID) {
            // DELETE
        }
    }
    // TODO: should have this send an error message to the client.
    printf("No record with employee ID %d found", ID);
    return -1;
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

int main() {
    // Allocate memory for an array of records.
    allocate_memory(DEFAULTSIZE);
    // Create the message queues.
    keeper_queue = create_keeper_queue();
    admin_queue = create_admin_queue();
    //while(1) {
        puts("Waiting for message...");
        record_message msg;
        int status = msgrcv(admin_queue, (void*)&msg, sizeof(Record), DATA, 0);
        if (status > 0) {
            puts("Recieved message: ");
            print_record(&msg.record);
        } else {
            puts("Failed");
        }
    //}
}

