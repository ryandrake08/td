//
//  UIResponder+PresentingErrors.m
//  CapitaineTrain
//
//  Created by Pierre de La Morinerie on 24/01/13.
//  Copyright (c) 2013 Capitaine Train. All rights reserved.
//

#import "UIResponder+PresentingErrors.h"
#import <UIKit/UIAlertController.h>
#import <UIKit/UIWindow.h>

@implementation UIResponder (PresentingErrors)

- (void) presentError:(NSError *)error
{
    [self.nextResponder presentError:error];
}

@end

@implementation UIApplication (PresentingErrors)

- (void) presentError:(NSError *)error
{
    UIAlertController* alert = [UIAlertController alertControllerWithTitle:error.localizedDescription message:error.localizedFailureReason preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"OK", nil) style:UIAlertActionStyleCancel handler:nil]];
    [[[self keyWindow] rootViewController] presentViewController:alert animated:YES completion:nil];
}

@end
