//
//  TBConnectToWindowController.m
//  td
//
//  Created by Ryan Drake on 8/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBConnectToWindowController.h"

@implementation TBConnectToWindowController

#pragma mark Actions

- (IBAction)cancelButtonDidChange:(id)sender {
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseCancel];
}

- (IBAction)connectButtonDidChange:(id)sender {
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseOK];
}

@end
