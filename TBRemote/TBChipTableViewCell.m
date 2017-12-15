//
//  TBChipTableViewCell.m
//  td
//
//  Created by Ryan Drake on 12/13/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBChipTableViewCell.h"
#import "TBColor+CSS.h"
#import "TBEllipseView.h"

@interface TBChipTableViewCell ()

@property (nonatomic, weak) IBOutlet TBEllipseView* colorEllipseView;
@property (nonatomic, weak) IBOutlet UILabel* valueLabel;

@end

@implementation TBChipTableViewCell

- (void)setChip:(NSDictionary*)chip {
    [[self colorEllipseView] setColor:[TBColor colorWithName:chip[@"color"]]];
    [[self valueLabel] setText:[chip[@"denomination"] stringValue]];
}

@end
