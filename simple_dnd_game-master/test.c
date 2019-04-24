#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "login.h"
#include "util.h"
#include "gameplay.h"

int main(void){
    srand (time(NULL));
    int i = rand()%20;
    printf("%d\n",i);
}