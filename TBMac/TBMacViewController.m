//
//  TBMacViewController.m
//  TBMac
//
//  Created by Ryan Drake on 1/7/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBMacViewController.h"
#import "Document.h"
#import "TBSeatingViewController.h"
#import "TBResultsViewController.h"
#import "TBPlayersViewController.h"

@interface TBMacViewController ()

// The global shared session
@property (strong) TournamentSession* session;

// UI Outlets
@property (weak) IBOutlet NSView* leftPaneView;
@property (weak) IBOutlet NSView* rightPaneView;
@property (weak) IBOutlet NSView* centerPaneView;

// View Controllers
@property (strong) TBSeatingViewController* seatingViewController;
@property (strong) TBPlayersViewController* playersViewController;
@property (strong) TBResultsViewController* resultsViewController;

@end

@implementation TBMacViewController

- (Document*)document {
    return [[[[self view] window] windowController] document];
}

- (void)prepareForSegue:(NSStoryboardSegue *)segue sender:(id)sender {
    // reference the container view controllers
    if([[segue identifier] isEqualToString:@"presentSeatingView"]) {
        [self setSeatingViewController:[segue destinationController]];
    } else if([[segue identifier] isEqualToString:@"presentPlayersView"]) {
        [self setPlayersViewController:[segue destinationController]];
    } else if([[segue identifier] isEqualToString:@"presentResultsView"]) {
        [self setResultsViewController:[segue destinationController]];
    }

    // once we have both of these, set seating vc as delegate of players vc
    if([self playersViewController] && [self seatingViewController]) {
        [[self playersViewController] setDelegate:[self seatingViewController]];
    }
}

- (void)viewDidLoad {
    if([NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [super viewDidLoad];
    }
}

- (void)loadView {
    [super loadView];
    if(![NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [self viewDidLoad];
    }
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // set session
    [self setSession:representedObject];

    // also set for containers
    [[self seatingViewController] setRepresentedObject:representedObject];
    [[self playersViewController] setRepresentedObject:representedObject];
    [[self resultsViewController] setRepresentedObject:representedObject];
}

#pragma mark Attributes

- (NSView*)printableView {
    return [[self seatingViewController] view];
}


@end
