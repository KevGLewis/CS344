//
//  processArgs.c
//  Assignment-3
//
//  Created by Kevin Lewis on 2/20/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//

#include "processArgs.h"

int ProcessLine(char **args, int nArgs)
{
    if(strcmp(args[0], "exit") == 0)
    {
        ProcessExit();
        return 0;
    }
    
    return 1;
}

void ProcessExit()
{
    
}

