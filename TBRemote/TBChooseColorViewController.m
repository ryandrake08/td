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

@interface TBChooseColorViewController ()

@end

@implementation TBChooseColorViewController

//static NSString* const reuseIdentifier = @"Cell";

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

#pragma mark <UICollectionViewDataSource>

- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView*)collectionView {
    return 1;
}


- (NSInteger)collectionView:(UICollectionView*)collectionView numberOfItemsInSection:(NSInteger)section {
    return [[TBColor allColorNames] count];
}

- (UICollectionViewCell*)collectionView:(UICollectionView*)collectionView cellForItemAtIndexPath:(NSIndexPath*)indexPath {
    UICollectionViewCell* cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"ChooseColorCell" forIndexPath:indexPath];
    NSString* colorName = [TBColor allColorNames][[indexPath row]];
    [(TBEllipseView*)[cell viewWithTag:100] setColor:[TBColor colorWithName:colorName]];
    return cell;
}

#pragma mark <UICollectionViewDelegate>

/*
// Uncomment this method to specify if the specified item should be highlighted during tracking
- (BOOL)collectionView:(UICollectionView*)collectionView shouldHighlightItemAtIndexPath:(NSIndexPath*)indexPath {
	return YES;
}
*/

/*
// Uncomment this method to specify if the specified item should be selected
- (BOOL)collectionView:(UICollectionView*)collectionView shouldSelectItemAtIndexPath:(NSIndexPath*)indexPath {
    return YES;
}
*/

/*
// Uncomment these methods to specify if an action menu should be displayed for the specified item, and react to actions performed on the item
- (BOOL)collectionView:(UICollectionView*)collectionView shouldShowMenuForItemAtIndexPath:(NSIndexPath*)indexPath {
	return NO;
}

- (BOOL)collectionView:(UICollectionView*)collectionView canPerformAction:(SEL)action forItemAtIndexPath:(NSIndexPath*)indexPath withSender:(id)sender {
	return NO;
}

- (void)collectionView:(UICollectionView*)collectionView performAction:(SEL)action forItemAtIndexPath:(NSIndexPath*)indexPath withSender:(id)sender {
	
}
*/

@end
