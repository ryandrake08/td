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
@property (strong) IBOutlet TBTableViewController* chipsViewController;
@property (strong) IBOutlet TBTableViewController* fundingViewController;
@property (strong) IBOutlet TBTableViewController* playersViewController;
@property (strong) IBOutlet TBTableViewController* roundsViewController;

@end

@implementation TBConfigurationWindowController

- (void)windowDidLoad {
    [super windowDidLoad];

    // pass whole-configuration changes to session
    [[self KVOController] observe:self keyPath:@"configuration" options:0 action:@selector(configureSession)];

    // get view controller for the tab selected in IB
    NSTabViewItem* selectedItem = [[self tabView] selectedTabViewItem];
    [self tabView:[self tabView] didSelectTabViewItem:selectedItem];
}

- (void)configureSession {
    // only send parts of configuration that changed
    NSMutableDictionary* configToSend = [[self configuration] mutableCopy];
    NSMutableArray* keysToRemove = [NSMutableArray array];
    [configToSend enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL* stop) {
        NSString* propertyName = [key asCamelCaseFromUnderscore];
        if([obj isEqual:[[self session] valueForKey:propertyName]]) {
            [keysToRemove addObject:key];
        }
    }];
    [configToSend removeObjectsForKeys:keysToRemove];

    if([configToSend count] > 0) {
        [[self session] configure:configToSend withBlock:^(id json) {
            if(![json isEqual:[self configuration]]) {
                [[self configuration] setDictionary:json];
            }
        }];
    }
}

#pragma mark NSWindowDelegate

- (BOOL)windowShouldClose:(id)sender {
    [self configureSession];
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

    // configure session when switching tabs (for now)
    [self configureSession];

    // set the view
    [tabViewItem setView:controller.view];
}

@end
