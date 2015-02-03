//
//  TBFundingTableCellView.h
//  Tournament Buddy
//
//  Created by Ryan Drake on 2/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TBFundingTableCellView : NSTableCellView

@property IBOutlet NSTextField* costField;
@property IBOutlet NSTextField* feeField;
@property IBOutlet NSTextField* equityField;
@property IBOutlet NSTextField* chipsField;
@property IBOutlet NSPopUpButton* typeButton;
@property IBOutlet NSButton* allowedButton;
@property IBOutlet NSPopUpButton* untilButton;

@end
