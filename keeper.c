// Database, using a growing array.

#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>

#include "record.h"
#include "messages.h"

#define DEFAULTSIZE 10


Record* records = NULL;


// Allocate an array to store records in.
// If the array already exists, reallocate instead.
// Size represents the number of records the new array should be able to hold
void allocate_memory(int size) {
    // realloc() tries to reallocate an allocated block of memory.
    // If the supplied pointer is null, it will try to create a new block.
    // Thus, it can be used both to create the database initially, and to 
    // resize it.
    records = (Record*) realloc(records, size * sizeof(Record));
    if (records == NULL) {
        perror("Memory allocation failed.");
        exit(EXIT_FAILURE);
    }
}

int add_record() {

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

int main() {
    allocate_memory(DEFAULTSIZE);
    int keeperQueue = createKeeperQueue();
}

