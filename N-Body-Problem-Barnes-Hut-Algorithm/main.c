//
//  main.c
//  N-Body-Problem-Barnes-Hut-Algorithm
//
//  Created by Murphy, Jude {BIS} on 2/1/16.
//  Copyright © 2016 Iona College. All rights reserved.
//

#include "Body.c"
#include "Node.c"
#include "Forces.c"
#include "BHTree.c"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <Time.h>

#define kNumberOfIterations 1000
#define kFileOutName "output.out"
#define kFileInName "asteroid.in"
#define kG	0.00000000006673
#define kDeltaTime .5
#define Theta 0.1

//MUST SET STACKMAX TO BE GREAT THAN THE NUMBER OF BODIES IN WHICH YOU ARE TESTING
//SHOULD BE EQUAL TO NUMBER OF BODIES
#define STACK_MAX 2000

struct Stack
{
    struct Body * data[STACK_MAX];
    int size;
}; typedef struct Stack Stack;

void Stack_Push(Stack *S, struct Body * d);
struct Body * Stack_Top(Stack *S);
void Stack_Init(Stack *S);
void Stack_Pop(Stack *S);

struct Node * initNewNode();
struct BHTree * initNewBHTree();
void initNewBody(struct Body *body);
struct Node * reposition (struct Node *node);
struct Forces * getForce (struct Body *body, struct Node *node);
struct Stack * scanInFileAndSetUpBodies(int numberOfBodies, FILE *in_file, FILE *out_file);
struct Node * buildBHTree(double xmin, double xmax, double ymin, double ymax, struct Stack *bodyStack);

int main(int argc, const char * argv[])
{
    FILE *in_file  = fopen(kFileInName, "r");
    FILE *out_file = fopen(kFileOutName, "w");
    
    if (in_file == NULL || out_file == NULL)
    {
        char fileName [50];
        printf("Invalid File Name. Please Enter Another File Name: \n");
        scanf("%s", fileName);
        return 1;
    }
    
    //GET NUMBER OF BODIES FROM FILE || PRINT NUMBER OF BODIES || PRINT NUMBER OF ITERATIONS
    int numberOfBodies;
    fscanf(in_file, "%d\n", &numberOfBodies);
    fprintf(out_file, "%d\n", numberOfBodies);
    fprintf(out_file, "%d\n", kNumberOfIterations);
    
    //GET RADIUS OF UNIVERSE FROM FILE
    double radiusOfUniverse;
    fscanf(in_file, "%lf\n", &radiusOfUniverse);
    fprintf(out_file, "%lf\n", radiusOfUniverse);
    
    //READ IN THE NUMBER OF BODIES | WRITE OUT THE COLORS FROM EACH BODY
    struct BHTree * BHTree = malloc(sizeof(struct BHTree));
    BHTree->root = malloc(sizeof(struct Node));
    BHTree->root->bodyStack = scanInFileAndSetUpBodies(numberOfBodies, in_file, out_file);
    
    //CREATES THE UNIVERSE WIDTH AND HEIGHT PARAMETERS
    double xmin = (-1) * (radiusOfUniverse);
    double xmax = radiusOfUniverse;
    double ymin = (-1) * (radiusOfUniverse);
    double ymax = radiusOfUniverse;

    clock_t start = clock(), diff;
    for (int i = 0; i < kNumberOfIterations; i++)
    {
        BHTree->root = buildBHTree(xmin, xmax, ymin, ymax, BHTree->root->bodyStack);
        BHTree->root = reposition(BHTree->root);
        
        for (int n = 0; n < BHTree->root->bodyStack->size; n++)
        {
            fprintf(out_file, "%f %f\n", BHTree->root->bodyStack->data[n]->pos_x, BHTree->root->bodyStack->data[n]->pos_y);
        }
    }
    
    diff = clock() - start;
    int msec = (int)(diff * 1000 / CLOCKS_PER_SEC);
    printf("EXECUTION TIME: %d seconds and %d milliseconds\n", msec/1000, msec%1000);

    fclose(in_file);
    fclose(out_file);
}

