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
#import "TBInvertableImageView.h"

@interface TBChipTableViewCell ()

@property (nonatomic, weak) IBOutlet TBEllipseView* colorEllipseView;
@property (nonatomic, weak) IBOutlet TBInvertableImageView* backgroundImageView;
@property (nonatomic, weak) IBOutlet UILabel* valueLabel;

@end

@implementation TBChipTableViewCell

- (void)setChip:(NSDictionary*)chip withInvertedImage:(BOOL)inverted {
    [[self colorEllipseView] setColor:[TBColor colorWithName:chip[@"color"]]];
    [[self backgroundImageView] setImageInverted:inverted];
    [[self valueLabel] setText:[chip[@"denomination"] stringValue]];
}

@end
