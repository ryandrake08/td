//
//  TBChipsTableCellView.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 2/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBChipsTableCellView.h"
#import "TournamentKit/TournamentKit.h"

@interface TBChipsTableCellView ()

@property IBOutlet NSColorWell* colorWell;

@end

@implementation TBChipsTableCellView

- (void)setObjectValue:(id)objectValue {
    [super setObjectValue:objectValue];

    if([self objectValue]) {
        NSString* colorName = [[self objectValue] objectForKey:@"color"];
        NSColor* color = [NSColor colorWithName:colorName];
        [[self colorWell] setColor:color];
    }
}

#pragma mark Controls

- (IBAction)colorValueDidChange:(id)sender {
    NSColorWell* well = sender;
    NSString* colorName = [[well color] name];
    [[self objectValue] setObject:colorName forKey:@"color"];
}

@end
