//
//  TBEditableTableViewCell
//  td
//
//  Created by Ryan Drake on 9/25/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface TBEditableTableViewCell : UITableViewCell

// the object and keypath to observe/sync with field
- (void)setEditableObject:(id)object keypath:(NSString*)keyPath;

@end

@interface TBEditableTextTableViewCell : TBEditableTableViewCell

@end

@interface TBEditableNumberTableViewCell : TBEditableTextTableViewCell

// represented object is a NSNumber, use this to format to and from text
@property (nonatomic, strong) NSNumberFormatter* formatter;

@end

@interface TBPickableTextTableViewCell : TBEditableTextTableViewCell

// use a picker instead of free-form text, optionally specifying picker titles
- (void)setAllowedValues:(NSArray*)allowedValues withTitles:(NSArray*)titles;

@end

