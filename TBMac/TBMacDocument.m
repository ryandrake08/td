//
//  TBMacDocument.m
//  td
//
//  Created by Ryan Drake on 1/18/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBMacDocument.h"
#import "TBMacViewController.h"
#import "TBViewerViewController.h"
#import "TournamentSession.h"
#import "TournamentDaemon.h"
#import "NSObject+FBKVOController.h"

@interface TBMacDocument ()

// Model
@property (strong) TournamentDaemon* server;
@property (strong, readwrite) TournamentSession* session;
@property (strong, readwrite) NSMutableDictionary* configuration;

@end

@implementation TBMacDocument

- (instancetype)init {
    self = [super init];
    if (self) {
        _server = [[TournamentDaemon alloc] init];
        _session = [[TournamentSession alloc] init];
        _configuration = [[NSMutableDictionary alloc] init];

        // register for KVO
        [[self KVOController] observe:self keyPaths:@[@"session.state.connected", @"session.state.authorized"] options:0 block:^(id observer, TBMacDocument* object, NSDictionary *change) {
            if([[[object session] state][@"connected"] boolValue] && [[[object session] state][@"authorized"] boolValue]) {
                NSLog(@"Connected and authorized locally");

                // on connection, send entire configuration to session, unconditionally, and then replace with whatever the session has
                NSLog(@"Synchronizing session unconditionally");
                [[self session] configure:[self configuration] withBlock:^(id json) {
                    if(![json isEqual:[self configuration]]) {
                        NSLog(@"Document differs from session");
                        [[self configuration] setDictionary:json];
                    }
                }];
            }
        }];

        // whenever tournament name changes, re-publish
        [[self KVOController] observe:self keyPath:@"session.state.name" options:NSKeyValueObservingOptionInitial block:^(id observer, TBMacDocument* object, NSDictionary *change) {
            [[self server] publishWithName:[[object session] state][@"name"]];
        }];

        // Start serving using this device's auth key
        TournamentService* service = [[self server] startWithAuthCode:[TournamentSession clientIdentifier]];

        // Start the session, connecting locally
        if(![[self session] connectToTournamentService:service]) {
            // TODO: handle error
        }
    }
    return self;
}

- (void)close {
    [[self KVOController] unobserveAll];
    [[self session] disconnect];
    [[self server] stop];
    [super close];
}

+ (BOOL)autosavesInPlace {
    return YES;
}

- (void)makeWindowControllers {
    NSWindowController* wc = [[NSStoryboard storyboardWithName:@"TBMac" bundle:nil] instantiateControllerWithIdentifier:@"Document Window Controller"];
    NSViewController* vc = [wc contentViewController];
    [vc setRepresentedObject: [self session]];
    [self addWindowController:wc];
}

- (NSData*)dataOfType:(NSString*)typeName error:(NSError**)outError {
    if([typeName isEqualToString:@"JSON"] || [typeName isEqualToString:@"TournamentBuddy"]) {
        // serialize json configuration to NSData
        return [NSJSONSerialization dataWithJSONObject:[self configuration] options:0 error:outError];
    } else if([typeName isEqualToString:@"CSV"]) {
        // serialize results to NSData
        NSMutableArray* results = [[NSMutableArray alloc] initWithObjects:@"Player,Finish,Win", nil];
        // 1 result per line
        [[[self session] state][@"results"] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL* stop) {
            NSString* line = [NSString stringWithFormat:@"\"%@\",%@,%@", obj[@"name"], obj[@"place"], obj[@"payout"]];
            [results addObject:line];
        }];
        // combine
        NSString* csv = [results componentsJoinedByString:@"\n"];
        // use WINDOWS-1252 for now
        return [csv dataUsingEncoding:NSWindowsCP1252StringEncoding];
    }
    return nil;
}

- (BOOL)readFromData:(NSData*)data ofType:(NSString*)typeName error:(NSError**)outError {
    if([typeName isEqualToString:@"JSON"] || [typeName isEqualToString:@"TournamentBuddy"]) {
        // deserialize json configuration
        [[self configuration] setDictionary:[NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:outError]];
    }
    return *outError == nil;
}

- (NSPrintOperation*)printOperationWithSettings:(NSDictionary*)ps error:(NSError**)outError {
    TBMacViewController* vc = (TBMacViewController*)[[[self windowControllers] firstObject] contentViewController];
    NSPrintInfo* printInfo = [self printInfo];
    NSPrintOperation* printOp = [NSPrintOperation printOperationWithView:[vc printableView] printInfo:printInfo];
    return printOp;
}

#pragma mark Attributes

// Shortcut to main window
- (NSWindow*)mainWindow {
    if([[self windowControllers] count] == 1) {
        return [[[self windowControllers] firstObject] window];
    }
    return nil;
}

#pragma mark Operations

// Add to configuration
- (void)addConfiguration:(NSDictionary*)config {
    [[self session] selectiveConfigure:config andUpdate:[self configuration]];
    [[self mainWindow] setDocumentEdited:YES];
}

// Add an authorized client
- (void)addAuthorizedClient:(NSDictionary*)code {
    [[self configuration][@"authorized_clients"] addObject:code];
    [[self session] selectiveConfigure:[self configuration] andUpdate:[self configuration]];
    [[self mainWindow] setDocumentEdited:YES];
}

// Plan seating for given number of players
- (void)planSeatingFor:(NSUInteger)maxPlayers {
    NSLog(@"Planning seating for %lu players", (unsigned long)maxPlayers);
    if(maxPlayers > 1) {
        [[self session] planSeatingFor:@(maxPlayers)];
    }
}

// Re-plan seating, clearing current game
- (void)planSeating {
    id players = [[self session] state][@"max_expected_players"];
    [self planSeatingFor:[players unsignedIntegerValue]];
}



@end
