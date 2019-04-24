#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct USER_NODE {
    int id;
    char *username;
    int HP;
    //coordinate
    int x;
    int y;
    //defend
    int is_defending;
    struct USER_NODE* next;
} user_node;


//User
user_node* create_new_user(char* username, int x, int y);
user_node* find_user(user_node* head, char* username);
void add_user(user_node* head,user_node* node);
void print_user_list(user_node* head);
void user_info(user_node* head, char* message);

//Gameplay
void decrease_hp(user_node* user, int amount);
void check_game_info();

#endif /* GAMEPLAY_H */