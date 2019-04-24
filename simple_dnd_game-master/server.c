#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<fcntl.h> // for open
#include<unistd.h> // for close
#include<pthread.h>
#include<time.h>

#include "util.h"
#include "login.h"
#include "gameplay.h"

#define MAX_THREAD 64
#define MESSAGE_SIZE 2048
#define MAP_WIDTH 2
#define MAP_HEIGHT 2

#define USER_DAMAGE 10
#define MONSTER_DAMAGE 3


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

login_node* login_database; // store login database taken from txt file
user_node* user_data;
user_node* active_user;

int user_counter;
int turn_counter;
int dice;

int monster_x, monster_y, monster_hp;

void * socketThread(void *arg){
    int newSocket = *((int *)arg);
    int break_flip = 1; // change to 0 if thread should be dead

    char client_message[MESSAGE_SIZE];
    char server_message[MESSAGE_SIZE];
    char buffer[64];

    user_node* current_user;
    user_node* temp;

    current_user=NULL;

    int login_flip;
    int user_count;
    int damage;

    char* username;
    char* password;
    char* client_message_type;
    char* direction;

    //Main loop
    while(break_flip){

        memset(server_message, 0, sizeof(server_message));
        memset(client_message, 0, sizeof(client_message));
        memset(buffer, 0, sizeof(buffer));
        if(active_user!= NULL&&current_user!=NULL) printf("Active user is: %s\n",active_user->username);
        if(recv(newSocket, client_message, MESSAGE_SIZE, 0) < 0){
            printf("Receive failed\n");
        }
        if(DEBUG) printf("Thead %d: \"%s\"\n", newSocket ,client_message);

        client_message_type = strtok(client_message, " "); //first call strok

        if(strcmp(client_message_type,"LOGIN")==0){
            username = strtok(NULL, " "); //second call strok
            password = strtok(NULL, " "); //third call strok
            login_flip = check_login(login_database,username,password);

            if(login_flip == 1) {
                //create new user
                if(user_data==NULL){ //if is first player
                    user_data = create_new_user(username,1,1);
                    current_user = active_user = user_data; //become the first active player
                }
                else{
                    current_user = create_new_user(username,1,1);
                    add_user(user_data, current_user);
                }
                if(DEBUG) print_user_list(user_data);

                printf("Player %s joined the game.\n", username);

                strcpy(server_message,"OK");
            }
            if(login_flip == 0) strcpy(server_message,"NOUSER");
            if(login_flip == -1) strcpy(server_message,"WRONGPASS");

            send(newSocket,server_message,strlen(server_message),0);
            continue;
        }

        //At least 1 player active
        
        //Command
        if(monster_hp==0){ 
            send(newSocket,"WIN",2048,0);
        }
        if(user_data->HP==0){ 
            send(newSocket,"LOSE",2048,0);
        }
        if(strcmp("EXIT",client_message)==0) {
            current_user->HP=0;
            break_flip=0;
            continue;
        }
        if(strcmp(client_message_type,"HELP")==0){
            strcpy(server_message,"COMMAND: \n");
            strcat(server_message,"- HELP : show command. \n");
            strcat(server_message,"- MOVE <DIRECTION> : move up down left right. \n");
            strcat(server_message,"- ATTACK: if same space with monster, user can attack. \n");
            strcat(server_message,"- DEFEND: decease damage taken this turn. \n");
            strcat(server_message,"- INFO: show infomation of player(s). \n");
            strcat(server_message,"- EXIT: exit the client. \n");
            send(newSocket,server_message,strlen(server_message),0);
            continue;
        }

        if(strcmp(client_message_type,"INFO")==0){
            strcpy(server_message,"INFO: \n");
            strcat(server_message,"- USER : \n");
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"\t + Active user is: %s\n",active_user->username);
            strcat(server_message,buffer);

            user_count=0;
            for(temp = user_data; temp ; temp=temp->next){
                user_count++;
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer,"\t + %s: %d/100 location: %d-%d \n", temp->username, temp->HP, temp->x, temp->y);
                strcat(server_message,buffer);
            }
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"Total: %d player(s) \n",user_count);
            strcat(server_message,buffer);
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"- Monster: %d/200 location: %d-%d \n",monster_hp,monster_x,monster_y);
            strcat(server_message,buffer);
            send(newSocket,server_message,strlen(server_message),0);
            continue;
        }

        //Turn bound actions:
        
        //Send message to the client socket 

        if(current_user == active_user){

            //Make a move
            if(current_user->HP==0){ 
                send(newSocket,"You are dead and can't act!\n",2048,0);
            }
            else
            if(strcmp(client_message_type,"MOVE")==0){
                direction = strtok(NULL, " "); //second call strok
                strcat(direction,"");
                if(strcmp(direction,"UP")==0){
                    if(current_user->y<MAP_HEIGHT) current_user->y++;
                }
                if(strcmp(direction,"DOWN")==0){
                    if(current_user->y>0) current_user->y--;
                }
                if(strcmp(direction,"LEFT")==0){
                    if(current_user->x>0) current_user->x--;
                }
                if(strcmp(direction,"RIGHT")==0){
                    if(current_user->x<MAP_WIDTH) current_user->x++;
                }
                strcpy(server_message,"");
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer,"Player %s MOVE %s to %d-%d",current_user->username,direction,current_user->x,current_user->y);
                strcat(server_message,buffer);
                strcat(server_message,"");
                printf("%s\n",server_message);
                send(newSocket,server_message,strlen(server_message),0);
            }
            else
            if(strcmp(client_message_type,"DEFEND")==0){
                current_user->is_defending = 1;
                strcpy(server_message,"");
                memset(buffer, 0, sizeof(buffer));
                printf("%s\n",buffer);
                sprintf(buffer,"Player is defending");
                strcat(server_message,buffer);
                send(newSocket,server_message,strlen(server_message),0);
            }
            else
            if(strcmp(client_message_type,"ATTACK")==0){
                if(current_user->x == monster_x &&current_user->y == monster_y){
                    damage = USER_DAMAGE*(rand()%12+1);
                    monster_hp -= damage;
                    if(monster_hp < 0) monster_hp =0;
                    strcpy(server_message,"");
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer,"Player %s attack monster for %d damage, remaining %d",current_user->username,damage,monster_hp);
                    strcat(server_message,buffer);
                    printf("%s\n",server_message);
                    send(newSocket,server_message,strlen(server_message),0);
                }
                else send(newSocket,"ATTACK MISSED",2048,0);
            }
            else{
                send(newSocket,"Invalid command, input HELP for detail",2048,0);
                continue;
            }
            
            
            if(active_user->next==NULL){ // if next user is null, revert back to first player and end player turn (calculate monster damage)
                active_user = user_data;
                temp = user_data;
                
                printf("End player turn.\n");
                //Monster attack
                damage = MONSTER_DAMAGE*(rand()%12+1);
                temp->HP -= damage;
                if(temp->HP<0) temp->HP=0;
                printf("Monster attack %s for %d damage, remaining %d\n",temp->username,damage,temp->HP);

            } 
            else{
                active_user=active_user->next;
            }

        }else send(newSocket,"It is not your turn yet!",128,0);
        

        //clean the message variable
    }

    printf("User exited. \n");
    close(newSocket);
    pthread_exit(NULL);
}

