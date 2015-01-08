//
//  TournamentDetailsViewController.h
//  TBRemote
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "TournamentKit_ios/TournamentKit.h"

@protocol TournamentDetailsViewControllerDelegate;

@interface TournamentDetailsViewController : UITableViewController
@property (nonatomic, weak) id <TournamentDetailsViewControllerDelegate> delegate;
@end

@protocol TournamentDetailsViewControllerDelegate <NSObject>
- (void)tournamentDetailsViewControllerDidCancel:(TournamentDetailsViewController*)controller;
- (void)tournamentDetailsViewController:(TournamentDetailsViewController*)controller didAddServer:(TournamentServer*)server;
@end

