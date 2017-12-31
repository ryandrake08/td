//
//  TBActionClockSegue.m
//  td
//
//  Created by Ryan Drake on 12/30/17.
//  Copyright © 2017 HDna Studio. All rights reserved.
//

#import "TBActionClockSegue.h"

@interface TBActionClockAnimator : NSObject <NSViewControllerPresentationAnimator>
@end

@implementation TBActionClockAnimator

- (void)animatePresentationOfViewController:(NSViewController*)viewController fromViewController:(NSViewController*)fromViewController {
    // make sure the view has a CA layer for smooth animation
    [[viewController view] setWantsLayer:YES];

    // start out invisible
    [[viewController view] setAlphaValue:0.0f];

    // add view of presented viewcontroller
    [[fromViewController view] addSubview:[viewController view]];

    // adjust size
    CGRect frame = NSRectToCGRect([[fromViewController view] frame]);
    frame = CGRectInset(frame, 40, 40);
    [[viewController view] setFrame:NSRectFromCGRect(frame)];

    // Do some CoreAnimation stuff to present view
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext* context) {
        [context setDuration:0.5];
        [[[viewController view] animator] setAlphaValue:0.9f];
    } completionHandler:nil];

}

- (void)animateDismissalOfViewController:(NSViewController*)viewController fromViewController:(NSViewController*)fromViewController {
    // make sure the view has a CA layer for smooth animation
    [[viewController view] setWantsLayer:YES];

    // Do some CoreAnimation stuff to present view
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext* context) {
        [context setDuration:0.5];
        [[[viewController view] animator] setAlphaValue:0.0f];
    } completionHandler:^{
        [[viewController view] removeFromSuperview];
    }];
}

@end

@implementation TBActionClockSegue

- (void)perform {
    id animator = [[TBActionClockAnimator alloc] init];
    [[self sourceController] presentViewController:[self destinationController] animator:animator];
}

@end