struct Stack * scanInFileAndSetUpBodies(int numberOfBodies, FILE *in_file, FILE *out_file)
{
    struct Stack *newStack = malloc(sizeof(struct Stack));
    Stack_Init(newStack);
    
    //ALLOCATE MEMORY FOR ALL THE BODIES IN THE UNIVERSE
    for(int i = 0; i < numberOfBodies; i++)
    {
        struct Body *newBody = (struct Body *) malloc(sizeof(struct Body));
        initNewBody(newBody);
        
        fscanf(in_file, "%lf %lf %lf %lf %lf %d %d %d", &newBody->pos_x, &newBody->pos_y, &newBody->v_x, &newBody->v_y, &newBody->mass, &newBody->red, &newBody->green, &newBody->blue);
        fprintf(out_file, "%d %d %d\n", newBody->red, newBody->green, newBody->blue);
        Stack_Push(newStack, newBody);
    }
    
    return newStack;
}

void initNewBody(struct Body *body)
{
    body->pos_x = 0.0;
    body->pos_y = 0.0;
    body->v_x = 0.0;
    body->v_y = 0.0;
    body->mass = 0.0;
    body->force_x = 0.0;
    body->force_y = 0.0;
    body->red = 0;
    body->green = 0;
    body->blue = 0;
}

struct Node * buildBHTree(double xmin, double xmax, double ymin, double ymax, struct Stack *bodyStack)
{
    if (bodyStack->size == 0)
    {
        return NULL;
    }
    else if (bodyStack->size == 1)
    {
        struct Node *singleNode = initNewNode();
        singleNode->bodyStack = bodyStack;
        singleNode->xmin = xmin;
        singleNode->xmax = xmax;
        singleNode->ymin = ymin;
        singleNode->ymax = ymax;
        
        singleNode->mass = 0;
        singleNode->cMass[0] = 0;
        singleNode->cMass[1] = 0;
        
        for (int i = 0; i < bodyStack->size; i++)
        {
            singleNode->mass += singleNode->bodyStack->data[i]->mass;
            singleNode->cMass[0] += singleNode->bodyStack->data[i]->mass * singleNode->bodyStack->data[i]->pos_x;
            singleNode->cMass[1] += singleNode->bodyStack->data[i]->mass * singleNode->bodyStack->data[i]->pos_y;
        }
        
        singleNode->cMass[0] /= singleNode->mass;
        singleNode->cMass[1] /= singleNode->mass;
        
        return singleNode;
    }
    else
    {
        struct Stack *list1 = malloc(sizeof(struct Stack));
        list1->size = 0;
        
        struct Stack *list2 = malloc(sizeof(struct Stack));
        list2->size = 0;
        
        struct Stack *list3 = malloc(sizeof(struct Stack));
        list3->size = 0;
        
        struct Stack *list4 = malloc(sizeof(struct Stack));
        list4->size = 0;
        
        for (int i = 0; i < bodyStack->size; i++)
        {
            //QUADRANT 1
            if (bodyStack->data[i]->pos_x >= (xmin + xmax)/2 &&
                bodyStack->data[i]->pos_x <= xmax &&
                bodyStack->data[i]->pos_y >= (ymin + ymax)/2 &&
                bodyStack->data[i]->pos_y <= ymax)
            {
                Stack_Push(list1, bodyStack->data[i]);
            }
            //QUADRANT 2
            else if (bodyStack->data[i]->pos_x >= (xmin + xmax)/2 &&
                     bodyStack->data[i]->pos_x <= xmax &&
                     bodyStack->data[i]->pos_y >= ymin &&
                     bodyStack->data[i]->pos_y <= (ymin + ymax)/2)
            {
                Stack_Push(list2, bodyStack->data[i]);
            }
            //QUADRANT 3
            else if (bodyStack->data[i]->pos_x >= xmin &&
                     bodyStack->data[i]->pos_x <= (xmin + xmax)/2 &&
                     bodyStack->data[i]->pos_y >= ymin &&
                     bodyStack->data[i]->pos_y <= (ymin + ymax)/2)
            {
                Stack_Push(list3, bodyStack->data[i]);
            }
            //QUADRANT 4
            else if (bodyStack->data[i]->pos_x >= xmin &&
                     bodyStack->data[i]->pos_x <= (xmin + xmax)/2 &&
                     bodyStack->data[i]->pos_y >= (ymin + ymax)/2 &&
                     bodyStack->data[i]->pos_y <= ymax)
            {
                Stack_Push(list4, bodyStack->data[i]);
            }
        }
        
        struct Node *nodeTemp = (struct Node *) malloc(sizeof(struct Node));
        
        nodeTemp->node1 = buildBHTree((xmin + xmax)/2, xmax, (ymin + ymax)/2, ymax, list1);
        nodeTemp->node2 = buildBHTree((xmin + xmax)/2, xmax, ymin, (ymin + ymax)/2, list2);
        nodeTemp->node3 = buildBHTree(xmin, (xmin + xmax)/2, ymin, (ymin + ymax)/2, list3);
        nodeTemp->node4 = buildBHTree(xmin, (xmin + xmax)/2, (ymin + ymax)/2, ymax, list4);
        
        nodeTemp->bodyStack = bodyStack;
        nodeTemp->xmin = xmin;
        nodeTemp->xmax = xmax;
        nodeTemp->ymin = ymin;
        nodeTemp->ymax = ymax;
        nodeTemp->mass = 0;
        nodeTemp->cMass[0] = 0;
        nodeTemp->cMass[1] = 0;
        
        for (int i = 0; i < bodyStack->size; i++)
        {
            nodeTemp->mass += nodeTemp->bodyStack->data[i]->mass;
            nodeTemp->cMass[0] += nodeTemp->bodyStack->data[i]->mass * nodeTemp->bodyStack->data[i]->pos_x;
            nodeTemp->cMass[1] += nodeTemp->bodyStack->data[i]->mass * nodeTemp->bodyStack->data[i]->pos_y;
        }
        
        nodeTemp->cMass[0] /= nodeTemp->mass;
        nodeTemp->cMass[1] /= nodeTemp->mass;
        
        return nodeTemp;
    }
}

