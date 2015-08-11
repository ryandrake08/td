//
//  TBConfigurationWindowController.m
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBConfigurationWindowController.h"
#import "TBTableViewController.h"
#import "NSString+CamelCase.h"
#import "NSObject+FBKVOController.h"

@interface TBConfigurationWindowController () <NSWindowDelegate, NSTabViewDelegate>

// UI
@property (strong) IBOutlet NSTabView* tabView;

// Controllers
@property (strong) IBOutlet TBTableViewController* playersViewController;
@property (strong) IBOutlet TBTableViewController* chipsViewController;
@property (strong) IBOutlet TBTableViewController* fundingViewController;
@property (strong) IBOutlet TBTableViewController* roundsViewController;
@property (strong) IBOutlet TBTableViewController* authsViewController;

@end

@implementation TBConfigurationWindowController

- (void)windowDidLoad {
    [super windowDidLoad];

    // pass whole-configuration changes to session
    [[self KVOController] observe:self keyPath:@"configuration" options:0 block:^(id observer, id object, NSDictionary *change) {
        [[self session] selectiveConfigureAndUpdate:[self configuration]];
    }];

    // get view controller for the tab selected in IB
    NSTabViewItem* selectedItem = [[self tabView] selectedTabViewItem];
    [self tabView:[self tabView] didSelectTabViewItem:selectedItem];
}

#pragma mark NSWindowDelegate

- (BOOL)windowShouldClose:(id)sender {
    [[self session] selectiveConfigureAndUpdate:[self configuration]];
    return YES;
}

#pragma mark NSTabViewDelegate

- (void)tabView:(NSTabView*)tabView didSelectTabViewItem:(NSTabViewItem*)tabViewItem {
    // these tab view items' identifiers correspond cleverly to their instance variables
    TBTableViewController* controller = [self valueForKey:[tabViewItem identifier]];

    // set configuration and session
    if([controller configuration] == nil) {
        [controller setConfiguration: [self configuration]];
    }

    // set the view
    [tabViewItem setView:controller.view];
}

@end
