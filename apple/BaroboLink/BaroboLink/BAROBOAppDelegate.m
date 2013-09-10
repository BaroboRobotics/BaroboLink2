//
//  BAROBOAppDelegate.m
//  BaroboLink
//
//  Created by Adam Ruggles on 9/4/13.
//  Copyright (c) 2013 Barobo, Inc,. All rights reserved.
//

#import "BAROBOAppDelegate.h"
#import "BAROBORobot.h"


#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOBSD.h>
#include <IOKit/serial/IOSerialKeys.h>

__BEGIN_DECLS
#include <mach/mach.h>
#include <IOKit/iokitmig.h>
__END_DECLS

#include "RecordMobot.h"

@implementation BAROBOAppDelegate

void usbDeviceAdded(void *refCon, io_iterator_t iterator)
{
    NSLog(@"USB Added");
    sleep(10);
    BAROBOAppDelegate *appDelegate = (__bridge BAROBOAppDelegate *)(refCon);
    if (appDelegate) {
        [appDelegate addRobot: iterator];
    }
}
void usbDeviceRemoved(void *refCon, io_iterator_t iterator)
{
    NSLog(@"USB Removed");
    BAROBOAppDelegate *appDelegate = (__bridge BAROBOAppDelegate *)(refCon);
    if (appDelegate) {
        [appDelegate removeRobot: iterator];
    }
}

- (void)addRobot:(io_iterator_t)iterator
{
    io_object_t usbDevice;
    
    while ((usbDevice = IOIteratorNext(iterator))) {
        NSLog(@"%s(): device added %d.\n", __func__, (int)usbDevice);
        io_name_t devName = {0};
        io_string_t pathName = {0};
        
        //char path[MAXPATHLEN] = {0};
        /*
         kern_return_t ret = IORegistryEntryGetPath(usbDevice, kIOServicePlane, path);
         if (ret != KERN_SUCCESS) {
         NSLog(@"Error returning the path (%08x)\n", kr);
         }
         NSLog(@"Path: %s", path);
         */
        IORegistryEntryGetName(usbDevice, devName);
        printf("Device's name = %s\n", devName);
        IORegistryEntryGetPath(usbDevice, kIOUSBPlane, pathName);
        printf("Device's path in IOUSB plane = %s\n", pathName);
        
        
        CFTypeRef typeData = (CFTypeRef)IORegistryEntrySearchCFProperty (usbDevice, kIOUSBPlane,
                                                                         CFSTR ( kIOCalloutDeviceKey ),
                                                                         kCFAllocatorDefault,
                                                                         kIORegistryIterateRecursively);
        
        
        
        
        NSString* bsdPath = (__bridge NSString*)typeData;
        
        
        
        //Create an intermediate plug-in
        /*
         kr = IOCreatePlugInInterfaceForService(usbDevice,
         kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID,
         &plugInInterface, &score);
         //Don’t need the device object after intermediate plug-in is created
         kr = IOObjectRelease(usbDevice);
         if ((kIOReturnSuccess != kr) || !plugInInterface)
         {
         NSLog(@"Unable to create a plug-in (%08x)\n", kr);
         continue;
         }
         
         //Now create the device interface
         result = (*plugInInterface)->QueryInterface(plugInInterface,
         CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
         (LPVOID *)&dev);
         //Don’t need the intermediate plug-in after device interface
         //is created
         (*plugInInterface)->Release(plugInInterface);
         
         if (result || !dev) {
         NSLog(@"Couldn’t create a device interface (%08x)\n", (int) result);
         continue;
         }
         */
        
        NSLog(@"BSD Path = %@", bsdPath);
        if (bsdPath) {
        //NSRange range = [bsdPath rangeOfString:@"bluetooth" options:NSCaseInsensitiveSearch];
        //if (range.location == NSNotFound) {
            // Attempt to connect to the mobot.
            BAROBORobot *robot = [[BAROBORobot alloc] init];
        
        
            Mobot_init((mobot_t*) robot.robot);
            if(!Mobot_connectWithTTY((mobot_t*)robot.robot, [bsdPath UTF8String])) {
                NSLog(@"Found robot at %@", bsdPath);
                NSString *name = [NSString stringWithCString:((mobot_t*)robot.robot)->serialID encoding:NSASCIIStringEncoding];
                
                [robot setName:name];
                [robot setLocation:bsdPath];
                [robot setLocationType:LOCATION_USB];
                [self.tvc addRobot:robot];
                [self.tvc refreshTable];
            } else {
                NSLog(@"Robot NOT found at %@", bsdPath);
            }
        }
        //}
		//NSLog(@"Vendor ID: %@", [NSString stringWithFormat: @"0x%04x", vendor]);
        //NSLog(@"Product ID: %@", [NSString stringWithFormat: @"0x%04x", product]);
        
        NSLog(@"Service: %@", [NSNumber numberWithInt: usbDevice]);
    }
}

