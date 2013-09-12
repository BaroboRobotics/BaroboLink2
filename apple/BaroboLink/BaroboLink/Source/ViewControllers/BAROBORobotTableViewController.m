//
//  BAROBORobotTableViewController.m
//  BaroboLink
//
//  Created by Adam Ruggles on 9/7/13.
//  Copyright (c) 2013 Barobo, Inc,. All rights reserved.
//

#import "BAROBORobotTableViewController.h"

@implementation BAROBORobotTableViewController

- (id)init
{
    if ((self = [super init])) {
        self.dataSource = [NSMutableArray array];
    }
    return self;
}

- (void)addRobots:(NSArray*)robots
{
    [self.dataSource addObjectsFromArray:robots];
}

- (void)addRobot:(BAROBORobot*)robot {
    [self.dataSource addObject:robot];
}

- (void)clearTable
{
    [self.dataSource removeAllObjects];
}

- (void)refreshTable
{
    [self.tableView reloadData];
}

- (BAROBORobot*)selectedRobot {
    if ([self.dataSource count] == 0) {
        return nil;
    }
    NSInteger row = [[self tableView] selectedRow];
    if (row < 0) {
        row = 0;
    }
    return [self.dataSource objectAtIndex:row];
}

/**
 * NSTableViewDataSource implementation.
 */
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [self.dataSource count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    BAROBORobot *robot = [self.dataSource objectAtIndex:row];
    NSString *identifier = [tableColumn identifier];
    return [robot valueForKey:identifier];
}

@end
