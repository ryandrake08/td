//
//  TBSetupFilesViewController.m
//  TBPhone
//
//  Created by Ryan Drake on 3/17/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupFilesViewController.h"
#import "UIResponder+PresentingErrors.h"

@interface TBSetupFilesViewController ()

@property (nonatomic, strong) NSArray* documents;

@end

@implementation TBSetupFilesViewController

- (void)refreshDocuments {
    NSError* error;
    NSFileManager* fileManager = [NSFileManager defaultManager];

    // find documents directory
    NSURL* docDirectoryUrl = [fileManager URLForDirectory:NSDocumentDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:&error];
    if(docDirectoryUrl == nil) {
        [self presentError:error];
        return;
    }
    // get directory contents
    NSArray* contents = [fileManager contentsOfDirectoryAtURL:docDirectoryUrl includingPropertiesForKeys:@[] options:NSDirectoryEnumerationSkipsHiddenFiles error:&error];
    if(contents == nil) {
        [self presentError:error];
        return;
    }

    // set new content
    [self setDocuments:contents];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Do any additional setup after loading the view.
    [self refreshDocuments];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Navigation

- (IBAction)cancelTapped:(id)sender {
    // dismiss dialog when cancel is tapped
    [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark <UICollectionViewDataSource>

- (NSInteger)collectionView:(UICollectionView*)collectionView numberOfItemsInSection:(NSInteger)section {
    return [[self documents] count] + 1;
}

- (UICollectionViewCell*)collectionView:(UICollectionView*)collectionView cellForItemAtIndexPath:(NSIndexPath*)indexPath {
    UICollectionViewCell* cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"FileCell" forIndexPath:indexPath];

    // Configure the cell
    UIImageView* imageView = (UIImageView*)[cell viewWithTag:100];
    UILabel* label = (UILabel*)[cell viewWithTag:101];

    NSUInteger urow = (NSUInteger)[indexPath row];
    if(urow >= [[self documents] count]) {
        [imageView setImage:[UIImage imageNamed:@"f_new"]];
        [label setText:NSLocalizedString(@"Save As New...", @"Save as a new file")];
    } else {
        NSURL* url = [self documents][urow];
        [imageView setImage:[UIImage imageNamed:@"f_piranha"]];
        [label setText:[url lastPathComponent]];
    }

    return cell;
}

#pragma mark <UICollectionViewDelegate>

/*
// Uncomment this method to specify if the specified item should be highlighted during tracking
- (BOOL)collectionView:(UICollectionView *)collectionView shouldHighlightItemAtIndexPath:(NSIndexPath *)indexPath {
	return YES;
}
*/

/*
// Uncomment this method to specify if the specified item should be selected
- (BOOL)collectionView:(UICollectionView *)collectionView shouldSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    return YES;
}
*/

/*
// Uncomment these methods to specify if an action menu should be displayed for the specified item, and react to actions performed on the item
- (BOOL)collectionView:(UICollectionView *)collectionView shouldShowMenuForItemAtIndexPath:(NSIndexPath *)indexPath {
	return NO;
}

- (BOOL)collectionView:(UICollectionView *)collectionView canPerformAction:(SEL)action forItemAtIndexPath:(NSIndexPath *)indexPath withSender:(id)sender {
	return NO;
}

- (void)collectionView:(UICollectionView *)collectionView performAction:(SEL)action forItemAtIndexPath:(NSIndexPath *)indexPath withSender:(id)sender {
	
}
*/

@end
