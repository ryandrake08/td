//
//  TBPlayersViewController.h
//  td
//
//  Created by Ryan Drake on 8/11/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"
@class TournamentSession;

@protocol TBPlayersViewDelegate <NSObject>

- (void)selectSeatForPlayerId:(NSString*)playerId;

@end

@interface TBPlayersViewController : TBTableViewController

// The session
@property (nonatomic, strong) TournamentSession* session;

@property (weak) NSObject<TBPlayersViewDelegate>* delegate;

@end