struct Node * initNewNode()
{
    struct Node *node = (struct Node *) malloc(sizeof(struct Node));
    node->bodyStack = (struct Stack *) malloc(sizeof(struct Stack));
    Stack_Init(node->bodyStack);
    node->bodyStack->size = 0;
    node->node1 = NULL;
    node->node2 = NULL;
    node->node3 = NULL;
    node->node4 = NULL;
    node->xmax = 0.0;
    node->xmin = 0.0;
    node->ymax = 0.0;
    node->ymin = 0.0;
    node->mass = 0;
    node->cMass[0] = 0;
    node->cMass[1] = 0;
    
    return node;
}

struct Node * reposition (struct Node *node)
{
    double xValues [node->bodyStack->size];
    double yValues [node->bodyStack->size];
    
    for (int i = 0; i < node->bodyStack->size; i++)
    {
        double mass = node->bodyStack->data[i]->mass;
        struct Forces *forces = getForce(node->bodyStack->data[i], node);
        double xVelocity = node->bodyStack->data[i]->v_x + (kDeltaTime * forces->x)/mass;
        double yVelocity = node->bodyStack->data[i]->v_y + (kDeltaTime * forces->y)/mass;
        
        xValues[i] = node->bodyStack->data[i]->pos_x + xVelocity * kDeltaTime;
        yValues[i] = node->bodyStack->data[i]->pos_y + yVelocity * kDeltaTime;
        
        node->bodyStack->data[i]->v_x = xVelocity;
        node->bodyStack->data[i]->v_y = yVelocity;
    }
    
