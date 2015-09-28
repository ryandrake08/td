//
//  TBEditableTableViewCell.h
//  td
//
//  Created by Ryan Drake on 9/25/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface TBEditableTableViewCell : UITableViewCell

// if formatter is set, represented object is a NSNumber
@property (nonatomic, strong) NSNumberFormatter* formatter;

// the object and keypath to observe/sync with text field
- (void)setEditableObject:(id)object keypath:(NSString*)keyPath;

// use a picker instead of free-form text, optionally specifying picker titles
- (void)setAllowedValues:(NSArray*)allowedValues withTitles:(NSArray*)titles;

@end

@interface TBEditableNumberTableViewCell : TBEditableTableViewCell

@end
