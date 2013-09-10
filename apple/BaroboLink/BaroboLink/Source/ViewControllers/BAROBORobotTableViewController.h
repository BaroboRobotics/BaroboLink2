//
//  BAROBORobotTableViewController.h
//  BaroboLink
//
//  Created by Adam Ruggles on 9/7/13.
//  Copyright (c) 2013 Barobo, Inc,. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BAROBORobot.h"

@interface BAROBORobotTableViewController : NSObject <NSTableViewDataSource>

@property IBOutlet NSTableView *tableView;
@property (atomic) NSMutableArray *dataSource;

- (void)addRobots:(NSArray*)robots;
- (void)addRobot:(BAROBORobot*)robot;
- (void)clearTable;
- (void)refreshTable;

@end
