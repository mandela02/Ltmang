#ifndef LOGIN_H
#define LOGIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LOGIN_NODE {
    char* username;
    char* password;
    struct LOGIN_NODE* next;
} login_node;

// read login info from a txt file and return the head of linked list containing the info
login_node* read_login_info(char* filename);

// print the linked list to the screen
void print_login_info(login_node* head);

//check if the login is successful and return an int (1: success, 0: fail, -1: wrong password)
int check_login(login_node* head, char* username, char* password);
void free_login(login_node* head);

#endif /* LOGIN_H */