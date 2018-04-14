//
//  TBSeatingViewController.h
//  td
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"
#import "TBPlayersViewController.h"
@class TournamentSession;

@interface TBSeatingViewController : TBTableViewController <TBPlayersViewDelegate>

// The session
@property (nonatomic, strong) TournamentSession* session;

@end
