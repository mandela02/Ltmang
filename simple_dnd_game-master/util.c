#include "util.h"

char *trimwhitespace(char *str){
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

// get input from stdin and remove trailing linebreak, if string is "\n" then convert it to ""
void get_input(char* command){
  fgets(command,128,stdin);
  if(strcmp(command,"\n")==0) strcpy(command,"");
  command = strtok(command, "\n");
}

//get a string, return 1 if string contain whitespace, 2 if string is empty or null
int contain_white_space_or_empty(char* s){
  
  if(s[0]=='\0'||s==NULL) return 2;

  while (*s != '\0') {
    if(isspace((unsigned char)*s)) return 1;
    s++;
  }
  return 0;
}