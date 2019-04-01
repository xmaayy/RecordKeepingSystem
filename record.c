#include "record.h"

#include <stdio.h>
#include <stdlib.h>



void print_record(Record* r) {
    printf("Name:\t\t%s\n"
            "Department:\t%s\n"
            "Employee ID:\t%d\n"
            "Salary ($):\t%d\n",
            r->name, r->deptName, r->employeeNum, r->salary);
}

Record create_record(char* name, char* dept, int id, int salary) {
    Record r;
    strcpy(r.name, name);
    strcpy(r.deptName, dept);
    r.employeeNum = id;
    r.salary = salary;
    return r;
}

void print_store(RecordStore recs){
    printf("Printing Employee Records\n");
    ServerRecord* rec = recs.firstRec;
    while(rec != NULL){
        printf("------------------------------------------\n");
        print_record(&(rec->record));
        rec = rec->nextRec;
    }
    printf("------------------------------------------\n");
    return;
}


int add_record(RecordStore* emp_rec, Record new_rec){
    ServerRecord* rec_alloc = malloc(sizeof(ServerRecord));
    rec_alloc->prevRec = NULL;
    rec_alloc->record = new_rec;
    rec_alloc->nextRec = NULL;

    // List is empty
    if(emp_rec->firstRec == NULL){
        emp_rec->firstRec = rec_alloc;
        emp_rec->lastRec = rec_alloc;
    } else { // Not empty
        rec_alloc->prevRec = emp_rec->lastRec;
        emp_rec->lastRec->nextRec = rec_alloc;
        emp_rec->lastRec = rec_alloc;
    }
    emp_rec->size += 1;
    return 1;
}

int delete_record(RecordStore* emp_rec, int emp_id){
    ServerRecord* rec_loc = emp_rec->firstRec;
    while(rec_loc != NULL){ // Look for the employee with a specific number
        if(rec_loc->record.employeeNum == emp_id){
            // Update the node that maps to this one to make it jump over
            if(rec_loc->prevRec != NULL) rec_loc->prevRec->nextRec = rec_loc->nextRec;
            rec_loc->record.employeeNum = -1; // Mark this one as invalid
            break;
        }
        rec_loc = rec_loc->nextRec;
    }

    if (rec_loc == NULL) return 0; // Employee not found

    if(emp_rec->lastRec->record.employeeNum == -1){
        // If we have an invalid employee and the list is longer than
        // 1 element, shift the end pointer back one to get a valid one
        // otherwise if its pointing to an invalid employee and the list is
        // only one it will nullify the end pointer because lastRec->prev is NULL
        emp_rec->lastRec = emp_rec->lastRec->prevRec;
    }
    if(emp_rec->firstRec->record.employeeNum == -1){
        // Same logic as above
        emp_rec->firstRec = emp_rec->firstRec->nextRec;
    }

    // We've shifted all the other pointers around this record so that
    // we can now free it and shrink the linked list
    if(rec_loc->record.employeeNum == -1){
        free(rec_loc);
    }
    emp_rec->size -= 1;

    return 1;
}

int free_store(RecordStore* rec_store){
    // If theres nothing there just leave it
    while(rec_store->firstRec != NULL){
        delete_record(rec_store, rec_store->firstRec->record.employeeNum);
    }
    free(rec_store);
    return 1;
}