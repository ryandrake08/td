//
//  TBSetupChipsViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupChipsViewController.h"
#import "TBEllipseView.h"
#import "TBColor+CSS.h"

@implementation TBSetupChipsViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setArrangedObjects:[self configuration][@"available_chips"]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupChipCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];
    
    [(UILabel*)[cell viewWithTag:101] setText:[object[@"denomination"] stringValue]];
    [(UILabel*)[cell viewWithTag:102] setText:[object[@"count_available"] stringValue]];

    // set up ellipse view
    TBEllipseView* ellipseView = (TBEllipseView*)[cell viewWithTag:100];
    [ellipseView setColor:[TBColor colorWithName:object[@"color"]]];
    
    return cell;
}

- (id)newObject {
    NSString* color = [TBColor randomColorName];
    NSNumber* denomination = @1;
    NSNumber* count_available = @100;

    return [[NSMutableDictionary alloc] initWithObjectsAndKeys:color, @"color", denomination, @"denomination", count_available, @"count_available", nil];
}

- (IBAction)addItem:(id)sender {
    [self addArrangedObject:[self newObject]];
}

@end