    for (int j = 0; j < node->bodyStack->size; j++)
    {
        node->bodyStack->data[j]->pos_x  = xValues[j];
        node->bodyStack->data[j]->pos_y = yValues[j];
    }
    
    return node;
}

struct Forces * getForce (struct Body *body, struct Node *node)
{
    struct Forces *force = (struct Forces *) malloc(sizeof(struct Forces));
    double bodyMass = body->mass;
    double bodyPositionX = body->pos_x;
    double bodyPositionY = body->pos_y;
    double forceOfX = 0;
    double forceOfY = 0;
    
    if (node->bodyStack->size == 1)
    {
        double dist = sqrt(pow((body->pos_x - node->bodyStack->data[0]->pos_x), 2) + pow((bodyPositionY - node->bodyStack->data[0]->pos_y), 2));
        if (dist != 0)
        {
            forceOfX = kG * bodyMass * node->mass * (node->bodyStack->data[0]->pos_x - bodyPositionX) / pow(dist, 3);
            forceOfY = kG * bodyMass * node->mass * (node->bodyStack->data[0]->pos_y - bodyPositionY) / pow(dist, 3);
        }
    }
    else
    {
        double nodeMass = node->mass;
        double cMassX = node->cMass[0];
        double cMassY = node->cMass[1];
        double dist = sqrt(pow((cMassX - bodyPositionX), 2) + pow((cMassY - bodyPositionY), 2));
        double s = node->xmax - node->xmin;
        if(dist != 0 && (s/dist) < Theta)
        {
            forceOfX += kG * bodyMass * nodeMass * (cMassX - bodyPositionX) / pow(dist, 3);
            forceOfY += kG * bodyMass * nodeMass * (cMassY - bodyPositionY) / pow(dist, 3);
        }
        else
        {
            struct Forces *forceForNext1 = (struct Forces *) malloc(sizeof(struct Forces));
            struct Forces *forceForNext2 = (struct Forces *) malloc(sizeof(struct Forces));
            struct Forces *forceForNext3 = (struct Forces *) malloc(sizeof(struct Forces));
            struct Forces *forceForNext4 = (struct Forces *) malloc(sizeof(struct Forces));
            
            if (node->node1 != NULL)
            {
                forceForNext1 = getForce(body, node->node1);
            }
            if (node->node2 != NULL)
            {
                forceForNext2 = getForce(body, node->node2);
            }
            if (node->node3 != NULL)
            {
                forceForNext3 = getForce(body, node->node3);
            }
            if (node->node4 != NULL)
            {
                forceForNext4 = getForce(body, node->node4);
            }
            
            forceOfX = forceForNext1->x + forceForNext2->x + forceForNext3->x + forceForNext4->x;
            forceOfY = forceForNext1->y + forceForNext2->y + forceForNext3->y + forceForNext4->y;
        }
    }
    
    force->x = forceOfX;
    force->y = forceOfY;
    
    return force;
}

struct BHTree * initNewBHTree()
{
    struct BHTree *BHTree = (struct BHTree *) malloc(sizeof(struct BHTree));
    BHTree->root = NULL;
    return BHTree;
}

//--------------------------------------------------------------------------------------------------

void Stack_Init(Stack *S)
{
    S->size = 0;
}

struct Body * Stack_Top(Stack *S)
{
    if (S->size == 0)
    {
        fprintf(stderr, "Error: stack empty\n");
        return NULL;
    }
    
    return S->data[S->size-1];
}

void Stack_Push(Stack *S, struct Body * d)
{
    if (S->size < STACK_MAX)
        S->data[S->size++] = d;
    else
        fprintf(stderr, "Error: stack full\n");
}

void Stack_Pop(Stack *S)
{
    if (S->size == 0)
        fprintf(stderr, "Error: stack empty\n");
    else
        S->size--;
}