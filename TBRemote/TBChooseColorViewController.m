//
//  TBChooseColorViewController.m
//  td
//
//  Created by Ryan Drake on 10/8/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBChooseColorViewController.h"
#import "TBEllipseView.h"
#import "TBColor+CSS.h"
#import "TBNotifications.h"

@implementation TBChooseColorViewController

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark UICollectionViewDataSource

- (NSInteger)collectionView:(UICollectionView*)collectionView numberOfItemsInSection:(NSInteger)section {
    return (NSInteger)[[TBColor allColorNames] count];
}

- (UICollectionViewCell*)collectionView:(UICollectionView*)collectionView cellForItemAtIndexPath:(NSIndexPath*)indexPath {
    UICollectionViewCell* cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"ChooseColorCell" forIndexPath:indexPath];
    NSUInteger urow = (NSUInteger)[indexPath row];
    NSString* colorName = [TBColor allColorNames][urow];
    [(TBEllipseView*)[cell viewWithTag:100] setColor:[TBColor colorWithName:colorName]];
    return cell;
}

#pragma mark UICollectionViewDelegate

- (void)collectionView:(UICollectionView*)collectionView didSelectItemAtIndexPath:(NSIndexPath*)indexPath {
    NSUInteger urow = (NSUInteger)[indexPath row];
    [self object][@"color"] = [TBColor allColorNames][urow];
    [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];
    [[self navigationController] popViewControllerAnimated:YES];
}

@end
