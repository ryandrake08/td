//
//  TBSeatingChartCollectionViewItem.h
//  td
//
//  Created by Ryan Drake on 10/14/19.
//  Copyright © 2019 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class TournamentSession;

@interface TBSeatingChartCollectionViewItem : NSCollectionViewItem

// The session
@property (nonatomic, strong) TournamentSession* session;

// Table name plus custom setter that sets filter
@property (nonatomic, copy) NSString* tableName;
- (void)setTableName:(NSString*)tableName;

@end

