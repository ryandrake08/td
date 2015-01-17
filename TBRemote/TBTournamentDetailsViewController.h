//
//  TBTournamentDetailsViewController.h
//  TBRemote
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "TournamentKit/TournamentKit.h"

@protocol TBTournamentDetailsViewControllerDelegate;

@interface TBTournamentDetailsViewController : UITableViewController
@property (nonatomic, weak) id <TBTournamentDetailsViewControllerDelegate> delegate;
@end

@protocol TBTournamentDetailsViewControllerDelegate <NSObject>
- (void)tournamentDetailsViewControllerDidCancel:(TBTournamentDetailsViewController*)controller;
- (void)tournamentDetailsViewController:(TBTournamentDetailsViewController*)controller didAddServer:(TournamentServerInfo*)server;
@end