int main(){
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    //Create the socket. 
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    // Configure settings of the server address struct
    // Address family = Internet 
    serverAddr.sin_family = AF_INET;
    //Set port number, using htons function to use proper byte order 
    serverAddr.sin_port = htons(7799);
    //Set IP address to localhost 
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //Set all bits of the padding field to 0 
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    //Bind the address struct to the socket 
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    //Listen on the socket, with 64 max connection requests queued 
    if(listen(serverSocket,MAX_THREAD)==0)
        printf("Listening\n");
    else printf("Error\n");
    
    login_database = read_login_info("login_info.txt"); // get login info from txt file
    user_data = NULL;
    pthread_t tid[MAX_THREAD];
    
    monster_x = 2;
    monster_y = 2;
    monster_hp = 200;

    int i = 0;
    srand (time(NULL));

    while(1){
        //Accept call creates a new socket for the incoming connection
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
        //for each client request creates a thread and assign the client request to it to process
        //so the main thread can entertain next request
        if( pthread_create(&tid[i], NULL, socketThread, &newSocket) != 0 )
            printf("Failed to create thread\n");
        if( i >= MAX_THREAD){
            i = 0;
            while(i < MAX_THREAD){
                pthread_join(tid[i++],NULL);
            }
            i = 0;
        }
    }

    free_login(login_database);

  return 0;
}