//
//  BAROBOControlViewController.h
//  BaroboLink
//
//  Created by Adam Ruggles on 9/10/13.
//  Copyright (c) 2013 Barobo, Inc,. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BAROBOControlViewController : NSObject {
    NSTimer *timer;
}

@property IBOutlet NSTextField *lblJointControl1;
@property IBOutlet NSTextField *lblJointControl2;
@property IBOutlet NSTextField *lblJointSpeed1;
@property IBOutlet NSTextField *lblJointSpeed2;

@property IBOutlet NSSlider *sliderJointSpeed1;
@property IBOutlet NSSlider *sliderJointSpeed2;

@property IBOutlet NSButton *btnJointForward1;
@property IBOutlet NSButton *btnJointForward2;
@property IBOutlet NSButton *btnJointStop1;
@property IBOutlet NSButton *btnJointStop2;
@property IBOutlet NSButton *btnJointBackward1;
@property IBOutlet NSButton *bthJointBackward2;

@property IBOutlet NSButton *btnForward;
@property IBOutlet NSButton *btnBackward;
@property IBOutlet NSButton *btnLeft;
@property IBOutlet NSButton *btnRight;
@property IBOutlet NSButton *btnStop;
@property IBOutlet NSButton *btnReset;

- (IBAction)sliderChanged:(id)sender;
- (IBAction)reset:(id)sender;
- (IBAction)moveForward:(id)sender;
- (IBAction)moveBackward:(id)sender;
- (IBAction)jointStop:(id)sender;

@end
