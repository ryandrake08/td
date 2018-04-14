//
//  NSTableCellView.h
//  td
//
//  Created by Ryan Drake on 1/15/18.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class TBEllipseView;
@class TBInvertableImageView;

@interface TBChipTableCellView : NSTableCellView

// UI
@property (nonatomic, weak) IBOutlet TBEllipseView* colorEllipseView;
@property (nonatomic, weak) IBOutlet TBInvertableImageView* backgroundImageView;

@end
