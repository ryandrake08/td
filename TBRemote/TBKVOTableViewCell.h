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

@end

@interface TBFormattedKVOTableViewCell : TBKVOTableViewCell

// if represented object is a NSNumber, use this to format to and from text
@property (nonatomic, strong) NSNumberFormatter* formatter;

@end

@interface TBLabelTableViewCell : TBFormattedKVOTableViewCell

@end

@interface TBTextFieldTableViewCell : TBFormattedKVOTableViewCell

@end

@interface TBPickableTextTableViewCell : TBTextFieldTableViewCell

// use a picker instead of free-form text, optionally specifying picker titles
- (void)setAllowedValues:(NSArray*)allowedValues withTitles:(NSArray*)titles;

@end

@interface TBCheckmarkNumberTableViewCell : TBKVOTableViewCell

@end

@interface TBEllipseTableViewCell : TBKVOTableViewCell

@end
