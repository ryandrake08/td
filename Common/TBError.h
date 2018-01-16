//
//  TBError.h
//  td
//
//  Created by Ryan Drake on 1/16/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import <Foundation/NSError.h>
#import <Foundation/NSString.h>

extern NSErrorDomain const TBErrorDomain;

typedef NS_ERROR_ENUM(TBErrorDomain, TBErrorCode) {
    TBErrorCouldNotCreateStreams = 1000
};
