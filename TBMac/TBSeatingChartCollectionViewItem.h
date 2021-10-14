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

// Array of seats that represents this table. Data structure is session.seating_chart_entry
@property (nonatomic, copy) NSArray* seats;

// Table name
@property (nonatomic, copy) NSString* tableName;

@end

