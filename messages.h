#define KEEPERQUEUE 4445
#define ADMINQUEUE 5556

#include "record.h"

#define LEN  11 // Inform client of return data len
#define DATA 22 // Transfer of a single record
#define CMD  33 // Client command to server

// Message containing a record, to transfer data between processes.
typedef struct record_message {
    long message_type;
    Record record;
} record_message;

// Message containing a command, for the administrator to issue command to the
// record keeper.
typedef struct cmd_message {
    long message_type;
    int command;
} cmd_message;