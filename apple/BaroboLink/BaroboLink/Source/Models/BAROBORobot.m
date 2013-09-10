//
//  BAROBORobot.m
//  BaroboLink
//
//  Created by Adam Ruggles on 9/7/13.
//  Copyright (c) 2013 Barobo, Inc,. All rights reserved.
//

#import "BAROBORobot.h"

@implementation BAROBORobot

- (id)init
{
    if ((self = [super init])) {
        self.robot = (recordMobot_t*)malloc(sizeof(recordMobot_t));
        RecordMobot_init(self.robot, "DONGLE");
    }
    return self;
}

- (void)dealloc {
    if (((mobot_t*)self.robot)->connected) {
        Mobot_disconnect((mobot_t*)self.robot);
    }
    free(self.robot);
}

- (BOOL)connected {
    if (((mobot_t*)self.robot)->connected) {
        return YES;
    }
    return NO;
}

@end
