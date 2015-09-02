//
//  TBMovementWindowController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBMovementWindowController.h"
#import "NSObject+FBKVOController.h"

@implementation TBMovementWindowController

- (IBAction)dismissClicked:(id)sender {
    NSTableCellView* cell = (NSTableCellView*)[sender superview];
    id object = [cell objectValue];
    [[self arrayController] removeObject:object];

    // close automatically if none left
    if([[[self arrayController] arrangedObjects] count] == 0) {
        [self close];
    }
}

#pragma mark NSWindowDelegate

- (void)windowDidLoad {
    [super windowDidLoad];

    // resize window when array controller contents change
    [[self KVOController] observe:[self arrayController] keyPath:@"arrangedObjects.count" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        NSUInteger cellsToShow = [[[self arrayController] arrangedObjects] count];
        cellsToShow = MAX(MIN(cellsToShow, 4), 1);
        CGRect rect = [[self window] frame];
        rect.size.height = cellsToShow * 96.0 + 32.0;
        [[self window] setFrame:[[self window] frameRectForContentRect:rect] display:YES];
    }];
}

- (void)windowWillClose:(NSNotification *)notification {
    [[self arrayController] setContent:nil];
}

@end
