//
//  TBConfigurationViewController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBConfigurationViewController.h"
#import "Document.h"
#import "TBConfigurationTabViewController.h"

@interface TBConfigurationViewController ()

// Configuration and session
@property (nonatomic, strong) NSMutableDictionary* configuration;

@end

@implementation TBConfigurationViewController

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    // reference the container view controllers
    if([[segue identifier] isEqualToString:@"presentConfigurationTabView"]) {
        id vc = [segue destinationController];
        [vc setRepresentedObject:[self configuration]];
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // set configuration
    [self setConfiguration:[representedObject mutableCopy]];
}

#pragma mark Actions

- (IBAction)doneButtonDidChange:(id)sender {
    // get document from sheet parent
    Document* document = [[[[[self view] window] sheetParent] windowController] document];

    // reconfigure document
    [document addConfiguration:[self configuration]];

    // dismiss
    [self dismissController:self];
}

@end
