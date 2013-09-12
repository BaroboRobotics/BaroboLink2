//
//  BAROBOAppDelegate.h
//  BaroboLink
//
//  Created by Adam Ruggles on 9/4/13.
//  Copyright (c) 2013 Barobo, Inc,. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BAROBORobotTableViewController.h"
#import "BAROBOControlViewController.h"
#import "BAROBORobot.h"

@interface BAROBOAppDelegate : NSObject <NSApplicationDelegate> {
    NSMutableArray *deviceArray;
    io_iterator_t gNewDeviceAddedIterator;
    io_iterator_t gNewDeviceRemovedIterator;
    IONotificationPortRef gNotifyPort;
	CFMutableDictionaryRef classToMatch;
    

}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet BAROBORobotTableViewController *tableViewController;
@property (assign) IBOutlet BAROBOControlViewController *controlViewController;

- (void)listenForRobots;
- (BAROBORobot*)selectedRobot;

@end
