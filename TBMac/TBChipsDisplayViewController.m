//
//  TBChipsDisplayViewController.m
//  td
//
//  Created by Ryan Drake on 9/10/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBChipsDisplayViewController.h"
#import "TBEllipseImageView.h"
#import "NSObject+FBKVOController.h"

@interface TBChipsDisplayViewController ()

@end

@implementation TBChipsDisplayViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // register for KVO
    [[[self tableView] KVOController] observe:[[self session] state] keyPath:@"available_chips" options:0 action:@selector(reloadData)];
}

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [[[self arrayController] arrangedObjects] count];
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView*)aTableView viewForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    NSTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    [[result imageView] bind:@"color" toObject:result withKeyPath:@"objectValue.color" options:@{NSValueTransformerNameBindingOption:@"TBColorValueTransformer"}];
    return result;
}

@end
