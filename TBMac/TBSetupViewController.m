//
//  TBSetupViewController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupViewController.h"
#import "TBMacDocument.h"
#import "TBSetupTabViewController.h"

@interface TBSetupViewController ()

// Configuration being worked on
@property (nonatomic, strong) NSMutableDictionary* configuration;

@end

@implementation TBSetupViewController

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    // reference the container view controllers
    if([[segue identifier] isEqualToString:@"presentConfigurationTabView"]) {
        id vc = [segue destinationController];
        [vc setRepresentedObject:[self configuration]];
    }
}

- (void)setInitialConfiguration:(NSDictionary*)config {
    // set configuration (use a deep copy)
    [self setConfiguration:[NSKeyedUnarchiver unarchiveObjectWithData:[NSKeyedArchiver archivedDataWithRootObject:config]]];
}

#pragma mark Actions

- (IBAction)doneButtonDidChange:(id)sender {
    // get document from sheet parent
    TBMacDocument* document = [[[[[self view] window] sheetParent] windowController] document];

    // reconfigure document
    [document addConfiguration:[self configuration]];

    // dismiss
    [self dismissController:self];
}

@end
