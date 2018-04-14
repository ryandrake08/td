//
//  TBChipTableViewCell.h
//  td
//
//  Created by Ryan Drake on 12/13/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>
@class TBEllipseView;
@class TBInvertableImageView;

@interface TBChipTableViewCell : UITableViewCell

@property (nonatomic, weak) IBOutlet TBEllipseView* colorEllipseView;
@property (nonatomic, weak) IBOutlet TBInvertableImageView* backgroundImageView;
@property (nonatomic, weak) IBOutlet UILabel* valueLabel;

@end
