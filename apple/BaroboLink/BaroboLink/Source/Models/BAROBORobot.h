//
//  BAROBORobot.h
//  BaroboLink
//
//  Created by Adam Ruggles on 9/7/13.
//  Copyright (c) 2013 Barobo, Inc,. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RecordMobot.h"

typedef enum {
    LOCATION_USB,
    LOCATION_ZIGBEE,
    LOCATION_BLUETOOTH
} BAROBOLocationType;

@interface BAROBORobot : NSObject

@property (nonatomic) NSString *name;
@property (nonatomic) NSString *location;
@property recordMobot_t *robot;
@property BAROBOLocationType locationType;

- (BOOL)connected;

@end
