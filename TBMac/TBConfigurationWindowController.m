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

@interface TBConfigurationWindowController () <NSTabViewDelegate>

// UI
@property (strong) IBOutlet NSTabView* tabView;

// Controllers
@property (strong) IBOutlet TBTableViewController* leagueViewController;
@property (strong) IBOutlet TBTableViewController* chipsViewController;
@property (strong) IBOutlet TBTableViewController* fundingViewController;
@property (strong) IBOutlet TBTableViewController* roundsViewController;
@property (strong) IBOutlet TBTableViewController* authsViewController;

@end

@implementation TBConfigurationWindowController

- (void)windowDidLoad {
    [super windowDidLoad];

    // get view controller for the tab selected in IB
    NSTabViewItem* selectedItem = [[self tabView] selectedTabViewItem];
    [self tabView:[self tabView] didSelectTabViewItem:selectedItem];
}

- (void)setConfiguration:(NSMutableDictionary *)configuration {
    if(_configuration != configuration) {
        _configuration = [configuration mutableCopy];
    }
}

#pragma mark NSTabViewDelegate

- (void)tabView:(NSTabView*)tabView didSelectTabViewItem:(NSTabViewItem*)tabViewItem {
    // these tab view items' identifiers correspond cleverly to their instance variables
    TBTableViewController* controller = [self valueForKey:[tabViewItem identifier]];

    // set configuration
    if([controller configuration] == nil) {
        [controller setConfiguration:[self configuration]];
    }

    // set the view
    [tabViewItem setView:controller.view];
}

#pragma mark Actions

- (IBAction)cancelButtonDidChange:(id)sender {
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseCancel];
}

- (IBAction)doneButtonDidChange:(id)sender {
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseOK];
}

@end
