//
//  BAROBOControlViewController.m
//  BaroboLink
//
//  Created by Adam Ruggles on 9/10/13.
//  Copyright (c) 2013 Barobo, Inc,. All rights reserved.
//

#import "BAROBOControlViewController.h"
#import "BAROBOAppDelegate.h"
#import "BAROBORobot.h"


@implementation BAROBOControlViewController

- (id) init {
    if ((self = [super init])) {
        timer = [NSTimer scheduledTimerWithTimeInterval:1
                                                 target:self
                                               selector:@selector(readRobotData)
                                               userInfo:nil
                                                repeats:YES];
    }
    return self;
    
}

// Private

double normalizeDeg(double deg)
{
    while(deg >= 180) {
        deg -= 360;
    }
    while (deg < -180) {
        deg += 360;
    }
    return deg;
}

CGFloat radiansToDegrees(CGFloat radians)
{
    return radians * 180 / M_PI;
}
CGFloat degreesToRadians(CGFloat degrees)
{
    return degrees * M_PI / 180;
}
double roundValue(float value) {
    return round (value * 100) / 100.0;
}


- (void) readRobotData {
    BAROBOAppDelegate *app = (BAROBOAppDelegate*)[NSApp delegate];
    BAROBORobot *robot = [app selectedRobot];
    if (robot) {
        double ja1, ja2, ja3, ja4;
        Mobot_getJointAngles((mobot_t*)robot.robot, &ja1, &ja2, &ja3, &ja4);
        [self.lblJointControl1 setDoubleValue:roundValue(normalizeDeg(radiansToDegrees(ja1)))];
        [self.lblJointControl2 setDoubleValue:roundValue(normalizeDeg(radiansToDegrees(ja3)))];
        Mobot_getJointSpeeds((mobot_t*)robot.robot, &ja1, &ja2, &ja3, &ja4);
        [self.lblJointSpeed1 setDoubleValue:roundValue(radiansToDegrees(ja1))];
        [self.lblJointSpeed2 setDoubleValue:roundValue(radiansToDegrees(ja3))];
    }
}

// Public

- (IBAction)sliderChanged:(id)sender
{
    NSString *identifier = [sender identifier];
    BAROBOAppDelegate *app = (BAROBOAppDelegate*)[NSApp delegate];
    BAROBORobot *robot = [app selectedRobot];
    if (!robot) {
        return;
    }
    if ([identifier isEqualToString:@"sliderJoint1"]) {
        NSLog(@"Slider1 %i", [sender intValue]);
        double value = [[NSNumber numberWithInt:[sender intValue]] doubleValue];
        value = degreesToRadians(value);
        Mobot_setJointSpeed((mobot_t*)robot.robot, ROBOT_JOINT1, value);
    } else if ([identifier isEqualToString:@"sliderJoint2"]) {
        NSLog(@"Slider2 %i", [sender intValue]);
        double value = [[NSNumber numberWithInt:[sender intValue]] doubleValue];
        value = degreesToRadians(value);
        Mobot_setJointSpeed((mobot_t*)robot.robot, ROBOT_JOINT3, value);
    }
}

- (IBAction)reset:(id)sender
{
    BAROBOAppDelegate *app = (BAROBOAppDelegate*)[NSApp delegate];
    BAROBORobot *robot = [app selectedRobot];
    if (!robot) {
        return;
    }
    Mobot_resetToZeroNB((mobot_t*)robot.robot);
}

- (IBAction)moveForward:(id)sender
{
    NSString *identifier = [sender identifier];
    BAROBOAppDelegate *app = (BAROBOAppDelegate*)[NSApp delegate];
    BAROBORobot *robot = [app selectedRobot];
    if (!robot) {
        return;
    }
    if ([identifier isEqualToString:@"moveForward1"]) {
        NSLog(@"Move Forward 1");
        Mobot_moveJointContinuousNB((mobot_t*)robot.robot, ROBOT_JOINT1, ROBOT_FORWARD);
    } else if ([identifier isEqualToString:@"moveForward2"]) {
        NSLog(@"Move Forward 2");
        Mobot_moveJointContinuousNB((mobot_t*)robot.robot, ROBOT_JOINT3, ROBOT_FORWARD);
    }
}

- (IBAction)moveBackward:(id)sender
{
    NSString *identifier = [sender identifier];
    BAROBOAppDelegate *app = (BAROBOAppDelegate*)[NSApp delegate];
    BAROBORobot *robot = [app selectedRobot];
    if (!robot) {
        return;
    }
    if ([identifier isEqualToString:@"moveBackward1"]) {
        NSLog(@"Move Forward 1");
        Mobot_moveJointContinuousNB((mobot_t*)robot.robot, ROBOT_JOINT1, ROBOT_BACKWARD);
    } else if ([identifier isEqualToString:@"moveBackward2"]) {
        NSLog(@"Move Forward 2");
        Mobot_moveJointContinuousNB((mobot_t*)robot.robot, ROBOT_JOINT3, ROBOT_BACKWARD);
    }
}

- (IBAction)jointStop:(id)sender
{
    NSString *identifier = [sender identifier];
    BAROBOAppDelegate *app = (BAROBOAppDelegate*)[NSApp delegate];
    BAROBORobot *robot = [app selectedRobot];
    if (!robot) {
        return;
    }
    if ([identifier isEqualToString:@"stop1"]) {
        NSLog(@"Stop 1");
        Mobot_moveJointContinuousNB((mobot_t*)robot.robot, ROBOT_JOINT1, ROBOT_NEUTRAL);
    } else if ([identifier isEqualToString:@"stop2"]) {
        NSLog(@"Stop 2");
        Mobot_moveJointContinuousNB((mobot_t*)robot.robot, ROBOT_JOINT3, ROBOT_NEUTRAL);
    }
}

@end
