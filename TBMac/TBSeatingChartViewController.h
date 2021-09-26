//
//  TBSeatingChartViewController.h
//  td
//
//  Created by Ryan Drake on 10/13/19.
//  Copyright © 2019 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class TournamentSession;

@interface TBSeatingChartViewController : NSViewController

// The session
@property (nonatomic, strong) TournamentSession* session;

@end
