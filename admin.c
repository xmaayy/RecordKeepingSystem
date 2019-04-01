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

Record prompt_for_record(int cmd) {
    // These can all be left undefined for any command
    // in which they are not read because... well... we
    // dont read them
    char name[12];
    char dept[12];
    char idString[12];
    char salaryString[12];
    int id;
    int salary;

    // Get name for commands 5 or 1
    if(cmd == 5 || cmd == 1){
        puts("Enter a name: ");
        fgets(name, 12, stdin);
        remove_trailing_newline(name);
    }

    // Department matters for command 1 and 6
    if(cmd == 6 || cmd == 1){
        puts("Enter a department");
        fgets(dept, 12, stdin);
        remove_trailing_newline(dept);
    }

    // ID matters for all cases except 5 and 6
    if(cmd != 5 && cmd != 6){
        puts("Enter an ID");
        fgets(idString, 12, stdin);
        id = strtol(idString, NULL, 10);
    }

    // Salary is only useful for command 1
        if(cmd == 1){
        puts("Enter a salary");
        fgets(salaryString, 12, stdin);
        salary = strtol(salaryString, NULL, 10);
    }

    return create_record(name, dept, id, salary);
}

void send_cmd(int command, Record r) {
    cmd_message cmd;
    cmd.message_type = CMD;
    cmd.command = command;

    record_message msg;
    msg.message_type = DATA;
    msg.record = r;

    puts("Pushing Command to Queue");
    int status = msgsnd(admin_queue, (void*)&cmd, sizeof(cmd.command), 0);
    if (status < 0) {
        perror("Command failed to send.");
    } else {
        puts("Command sent successfully...");
    }

    puts("Attempting to send associated record...");
    status = msgsnd(admin_queue, (void*)&msg, sizeof(msg.record), 0);
    if (status < 0) {
        perror("Record failed to send.");
    } else {
        puts("Record sent successfully.");
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


RecordStore* get_data(){
    cmd_message len;
    int status = msgrcv(keeper_queue, (void*)&len, sizeof(len.command), LEN, 0);
    if (status > 0) {
        printf("There (are|is) %d matching record(s)\n", len.command);
    } else {
        printf("Errored out\n");
        exit(EXIT_FAILURE);
    }

    // Return early if we have nothing to process
    RecordStore* temp_store = malloc(sizeof(RecordStore));
    temp_store->size = 0;
    if(len.command == 0) return temp_store;

    record_message msg;
    int num_recs = len.command;

    // Otherwise iterate through until we've taken in all the data we can
    for(int i = 0; i<num_recs; i++){
        status = msgrcv(keeper_queue, (void*)&msg, sizeof(msg.record), DATA, 0);
        if (status < 1){
            printf("Errored out\n");
            exit(EXIT_FAILURE);
        }
        add_record(temp_store, msg.record);
    }
    return temp_store;
}

int main() {
    keeper_queue = get_keeper_queue();
    admin_queue = get_admin_queue();

    char cmdStr[3];
    int cmd;
    Record r;
    RecordStore* ret_data;

    while(1){
        printf("-------------------------------------\n");
        printf("Insert | 1\nCheck Name | 2\n");
        printf("Check Department | 3\nCheck Salary | 4\n");
        printf("Check Emp Num | 5\nCheck | 6\n");
        printf("Delete | 7\n");
        printf("Please enter a command: ");
        fgets(cmdStr, 3, stdin);
        cmd = strtol(cmdStr, NULL, 10);
        printf("%d\n", cmd);
        switch (cmd)
        {
            case 1: // Insert
                printf("\nCommand %d | Insert New Record\n", cmd);
                r = prompt_for_record(cmd);
                send_cmd(cmd,r);
                break;
            case 6:// Check
                printf("\nCommand %d | Check\n", cmd);
                r = prompt_for_record(cmd);
                send_cmd(cmd,r);
                ret_data = get_data();
                print_store(*ret_data);
                free_store(ret_data);
                break;
            case 7: // Delete
                printf("\nCommand %d | Delete Record\n", cmd);
                r = prompt_for_record(cmd);
                send_cmd(cmd,r);
                break;
            default:
                break;
        }
    }

}
