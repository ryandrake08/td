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

#pragma mark UICollectionViewDataSource

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

#pragma mark UICollectionViewDelegate

- (void)collectionView:(UICollectionView*)collectionView didSelectItemAtIndexPath:(NSIndexPath*)indexPath {
    [self object][@"color"] = [TBColor allColorNames][[indexPath row]];
    [[self navigationController] popViewControllerAnimated:YES];
}

@end
