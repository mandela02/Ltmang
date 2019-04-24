#include "login.h"


// read login info from a txt file and return the head of linked list containing the info
login_node * read_login_info(char *filename){
    FILE *login_info_file;
    
    char temp[80];
    char* ch;
    char* username;
    char* password;

    login_node* head; 
    login_node* current;

    head = current = NULL;
    login_info_file = fopen("login_info.txt", "r");

    while ( fgets(temp, 80, login_info_file) != NULL ) {
        /* read from file */
        ch = strtok(temp, "-");
        username = ch;
        ch = strtok(NULL, "-");
        password = ch;
        
        login_node *node = malloc(sizeof(login_node));
        node->username = strdup(username);
        node->password = strdup(password);

        if(head == NULL){
            current = head = node;
        } else {
            current = current->next = node;
        }
    }
    fclose(login_info_file);

    return head;
};


// print the linked list to the screen
void print_login_info(login_node* head){
    login_node* current;
    current = NULL;

    for(current = head; current ; current=current->next){
        printf("- %s  %s\n", current->username,current->password);
    }
};

//check if the login is successful and return an int (1: success, 0: fail, -1: wrong password)
int check_login(login_node* head, char* username, char* password){
    login_node* current;
    current = NULL;
    
    for(current = head; current ; current=current->next){
        if(strcmp(username,current->username)==0){
            if(strcmp(password,current->password)==0) return 1;
            else return -1;
        };
    }

    return 0;
};

void free_login(login_node* head){
    login_node* tmp;

    while (head != NULL){
        tmp = head;
        head = head->next;
        free(tmp);
    }

}