//
//  TBPlayersViewController.m
//  td
//
//  Created by Ryan Drake on 8/11/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBPlayersViewController.h"
#import "TournamentSession.h"

@interface TBPlayersViewController ()

// global session
@property (strong) TournamentSession* session;

@end

@implementation TBPlayersViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptor
    NSSortDescriptor* playerNameSort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES];
    [[self arrayController] setSortDescriptors:@[playerNameSort]];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // set session
    [self setSession:representedObject];
}

- (IBAction)seatedButtonDidChange:(NSButton*)sender {
    NSTableCellView* cell = (NSTableCellView*)[sender superview];
    id ov = [cell objectValue];
    id playerId = ov[@"player_id"];
    if([sender state] == NSOnState) {
        [[self session] seatPlayer:playerId withBlock:nil];
        // select that seat
        if([self delegate]) {
            [[self delegate] performSelector:@selector(selectSeatForPlayerId:) withObject:playerId afterDelay:0.0];
        }
    } else {
        [[self session] unseatPlayer:playerId withBlock:nil];
    }
}

- (IBAction)playerChangeSelected:(id)sender {
    NSInteger selectedRow = [sender selectedRow];
    if (selectedRow != -1) {
        // get selected object
        NSArray* selectedObjects = [[self arrayController] selectedObjects];
        NSString* playerId = selectedObjects[0][@"player_id"];
        // select that seat
        if([self delegate]) {
            [[self delegate] selectSeatForPlayerId:playerId];
        }
    }
}

@end
