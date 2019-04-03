#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Record {
    char name[12];
    char deptName[12];
    int employeeNum;
    int salary;
} Record;

/**
 * A node in the linked list of records. Not ideal for a read heavy task like
 * this but by far the easiest. This is a doubly linked list, but it is not circular
 */
typedef struct ServerRecord{
    struct ServerRecord* prevRec;
    Record record;
    struct ServerRecord* nextRec;
} ServerRecord;

/**
 * The actual linked list implementation 
 */
typedef struct RecordStore{
    ServerRecord* firstRec;
    ServerRecord* lastRec;
    int size;
} RecordStore;


void print_record(Record* r);

// Create a new record and return a pointer to it.
Record create_record(char* name, char* dept, int id, int salary);

void print_store(RecordStore recs);
void print_element(RecordStore recs, int cmd);

int add_record(RecordStore* emp_rec, Record new_rec);

int delete_record(RecordStore* emp_rec, int emp_id);

int free_store(RecordStore* rec_store);

//Keeping this for now
/*

    if(emp_rec->lastRec->record.employeeNum == -1){
        if(emp_rec->lastRec->prevRec != NULL){
            // If we have an invalid employee and the list is longer than
            // 1 element, shift the end pointer back one
            emp_rec->lastRec = emp_rec->lastRec->prevRec;
        } else {
            // Else pointing to invalid employee and the list is only one,
            // nullify the end pointer
            emp_rec->lastRec = NULL;
        }
    }

    if(emp_rec->firstRec->record.employeeNum == -1){
        if(emp_rec->firstRec->nextRec != NULL){
            // If we have an invalid employee and the list is longer than
            // 1 element, shift the end pointer back one
            emp_rec->firstRec = emp_rec->firstRec->prevRec;
        } else {
            // Else pointing to invalid employee and the list is only one,
            // nullify the end pointer
            emp_rec->lastRec = NULL;
        }
    }
    */