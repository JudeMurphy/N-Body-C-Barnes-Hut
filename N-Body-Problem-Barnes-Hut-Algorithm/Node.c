//
//  Node.c
//  N-Body-Problem-Barnes-Hut-Algorithm
//
//  Created by Murphy, Jude {BIS} on 2/1/16.
//  Copyright © 2016 Iona College. All rights reserved.
//

#include "Node.h"

struct Node
{
    struct Stack *bodyStack;
    int totalNumberOfBodies;
    
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double mass;
    double cMass [2];
    struct Node *node1;
    struct Node *node2;
    struct Node *node3;
    struct Node *node4;
    
    double totalMass;
    int amountOfBodies;
};