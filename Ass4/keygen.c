//
//  keygen.c
//  DevEnviro
//
//  Created by Kevin Lewis on 3/5/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main ( int argc, char **argv )
{
    // First let's set up our random number
    srand((int) time(NULL));
    int keyLgth = atoi(argv[1]);
    int r;
    char charOut;
    for(int i = 0; i < keyLgth; i++)
    {
        r = rand() % 28;
        if(r == 27)
        {
            charOut = ' ';
        }
        else
        {
            charOut = 65 + r;
        }
        printf("%c", charOut);
    }
    printf("\n");
    
    return 0; // Indicates that everything went well.
}
