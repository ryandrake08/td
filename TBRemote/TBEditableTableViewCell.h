//
//  TBEditableTableViewCell.h
//  td
//
//  Created by Ryan Drake on 9/25/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface TBEditableTableViewCell : UITableViewCell

// the object and keypath to observe/sync with text field
- (void)setEditableObject:(id)object keypath:(NSString*)keyPath;

@end

@interface TBEditableNumberTableViewCell : TBEditableTableViewCell

// represented object is a NSNumber. need a formatter
@property (nonatomic, strong) NSNumberFormatter* formatter;

@end
