#include "gameplay.h"

user_node* create_new_user(char* username, int x, int y){
    user_node* node = malloc(sizeof(user_node));
    node->HP = 100;
    node->username = strdup(username);
    node->x = x;
    node->y = y;
    node->is_defending = 0;
    node->next = NULL;
    return node;
};

void add_user(user_node* head,user_node* node){
    user_node* current;
    current = head;
    if(head == NULL){
        head = node;
    } else {
        while(current->next!=NULL){
            current = current->next;
        }
        current->next = node;
    }
}

void print_user_list(user_node* head){
    user_node* current;
    current = NULL;
    if(head == NULL){
        printf("No user.\n");
        return;
    } 
    for(current = head; current ; current=current->next){
        printf("- %s  %d:%d\n", current->username,current->x, current->y);
    }
}


user_node* find_user(user_node* head, char* username){
    user_node* current;
    user_node* result;
    result = current = NULL;
    
    for(current = head; current ; current=current->next){
        if(strcmp(current->username,username)==0) result = current;
    }

    return result;
}