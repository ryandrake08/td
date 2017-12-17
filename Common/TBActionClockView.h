//
//  TBActionClockView.h
//
//  Adapted from BEMAnalogClockView
//
//  Created by Boris Emorine on 2/23/14.
//  Copyright (c) 2014 Boris Emorine. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import "TBCommon.h"

@protocol TBActionClockDelegate;

/// TBActionClock is an TBView subclass that gives you an easy way to create beautiful, interactive analog clocks for iOS.
@interface TBActionClockView : TBView


//------------------------------------------------------------------------------------//
//----- DELEGATE ---------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

/// TBActionClockView delegate object is essential to the clock. The delegate provides the clock with data and various parameters. The delegate can be set from the interface or from code.
@property (nonatomic, weak) IBOutlet id <TBActionClockDelegate> delegate;


//------------------------------------------------------------------------------------//
//----- PROPERTIES -------------------------------------------------------------------//
//------------------------------------------------------------------------------------//

/// The seconds property. Used to set up the second hand. Default value is 0.
@property (nonatomic) IBInspectable double seconds;

/// If set to YES, the hands will cast a shadow. Default value is YES.
@property (nonatomic) IBInspectable BOOL enableShadows;

/// If set to YES, the graduation on the clock will be visible. See the methods bellow to costumize the graduations. Default value is YES.
@property (nonatomic) IBInspectable BOOL enableGraduations;

/// If set to YES, the digits (5-60) will be displayed on the face of the clock. Default value is YES.
@property (nonatomic) IBInspectable BOOL enableDigit;

/// If set to YES, the countdown arc will be displayed on the face of the clock. Default value is YES.
@property (nonatomic) IBInspectable BOOL enableArc;


//----- CLOCK'S FACE CUSTOMIZATION -----//

/// The background color of the clock's face.
@property (nonatomic, strong) IBInspectable TBColor* faceBackgroundColor;

/// The alpha of the clock's face.
@property (nonatomic) IBInspectable CGFloat faceBackgroundAlpha;

/// The color of the clock's border.
@property (nonatomic, strong) IBInspectable TBColor* borderColor;

/// The alpha of the clock's border.
@property (nonatomic) IBInspectable CGFloat borderAlpha;

/// The width of the clock's border.
@property (nonatomic) IBInspectable CGFloat borderWidth;

/// The font of the digits appearing inside the clock
@property (nonatomic, strong) IBInspectable TBFont* digitFont;

/// The color of the digits appearing inside the clock
@property (nonatomic, strong) IBInspectable TBColor* digitColor;

/// The offset for the position of the digits on the clock's face. A value >0 will make the digits appear further away from the center of the clock. A valut <0 will make them closer to the center of the clock. Default value is 0.0.
@property (nonatomic) IBInspectable CGFloat digitOffset;


//----- ARC CUSTOMIZATION -----//

/// The background color of the countdown arc
@property (nonatomic, strong) IBInspectable TBColor* arcBackgroundColor;

/// The alpha of the countdown arc.
@property (nonatomic) IBInspectable CGFloat arcBackgroundAlpha;

/// The color of the countdown arc's border.
@property (nonatomic, strong) IBInspectable TBColor* arcBorderColor;

/// The alpha of the countdown arc's border.
@property (nonatomic) IBInspectable CGFloat arcBorderAlpha;

/// The width of the countdown arc's border.
@property (nonatomic) IBInspectable CGFloat arcBorderWidth;

/// If YES, arc fills in as clock counts down to zero. Default value is YES.
@property (nonatomic) IBInspectable BOOL arcFillsIn;

//----- HAND CUSTOMIZATION -----//

/// The color of the clock's hand. Default value is whiteColor.
@property (nonatomic, strong) IBInspectable TBColor* handColor;

/// The alpha of the clock's hand. Default value is 1.0.
@property (nonatomic) IBInspectable CGFloat handAlpha;

/// The width of the clock's hand. Default value is 1.0.
@property (nonatomic) IBInspectable CGFloat handWidth;

/// The length of the clock's hand. Default value is 60.
@property (nonatomic) IBInspectable CGFloat handLength;

/// The length of the offside part of the clock's hand. Default value is 20.
@property (nonatomic) IBInspectable CGFloat handOffsideLength;

@end


@protocol TBActionClockDelegate <NSObject>

@optional

//----- GRADUATION CUSTOMIZATION -----//

/** The color of the graduation line at a given index. This is called for each graduation on the clock.
 @param clock The clock object requesting the graduation color.
 @param index The index from 0 to 59 of a given graduation. The first value for the index is 0.
 @return The color of the graduation at the specific index. */
- (TBColor*)analogClock:(TBActionClockView*)clock graduationColorForIndex:(NSInteger)index;

/** The alpha of the graduation line at a given index. This is called for each graduation on the clock.
 @param clock The clock object requesting the graduation alpha.
 @param index The index from 0 to 59 of a given graduation. The first value for the index is 0.
 @return The alpha value of the graduation at the specific index. */
- (CGFloat)analogClock:(TBActionClockView*)clock graduationAlphaForIndex:(NSInteger)index;

/** The width of the graduation line at a given index. This is called for each graduation on the clock.
 @param clock The clock object requesting the graduation width.
 @param index The index from 0 to 59 of a given graduation. The first value for the index is 0.
 @return The width value of the graduation at the specific index. */
- (CGFloat)analogClock:(TBActionClockView*)clock graduationWidthForIndex:(NSInteger)index;

/** The length of the graduation line at a given index. This is called for each graduation on the clock.
 @param clock The clock object requesting the graduation length.
 @param index The index from 0 to 59 of a given graduation. The first value for the index is 0.
 @return The length value of the graduation at the specific index. */
- (CGFloat)analogClock:(TBActionClockView*)clock graduationLengthForIndex:(NSInteger)index;

/** The offset of the graduation line from the border of the clock at a given index. This is called for each graduation on the clock.
 @param clock The clock object requesting the graduation offset.
 @param index The index from 0 to 59 of a given graduation. The first value for the index is 0.
 @return The offset value from the border of the clock of the graduation at the specific index. */
- (CGFloat)analogClock:(TBActionClockView*)clock graduationOffsetForIndex:(NSInteger)index;

@end
