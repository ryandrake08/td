//
//  TournamentService.m
//  td
//
//  Created by Ryan Drake on 6/28/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentService.h"

@interface TournamentService () <TournamentConnectionDelegate>

@property (nonatomic, strong) NSDictionary* dict;

@end

@implementation TournamentService

// initialize with either a NetService, address/port, or unix socket
- (instancetype)initWithNetService:(NSNetService*)service {
    if (self = [super init]) {
        _dict = @{@"service":service, @"name":[service name]};
    }
    return self;
}

- (instancetype)initWithAddress:(NSString*)address andPort:(NSInteger)port {
    if (self = [super init]) {
        _dict = @{@"address":address, @"port":@(port), @"name":[NSString stringWithFormat:@"%@:%li", address, (long)port]};
    }
    return self;
}

- (instancetype)initWithUnixSocket:(NSString*)path {
    if (self = [super init]) {
        _dict = @{@"path":path, @"name":[path lastPathComponent]};
    }
    return self;
}

// connect to a service
- (BOOL)connectTo:(TournamentConnection*)connection {
    NSNetService* service = [self dict][@"service"];
    NSString* path = [self dict][@"path"];
    NSString* address = [self dict][@"address"];
    NSNumber* port = [self dict][@"port"];

    if(service) {
        return [connection connectToService:service];
    }

    if(path) {
        return [connection connectToUnixSocketNamed:path];
    }

    if(address) {
        return [connection connectToAddress:address andPort:[port integerValue]];
    }

    return NO;
}

- (BOOL)isRemote {
    return [self dict][@"path"] == nil;
}

- (NSString*)name {
    return [self dict][@"name"];
}

// retrieve the netService, if applicable
- (NSNetService*)netService {
    return [self dict][@"service"];
}

@end