#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define DEBUG 1

char *trimwhitespace(char *str);

// get input from stdin and remove trailing linebreak, if string is "\n" then convert it to ""
void get_input(char* command);

//get a string, return 1 if string contain whitespace, 2 if string is empty or null
int contain_white_space_or_empty(char* string); 
#endif /* UTIL_H */