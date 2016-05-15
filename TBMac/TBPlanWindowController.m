//
//  TBPlanWindowController.m
//  td
//
//  Created by Ryan Drake on 5/15/16.
//  Copyright © 2016 HDna Studio. All rights reserved.
//

#import "TBPlanWindowController.h"

@interface TBPlanWindowController ()

@end

@implementation TBPlanWindowController

#pragma mark Actions

- (IBAction)cancelButtonDidChange:(id)sender {
    [[[self window] sheetParent] endSheet:[self window] returnCode:NSModalResponseCancel];
}

- (IBAction)doneButtonDidChange:(id)sender {
    [[[self window] sheetParent] endSheet:[self window] returnCode:NSModalResponseOK];
}

@end
