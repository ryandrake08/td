//
//  TBSetupRoundsDetailsViewController.h
//  TBMac
//
//  Created by Ryan Drake on 5/4/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class TournamentSession;

@interface TBSetupRoundsDetailsViewController : NSViewController

// global configuration to set
@property (nonatomic, strong) NSDictionary* configuration;

// session to call into to generate structure
@property (nonatomic, strong) TournamentSession* session;

@end
