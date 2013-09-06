//
//  BAROBOAppDelegate.h
//  BaroboLink
//
//  Created by Adam Ruggles on 9/4/13.
//  Copyright (c) 2013 Barobo, Inc,. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface BAROBOAppDelegate : NSObject <NSApplicationDelegate> {
    NSMutableArray *deviceArray;
    io_iterator_t gNewDeviceAddedIterator;
    io_iterator_t gNewDeviceRemovedIterator;
    IONotificationPortRef gNotifyPort;
	CFMutableDictionaryRef classToMatch;
}

@property (assign) IBOutlet NSWindow *window;

@end
