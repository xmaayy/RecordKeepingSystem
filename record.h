#pragma once

#include <stdio.h>

typedef struct Record {
    char name[12];
    char deptName[12];
    int employeeNum;
    int salary;
} Record;

void print_record(Record* r) {
    printf("Name:\t\t%s\n"
            "Department:\t%s\n"
            "Employee ID:\t%d\n"
            "Salary ($):\t%d\n",
            r->name, r->deptName, r->employeeNum, r->salary);
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