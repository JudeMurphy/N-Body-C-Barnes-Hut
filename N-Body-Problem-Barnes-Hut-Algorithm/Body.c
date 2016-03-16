//
//  Body.c
//  N-Body-Problem-Barnes-Hut-Algorithm
//
//  Created by Murphy, Jude {BIS} on 2/1/16.
//  Copyright © 2016 Iona College. All rights reserved.
//

#include "Body.h"

struct Body
{
    double pos_x;
    double pos_y;
    double v_x;
    double v_y;
    double mass;
    double force_x;
    double force_y;
    int red;
    int green;
    int blue;
    
    struct Body *next;
    struct Body *last;
};