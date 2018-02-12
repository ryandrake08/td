//
//  TBEditableTableViewCell
//  td
//
//  Created by Ryan Drake on 9/25/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface TBKVOTableViewCell : UITableViewCell

// object (KV observable) to be edited
@property (nonatomic, strong) id object;

// keypath of object to be edited
@property (nonatomic, copy) NSString* keyPath;

// if underlying value is an array, table view cell will show the array count
// if this is set, adjust that count by this amount (used to ignore dummy round)
@property (nonatomic, assign) long valueOffset;

@end

@interface TBFormattedKVOTableViewCell : TBKVOTableViewCell

// if represented object is a NSNumber, use this to format to and from text
@property (nonatomic, strong) NSNumberFormatter* formatter;

@end

@interface TBLabelTableViewCell : TBFormattedKVOTableViewCell

// ui outlet
@property (nonatomic, strong) IBOutlet UILabel* label;

@end

@interface TBTextFieldTableViewCell : TBFormattedKVOTableViewCell

// ui outlet
@property (nonatomic, strong) IBOutlet UITextField* textField;

@end

@interface TBPickableTextTableViewCell : TBTextFieldTableViewCell

// use a picker instead of free-form text, optionally specifying picker titles
- (void)setAllowedValues:(NSArray*)allowedValues withTitles:(NSArray*)titles;

@end

@interface TBSliderTableViewCell : TBLabelTableViewCell

// ui outlet
@property (nonatomic, strong) IBOutlet UISlider* slider;

@end

@interface TBCheckmarkNumberTableViewCell : TBKVOTableViewCell

@end

@interface TBCheckmarkValueExistsTableViewCell : TBKVOTableViewCell

@end

@interface TBEllipseTableViewCell : TBKVOTableViewCell

@end
