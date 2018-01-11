//
//  NSToolbarBadgedItem.h
//  ToolbarBadges
//
//  Created by Robert McDowell on 04/11/2013.
//  Copyright (c) 2013 Bert McDowell. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSToolbarBadgedItem : NSToolbarItem

@property (nonatomic, copy) IBInspectable NSString* badgeValue;

@property (nonatomic, copy) IBInspectable NSString* badgeFontName;
@property (nonatomic, copy) IBInspectable NSColor* badgeTextColor;
@property (nonatomic, copy) IBInspectable NSColor* badgeTextShadowColor;

@property (nonatomic, copy) IBInspectable NSColor* badgeLineColor;
@property (nonatomic, copy) IBInspectable NSColor* badgeFillColor;
@property (nonatomic, copy) IBInspectable NSColor* badgeShadowColor;

@end
