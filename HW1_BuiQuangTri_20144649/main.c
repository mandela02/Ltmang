
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// max length of a string
#define MAX 256

enum
{
    user_register = 1,
    sign_in,
    search,
    sign_out
};

enum
{
    false,
    true
};

enum
{
    blocked,
    active
};

//input file
char c_fileName[256] = "account.txt";

typedef struct tagAccount
{
    char m_username[MAX];
    char m_password[MAX];
    int m_status;
    int m_isLogin;
    int m_numOfFalseLogin;
    struct tagAccount *next;
} account;

account *head = NULL;
account *current = NULL;

//print all linked list
void PrintList()
{
    account *ptr = head;
    printf("\n");
    while (ptr != NULL)
    {
        printf("Name: %-15s - Pass: %-10s - Status:%d - IsLogin:%d - NumOfFalseLogin:%d\n", ptr->m_username, ptr->m_password, ptr->m_status, ptr->m_isLogin, ptr->m_numOfFalseLogin);
        ptr = ptr->next;
    }
}

//insert a link to head of linked list
void InsertFirst(account acc)
{
    account *link = (account *)malloc(sizeof(account));
    strcpy(link->m_username, acc.m_username);
    strcpy(link->m_password, acc.m_password);
    link->m_status = acc.m_status;
    link->next = head;
    head = link;
}

//read data from file and add to linked list
//NOTE:: call this one time at the beginning of program
void ReadData()
{
    account acc;
    int i = 0;
    FILE *f;
    //r == read file text
    if (!(f = fopen(c_fileName, "r")))
    {
        printf("\n File not found!! \n\n");
    }
    else
    {
        while (!feof(f))
        {
            fscanf(f, "%s %s %d", acc.m_username, acc.m_password, &acc.m_status);
            acc.m_isLogin = 0;
            acc.m_numOfFalseLogin = 0;
            InsertFirst(acc);
        }
    }
    fclose(f);
}

//rewrite data after any change.
// Only use this if used file is really short
void ReWriteData()
{
    FILE *f;
    if (!(f = fopen(c_fileName, "w")))
    {
        printf("\n File not found!! \n\n");
    }
    else
    {
        account *ptr;
        ptr = head;
        while (ptr != NULL)
        {
            fprintf(f, "%s %s %d\n", ptr->m_username, ptr->m_password, ptr->m_status);
            ptr = ptr->next;
        }
    }
    fclose(f);
}

//check if usename is in list
int isExisting(char *input)
{
    int isExist = false;
    account *ptr;
    ptr = head;
    while (ptr != NULL)
    {
        if (strcmp(input, ptr->m_username) == 0)
            isExist = true;
        ptr = ptr->next;
    }
    return isExist;
}

// new user register
void UserRegister()
{
    account acc;
    printf("---Welcome, new user.---\n");
    do
    {
        printf("Please enter new username:");
        scanf("%s", acc.m_username);
        if (isExisting(acc.m_username) == true)
            printf("This name is already exist.\n");
    } while (isExisting(acc.m_username) == true);
    printf("please enter new password:");
    scanf("%s", acc.m_password);
    acc.m_status = active;
    acc.m_isLogin = false;
    acc.m_numOfFalseLogin = 0;
    FILE *f;
    // a == write to the end of file
    if (!(f = fopen(c_fileName, "a")))
    {
        printf("\n File not found!! \n\n");
    }
    else
    {
        fprintf(f, "\n%s %s %d", acc.m_username, acc.m_password, 1);
    }
    InsertFirst(acc);
    printf("Successful registration! Welcome \"%s\" join the team. \n\n", acc.m_username);
    fclose(f);
}

//sign in function
void UserSignIn()
{
    int isDone = false;
    account acc;
    printf("---Sign in---\n");
    do
    {
        printf("username: ");
        scanf("%s", acc.m_username);
        printf("Password:");
        scanf("%s", acc.m_password);
        account *ptr;
        ptr = head;
        int isExist = false;
        while (ptr != NULL)
        {
            //check if username is exist or not
            if (strcmp(acc.m_username, ptr->m_username) == 0)
            {
                //if username is found in list, 
                isExist = true;
                //check user status
                if (ptr->m_status == blocked)
                {
                    printf("Account \"%s\" is blocked.\nContract administrator for more information.\n\n", ptr->m_username);
                }
                else if (ptr->m_isLogin == true)
                {
                    printf("Account \"%s\" is allready login\n", ptr->m_username);
                }
                else
                {
                    //if this account is active, check password
                    if (strcmp(acc.m_password, ptr->m_password) != 0)
                    {
                        ptr->m_numOfFalseLogin++;
                        printf("password is incorrect.\n");
                        //after 3 time wrong pass, set status to "blocked"
                        if (ptr->m_numOfFalseLogin == 3)
                        {
                            ptr->m_status = blocked;
                            isDone = true;
                            ptr->m_numOfFalseLogin = 0;
                            ReWriteData();
                            printf("Password is incorrect 3 times. Account \"%s\" is blocked.\nContract administrator for more information.\n", ptr->m_username);
                        }
                    }
                    else
                    {
                        isDone = true;
                        ptr->m_isLogin = true;
                        ptr->m_numOfFalseLogin = 0;
                        printf("\nHello \"%s\".\n", ptr->m_username);
                    }
                }
            }
            ptr = ptr->next;
        }
        if (isExist == false)
        {
            printf("Account \"%s\" is not exist.\n", acc.m_username);
        }
    } while (isDone = false);
}

//search function
void UserSearch()
{
    printf("---Search information---\n");
    account *ptr;
    ptr = head;
    account acc;
    printf("Username: ");
    scanf("%s", acc.m_username);
    int isExist = false;
    while (ptr != NULL)
    {
        if (strcmp(acc.m_username, ptr->m_username) == 0)
        {
            isExist = true;
            if (ptr->m_status == active)
                printf("Account \"%s\" is active.\n", ptr->m_username);
            else
                printf("Account \"%s\" is blocked.\n", ptr->m_username);
        }
        ptr = ptr->next;
    }
    if (isExist == false)
        printf("Account \"%s\" is not exist.\n", ptr->m_username);
}

//sign out function
void UserSignOut()
{
    printf("---Sign out---\n");
    account *ptr;
    ptr = head;
    account acc;
    printf("Username: ");
    scanf("%s", acc.m_username);
    int isExist = false;
    while (ptr != NULL)
    {
        if (strcmp(acc.m_username, ptr->m_username) == 0)
        {
            isExist = true;
            if (ptr->m_isLogin == true)
            {
                ptr->m_isLogin = false;
                printf("Goodbye \"%s\".\n", ptr->m_username);
            }
            else
                printf("Account \"%s\" is not sign in.\n", ptr->m_username);
        }
        ptr = ptr->next;
    }
    if (isExist == false)
        printf("Account \"%s\" is not exist.\n", ptr->m_username);
}

int main(int argc, char const *argv[])
{
    ReadData();
    int choice;
    while (1)
    {
        printf("\n-----------------------\n");
        printf("USER MANAGER PROGRAM\n");
        printf("\n-----------------------\n");
        printf(" 1. Register\n 2. Sign in \n 3. Search \n 4. Sign out\n 5. For developer - account list\n");
        printf("Your choice (1 - 4, other to exit): ");
        scanf("%d", &choice);
        switch (choice)
        {
        case user_register:
            UserRegister();
            break;
        case sign_in:
            UserSignIn();
            break;
        case search:
            UserSearch();
            break;
        case sign_out:
            UserSignOut();
            break;
        case 5:
            PrintList();
            break;
        default:
            exit(1);
            break;
        }
    }
    return 0;
}
