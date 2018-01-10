//
//  TBMovementViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBMovementViewController.h"
#import "NSObject+FBKVOController.h"

@interface TBMovementViewController ()

@property (strong) IBOutlet NSArrayController* arrayController;

@end

@implementation TBMovementViewController

- (IBAction)dismissClicked:(id)sender {
    NSTableCellView* cell = (NSTableCellView*)[sender superview];
    id object = [cell objectValue];
    [[self representedObject] removeObject:object];

    // close automatically if none left
    if([[[self arrayController] arrangedObjects] count] == 0) {
        [self dismissViewController:self];
    }
}

#pragma mark NSWindowDelegate

- (void)viewDidLoad {
    [super viewDidLoad];

    // resize window when array controller contents change
    [[self KVOController] observe:[self arrayController] keyPath:@"arrangedObjects.count" options:NSKeyValueObservingOptionInitial block:^(id observer, NSArrayController* object, NSDictionary *change) {
        NSUInteger cellsToShow = [[object arrangedObjects] count];
        cellsToShow = cellsToShow > 4 ? 4 : (cellsToShow < 1 ? 1 : cellsToShow);
        CGRect rect = [[self view] bounds];
        rect.size.height = cellsToShow * 38.0f + 116.0f;
        [[self view] setBounds:rect];
    }];
}

#pragma mark Operations

- (void)addObjects:(NSArray*)objects {
    [[self arrayController] addObjects:objects];
}

@end
