//
//  TBSetupFilesFlowLayout.m
//  TBPhone
//
//  Created by Ryan Drake on 3/17/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupFilesFlowLayout.h"

@implementation TBSetupFilesFlowLayout

-(void) prepareLayout {
    // item width is minor dimension * 0.75
    CGSize size = [[self collectionView] frame].size;
    CGFloat itemWidth = size.width < size.height ? size.width * 0.75 : size.height * 0.75;
    [self setItemSize:CGSizeMake(itemWidth, itemWidth + 40.0)];
    [self setSectionInset:UIEdgeInsetsMake(itemWidth * 0.125, itemWidth * 0.125, itemWidth * 0.125, itemWidth * 0.125)];
}

- (BOOL)shouldInvalidateLayoutForBoundsChange:(CGRect)newBounds {
    return YES;
}

-(NSArray *)layoutAttributesForElementsInRect:(CGRect)rect {
    NSLog(@"Returning attributes for elements in {(%f, %f),(%f, %f)}", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
    NSArray* attributes = [super layoutAttributesForElementsInRect:rect];

    CGRect visibleRect;
    visibleRect.origin = self.collectionView.contentOffset;
    visibleRect.size = [[self collectionView] bounds].size;

    CGFloat collectionViewHalfFrame = [[self collectionView] frame].size.width / 2.0;

    for (UICollectionViewLayoutAttributes* layoutAttributes in attributes) {
        if (CGRectIntersectsRect([layoutAttributes frame], rect)) {
            CGFloat distance = CGRectGetMidX(visibleRect) - [layoutAttributes center].x;
            CGFloat normalizedDistance = distance / collectionViewHalfFrame;
            CGFloat absNormalizedDistance = fabs(normalizedDistance);

            if (absNormalizedDistance < 1.0) {
                CGFloat zoom = 1.0 + 0.25 * (1.0 - absNormalizedDistance);
                CATransform3D rotationTransform = CATransform3DMakeRotation(normalizedDistance * M_PI_2 * 0.8, 0.0, 1.0, 0.0);
                CATransform3D zoomTransform = CATransform3DMakeScale(zoom, zoom, 1.0);
                layoutAttributes.transform3D = CATransform3DConcat(zoomTransform, rotationTransform);
                layoutAttributes.zIndex = absNormalizedDistance;
                CGFloat alpha = (1.0 - absNormalizedDistance) + 0.1;
                layoutAttributes.alpha = (alpha > 1.0) ? 1.0 : alpha;
            } else {
                layoutAttributes.alpha = 0.0;
            }
        }
    }

    return attributes;
}

@end
