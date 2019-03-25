#define KEEPERQUEUE 4444
#define ADMINQUEUE 5555

#include "record.h"

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