- (void)removeRobot:(io_iterator_t)iterator
{
    kern_return_t   kr;
    io_service_t    usbDevice;
    
    while ((usbDevice = IOIteratorNext(iterator))) {
        NSLog(@"%s(): device removed %d.\n", __func__, (int) usbDevice);
        
        io_name_t devName = {0};
        io_string_t pathName = {0};
        
        IORegistryEntryGetName(usbDevice, devName);
        printf("Device's name = %s\n", devName);
        IORegistryEntryGetPath(usbDevice, kIOUSBPlane, pathName);
        printf("Device's path in IOUSB plane = %s\n", pathName);
        
        
        CFTypeRef typeData = (CFTypeRef)IORegistryEntrySearchCFProperty (usbDevice, kIOUSBPlane,
                                                                         CFSTR ( kIOCalloutDeviceKey ),
                                                                         kCFAllocatorDefault,
                                                                         kIORegistryIterateRecursively);
        
        
        
        
        NSString* bsdPath = (__bridge NSString*)typeData;
		NSLog(@"BSD Path = %@", bsdPath);
        if (bsdPath) {
            BAROBORobot *robotToRemove = nil;
            for (BAROBORobot *robot in self.tvc.dataSource) {
                if ([robot.location isEqualToString: bsdPath]) {
                    robotToRemove = robot;
                    break;
                }
            }
            if (robotToRemove) {
                [self.tvc.dataSource removeObject:robotToRemove];
                [self.tvc refreshTable];
            }
        }
        
        kr = IOObjectRelease(usbDevice);
        if (kr != kIOReturnSuccess) {
            printf("Couldn’t release raw device object: %08x\n", kr);
            continue;
        }
    }
}

- (void)listenForRobots
{
    OSStatus ret;
    CFRunLoopSourceRef runLoopSource;
    mach_port_t masterPort;
    kern_return_t kr;
    
    // Returns the mach port used to initiate communication with IOKit.
    //kr = IOMasterPort(MACH_PORT_NULL, &masterPort);
    IONotificationPortRef notificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       IONotificationPortGetRunLoopSource(notificationPort),
                       kCFRunLoopDefaultMode);
    /*
    if (kr != kIOReturnSuccess) {
        NSLog(@"%s(): IOMasterPort() couldn’t create a master I/O Kit port(%08x)", __func__, kr);
        return;
    }
     */
    
    //classToMatch = IOServiceMatching(kIOUSBDeviceClassName);
    classToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if (!classToMatch) {
        NSLog(@"%s(): IOServiceMatching couldn’t create a matching dictionary.\n", __func__);
        return;
    }
    
    // increase the reference count by 1 since die dict is used twice.
    CFRetain(classToMatch);
    
    //CFDictionaryAddValue(classToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDRS232Type));

    
    //gNotifyPort = IONotificationPortCreate(masterPort);
    //runLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
    //CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
    
    ret = IOServiceAddMatchingNotification(notificationPort,
                                           kIOFirstMatchNotification,
                                           classToMatch,
                                           usbDeviceAdded,
                                           (__bridge void *)(self),
                                           &gNewDeviceAddedIterator);
    
    // Iterate once to get already-present devices and arm the notification
    [self addRobot: gNewDeviceAddedIterator];
    
    while (IOIteratorNext(gNewDeviceAddedIterator)) {};

    
    ret = IOServiceAddMatchingNotification(notificationPort,
                                           kIOTerminatedNotification,
                                           classToMatch,
                                           usbDeviceRemoved,
                                           (__bridge void *)(self),
                                           &gNewDeviceRemovedIterator);
    
    // Iterate once to get already-present devices and arm the notification
    [self removeRobot: gNewDeviceRemovedIterator];
    
    while (IOIteratorNext(gNewDeviceRemovedIterator)) {};
    
    // done with the masterport
   // mach_port_deallocate(mach_task_self(), masterPort);
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //[self listenForRobots];
    //[NSThread detachNewThreadSelector:@selector(listenForRobots) toTarget:self withObject:nil];
    [self listenForRobots];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)application
{
    return YES;
}

@end
