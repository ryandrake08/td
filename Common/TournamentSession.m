//
//  TournamentSession.m
//  td
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentSession.h"
#import "TournamentConnection.h"
#import "NSString+CamelCase.h"
#import "TBCurrencyNumberFormatter.h"

@interface TournamentSession() <TournamentConnectionDelegate>

// record currently connected server
@property (nonatomic, strong) NSNetService* currentService;

// YES if currently authorized with server
@property (nonatomic, assign) BOOL authorized;

// the connection object, handles networking and JSON serialization
@property (nonatomic, strong) TournamentConnection* connection;

// mapping between unique command and block to handle the command's response
@property (nonatomic, strong) NSMutableDictionary* blocksForCommands;

// number formatter
@property (nonatomic, strong) NSNumberFormatter* decimalFormatter;

// tournament configuration
@property (nonatomic, strong) NSString* serverName;
@property (nonatomic, strong) NSString* serverVersion;
@property (nonatomic, strong) NSArray* authorizedClients;
@property (nonatomic, strong) NSString* name;
@property (nonatomic, strong) NSArray* players;
@property (nonatomic, strong) NSArray* blindLevels;
@property (nonatomic, strong) NSArray* availableChips;
@property (nonatomic, strong) NSString* costCurrency;
@property (nonatomic, strong) NSString* equityCurrency;
@property (nonatomic, strong) NSNumber* percentSeatsPaid;
@property (nonatomic, strong) NSNumber* roundPayouts;
@property (nonatomic, strong) NSNumber* payoutFlatness;
@property (nonatomic, strong) NSArray* fundingSources;
@property (nonatomic, strong) NSNumber* tableCapacity;
@property (nonatomic, strong) NSArray* manualPayouts;

// tournament state
@property (nonatomic, strong, getter=isRunning) NSNumber* running;
@property (nonatomic, strong) NSNumber* currentBlindLevel;
@property (nonatomic, strong) NSNumber* timeRemaining;
@property (nonatomic, strong) NSNumber* breakTimeRemaining;
@property (nonatomic, strong) NSNumber* actionClockTimeRemaining;
@property (nonatomic, strong) NSSet* buyins;
@property (nonatomic, strong) NSArray* entries;
@property (nonatomic, strong) NSArray* payouts;
@property (nonatomic, strong) NSNumber* totalChips;
@property (nonatomic, strong) NSNumber* totalCost;
@property (nonatomic, strong) NSNumber* totalCommission;
@property (nonatomic, strong) NSNumber* totalEquity;
@property (nonatomic, strong) NSArray* seats;
@property (nonatomic, strong) NSArray* playersFinished;
@property (nonatomic, strong) NSArray* emptySeats;
@property (nonatomic, strong) NSNumber* tables;
@property (nonatomic, strong) NSNumber* elapsedTime;

@end

@implementation TournamentSession

- (instancetype)init {
    if (self = [super init]) {
        _blocksForCommands = [[NSMutableDictionary alloc] init];
        _connection = [[TournamentConnection alloc] init];
        [_connection setDelegate:self];

        // Make formatter
        _decimalFormatter = [[NSNumberFormatter alloc] init];
        [[self decimalFormatter] setNumberStyle:NSNumberFormatterDecimalStyle];
    }
    return self;
}

- (void)connectToLocalPath:(NSString*)path {
    [self disconnect];
    [[self connection] connectToUnixSocketNamed:path];
    // TODO: handle error
}

- (void)connectToAddress:(NSString *)address port:(NSInteger)port {
    [self disconnect];
    [[self connection] connectToAddress:address andPort:port];
    // TODO: handle error
}

- (void)connectToService:(NSNetService*)service {
    [self disconnect];
    [self setCurrentService:service];
    [[self connection] connectToService:service];
}

- (void)connect:(TournamentService*)tournament {
    [self disconnect];
    [tournament connectTo:[self connection]];
}

- (void)disconnect {
    [self setAuthorized:NO];
    [[self connection] close];
    [self setCurrentService:nil];
}

- (BOOL) isConnected {
    return [[self connection] isConnected];
}

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString*)key {
    if([key isEqualToString:NSStringFromSelector(@selector(connection))]) {
        return NO;
    } else {
        return YES;
    }
}

// configure the session with configuration by sending only changed keys
- (void)selectiveConfigure:(NSDictionary*)config andUpdate:(NSMutableDictionary*)newConfig {
    NSLog(@"Synchronizing session");

    // only send parts of configuration that changed
    NSMutableDictionary* configToSend = [config mutableCopy];
    NSMutableArray* keysToRemove = [NSMutableArray array];
    [configToSend enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL* stop) {
        NSString* propertyName = [key asCamelCaseFromUnderscore];
        if([obj isEqual:[self valueForKey:propertyName]]) {
            [keysToRemove addObject:key];
        }
    }];
    [configToSend removeObjectsForKeys:keysToRemove];

    NSLog(@"Sending %ld configuration items", (long)[configToSend count]);

    if([configToSend count] > 0) {
        [self configure:configToSend withBlock:^(id json) {
            if(![json isEqual:newConfig]) {
                NSLog(@"Sent and received configurations differ");
                [newConfig setDictionary:json];
            }
        }];
    }
}

#pragma mark Internal routines

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier {
    NSString* key = @"clientIdentifier";
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    // if we don't already have one
    if([defaults objectForKey:key] == nil) {
        // generate a new identifier
        u_int32_t cid = arc4random() % 90000 + 10000;
        [defaults setObject:@(cid) forKey:key];
    }
    return [defaults objectForKey:key];
}

// uniquely identify each command sent, so we can associate it with a block handler
+ (NSNumber*)commandKey {
    static int incrementingKey = 0;
    return @(incrementingKey++);
}

#pragma mark Tournament Commands

// send a command through TournamentConnection
- (void)sendCommand:(NSString*)cmd withData:(NSDictionary*)arg andBlock:(void(^)(id,NSString*))block {
    // add extra stuff to each command
    NSMutableDictionary* json = [NSMutableDictionary dictionaryWithDictionary:arg];

    // append to every command: authentication
    NSNumber* cid = [TournamentSession clientIdentifier];
    json[@"authenticate"] = cid;

    // append to every command: command key
    NSNumber* cmdkey = [TournamentSession commandKey];
    json[@"echo"] = cmdkey;

    // add block & command key to our dictionary, if block exists
    if(block != nil) {
        [self blocksForCommands][cmdkey] = block;
    }

    // send it through connection
    [[self connection] sendCommand:cmd withData:json];
}

- (void)checkAuthorizedWithBlock:(void(^)(BOOL))block {
    [self sendCommand:@"check_authorized" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"checkAuthorizedWithBlock: %@\n", error);
        } else {
            // handle authorization check
            [self setAuthorized:[json[@"authorized"] boolValue]];
            if(block != nil) {
                block([json[@"authorized"] boolValue]);
            }
        }
    }];
}

- (void)getStateWithBlock:(void(^)(id))block {
    [self sendCommand:@"get_state" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"getStateWithBlock: %@\n", error);
        } else {
            // handle config response
            if(block != nil) {
                block(json);
            }
        }
    }];
}


- (void)getConfigWithBlock:(void(^)(id))block {
    [self sendCommand:@"get_config" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"getConfigWithBlock: %@\n", error);
        } else {
            // handle config response
            if(block != nil) {
                block(json);
            }
        }
    }];
}

- (void)configure:(id)config withBlock:(void(^)(id))block {
    [self sendCommand:@"configure" withData:config andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"configureWithBlock: %@\n", error);
        } else {
            // handle config response
            if(block != nil) {
                block(json);
            }
        }
    }];
}

- (void)startGameAt:(NSDate*)datetime {
    if(datetime) {
        [self sendCommand:@"start_game" withData:@{@"start_at" : datetime} andBlock:nil];
    } else {
        [self sendCommand:@"start_game" withData:nil andBlock:nil];
    }
}

- (void)stopGame {
    [self sendCommand:@"stop_game" withData:nil andBlock:nil];
}

- (void)resumeGame {
    [self sendCommand:@"resume_game" withData:nil andBlock:nil];
}

- (void)pauseGame {
    [self sendCommand:@"pause_game" withData:nil andBlock:nil];
}

- (void)togglePauseGame {
    [self sendCommand:@"toggle_pause_game" withData:nil andBlock:nil];
}

- (void)setPreviousLevelWithBlock:(void(^)(NSNumber*))block {
    [self sendCommand:@"set_previous_level" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"setPreviousLevelWithBlock: %@\n", error);
        } else {
            // handle blind level change
            if(block != nil) {
                block(json[@"blind_level_changed"]);
            }
        }
    }];
}

- (void)setNextLevelWithBlock:(void(^)(NSNumber*))block {
    [self sendCommand:@"set_next_level" withData:nil andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"setNextLevelWithBlock: %@\n", error);
        } else {
            // handle blind level change
            if(block != nil) {
                block(json[@"blind_level_changed"]);
            }
        }
    }];
}

- (void)setActionClock:(NSNumber*)milliseconds {
    if(milliseconds) {
        [self sendCommand:@"set_action_clock" withData:@{@"duration" : milliseconds} andBlock:nil];
    } else {
        [self sendCommand:@"set_action_clock" withData:nil andBlock:nil];
    }
}

- (void)genBlindLevels:(NSNumber*)count withDuration:(NSNumber*)durationMs breakDuration:(NSNumber*)breakDurationMs blindIncreaseFactor:(NSNumber*)increaseFactor {
    [self sendCommand:@"gen_blind_levels"withData:@{@"count" : count, @"duration" : durationMs, @"break_duration" : breakDurationMs, @"blind_increase_factor" : increaseFactor, } andBlock:nil];
}

- (void)fundPlayer:(id)playerId withFunding:(NSNumber*)sourceId {
    [self sendCommand:@"fund_player" withData:@{@"player_id" : playerId, @"source_id" : sourceId} andBlock:nil];
}

- (void)planSeatingFor:(NSNumber*)expectedPlayers {
    [self sendCommand:@"plan_seating" withData:@{@"max_expected_players" : expectedPlayers} andBlock:nil];
}

- (void)seatPlayer:(id)playerId withBlock:(void(^)(id,NSNumber*,NSNumber*))block {
    [self sendCommand:@"seat_player" withData:@{@"player_id" : playerId} andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"seatPlayerWithBlock: %@\n", error);
        } else {
            // handle seated player
            id playerSeated = json[@"player_seated"];
            if(playerSeated) {
                if(block != nil) {
                    block(playerSeated[@"player_id"], playerSeated[@"table_number"], playerSeated[@"seat_number"]);
                }
            } else {
                if(block != nil) {
                    block(nil, nil, nil);
                }
            }
        }
    }];
}

- (void)unseatPlayer:(id)playerId withBlock:(void(^)(id,NSNumber*,NSNumber*))block {
    [self sendCommand:@"unseat_player" withData:@{@"player_id" : playerId} andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"unseatPlayerWithBlock: %@\n", error);
        } else {
            // handle seated player
            id playerUnseated = json[@"player_unseated"];
            if(playerUnseated) {
                if(block != nil) {
                    block(playerUnseated[@"player_id"], playerUnseated[@"table_number"], playerUnseated[@"seat_number"]);
                }
            } else {
                if(block != nil) {
                    block(nil, nil, nil);
                }
            }
        }
    }];
}

- (void)bustPlayer:(id)playerId withBlock:(void(^)(NSArray*))block {
    [self sendCommand:@"bust_player" withData:@{@"player_id" : playerId} andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"bustPlayerWithBlock: %@\n", error);
        } else {
            // handle player movement
            // for now, just hand back the json
            // TODO: make this more sophisticated and populate a separate NSArray with objects
            if(block != nil) {
                block(json[@"players_moved"]);
            }
        }
    }];
}

#pragma mark Formatters

- (NSString*)formatBlindLevel:(NSDictionary*)level {
    NSNumber* bigBlind = level[@"big_blind"];
    NSNumber* littleBlind = level[@"little_blind"];
    NSNumber* ante = level[@"ante"];

    if([ante unsignedIntegerValue] > 0) {
        return [NSString localizedStringWithFormat:@"%@/%@ A:%@",
                [[self decimalFormatter] stringFromNumber:littleBlind],
                [[self decimalFormatter] stringFromNumber:bigBlind],
                [[self decimalFormatter] stringFromNumber:ante]];
    } else {
        return [NSString localizedStringWithFormat:@"%@/%@",
                [[self decimalFormatter] stringFromNumber:littleBlind],
                [[self decimalFormatter] stringFromNumber:bigBlind]];
    }
}

- (NSString*)formatDuration:(NSUInteger)duration {
    if(duration < 60000) {
        // SS.MSS
        unsigned long s = duration / 1000 % 60;
        unsigned long ms = duration % 1000;
        return [NSString stringWithFormat:@"%lu.%03lu", s, ms];
    } else if(duration < 3600000) {
        // MM:SS
        unsigned long m = duration / 60000;
        unsigned long s = duration / 1000 % 60;
        return [NSString stringWithFormat:@"%lu:%02lu", m, s];
    } else {
        // HH:MM:SS
        unsigned long h = duration / 3600000;
        unsigned long m = duration / 60000 % 60;
        unsigned long s = duration / 1000 % 60;
        return [NSString stringWithFormat:@"%lu:%02lu:%02lu", h, m, s];
    }
}

#pragma mark Derived Tournament State

+ (NSSet*)keyPathsForValuesAffectingValueForKey:(NSString*)key {

    NSSet *keyPaths = [super keyPathsForValuesAffectingValueForKey:key];

    if([key isEqualToString:@"planned"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"tables"]];
    } else if([key isEqualToString:@"onBreak"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"timeRemaining", @"breakTimeRemaining"]];
    } else if([key isEqualToString:@"clockText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"running", @"onBreak", @"timeRemaining", @"breakTimeRemaining"]];
    } else if([key isEqualToString:@"elapsedTimeText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"elapsedTime"]];
    } else if([key isEqualToString:@"currentRoundNumberText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"currentBlindLevel", @"blindLevels"]];
    } else if([key isEqualToString:@"currentGameText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"currentBlindLevel", @"blindLevels", @"onBreak"]];
    } else if([key isEqualToString:@"currentRoundText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"currentBlindLevel", @"blindLevels", @"onBreak"]];
    } else if([key isEqualToString:@"nextGameText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"currentBlindLevel", @"blindLevels"]];
    } else if([key isEqualToString:@"nextRoundText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"currentBlindLevel", @"blindLevels"]];
    } else if([key isEqualToString:@"playersLeftText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"seats"]];
    } else if([key isEqualToString:@"entriesText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"entries"]];
    } else if([key isEqualToString:@"averageStackText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"seats", @"totalChips"]];
    } else if([key isEqualToString:@"results"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"players", @"payouts", @"seats", @"playersFinished"]];
    } else if([key isEqualToString:@"playersLookup"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"players"]];
    } else if([key isEqualToString:@"seatedPlayers"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"seats",@"players",@"buyins"]];
    } else if([key isEqualToString:@"buyinText"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"fundingSources"]];
    } else if([key isEqualToString:@"costFormatter"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"costCurrency"]];
    } else if([key isEqualToString:@"equityFormatter"]) {
        keyPaths = [keyPaths setByAddingObjectsFromArray:@[@"equityCurrencyb"]];
    }

    return keyPaths;
}

- (BOOL)isPlanned {
    return [[self tables] intValue] > 0;
}

- (BOOL)isOnBreak {
    return [[self timeRemaining] unsignedIntegerValue] == 0 && [[self breakTimeRemaining] unsignedIntegerValue] != 0;
}

- (NSString*)clockText {
    NSUInteger timeRemaining = [[self timeRemaining] unsignedIntegerValue];
    NSUInteger breakTimeRemaining = [[self breakTimeRemaining] unsignedIntegerValue];

    // calculate new value
    NSString* newText = NSLocalizedString(@"PAUSED", nil);

    if([[self isRunning] boolValue]) {
        if([self isOnBreak]) {
            // on break
            newText = [self formatDuration:breakTimeRemaining];
        } else {
            newText = [self formatDuration:timeRemaining];
        }
    }

    return newText;
}

- (NSString*)elapsedTimeText {
    NSUInteger elapsedTime = [[self elapsedTime] unsignedIntegerValue];
    return [self formatDuration:elapsedTime];
}

- (NSString*)currentRoundNumberText {
    NSUInteger currentBlindLevel = [[self currentBlindLevel] unsignedIntegerValue];
    NSArray* blindLevels = [self blindLevels];

    // calculate new values
    NSString* newText = @"-";

    if(currentBlindLevel > 0 && currentBlindLevel < [blindLevels count]) {
        newText = [NSString stringWithFormat:@"%@", [self currentBlindLevel]];
    }

    return newText;
}

- (NSString*)currentGameText {
    NSUInteger currentBlindLevel = [[self currentBlindLevel] unsignedIntegerValue];
    NSArray* blindLevels = [self blindLevels];

    // calculate new values
    NSString* newText = @"";

    if(currentBlindLevel > 0 && currentBlindLevel < [blindLevels count]) {
        if(![self isOnBreak]) {
            newText = blindLevels[currentBlindLevel][@"game_name"];
        }
    }

    return newText;
}

- (NSString*)currentRoundText {
    NSUInteger currentBlindLevel = [[self currentBlindLevel] unsignedIntegerValue];
    NSArray* blindLevels = [self blindLevels];

    // calculate new values
    NSString* newText = @"-";

    if(currentBlindLevel > 0 && currentBlindLevel < [blindLevels count]) {
        if([self isOnBreak]) {
            newText = NSLocalizedString(@"BREAK", nil);
        } else {
            newText = [self formatBlindLevel:blindLevels[currentBlindLevel]];
        }
    }

    return newText;
}

- (NSString*)nextGameText {
    NSUInteger currentBlindLevel = [[self currentBlindLevel] unsignedIntegerValue];
    NSArray* blindLevels = [self blindLevels];

    // calculate new values
    NSString* newText = @"";

    if(currentBlindLevel > 0 && currentBlindLevel+1 < [blindLevels count]) {
        newText = blindLevels[currentBlindLevel+1][@"game_name"];
    }

    return newText;
}

- (NSString*)nextRoundText {
    NSUInteger currentBlindLevel = [[self currentBlindLevel] unsignedIntegerValue];
    NSArray* blindLevels = [self blindLevels];

    // calculate new values
    NSString* newText = @"-";

    if(currentBlindLevel > 0 && currentBlindLevel+1 < [blindLevels count]) {
        newText = [self formatBlindLevel:blindLevels[currentBlindLevel+1]];
    }

    return newText;
}

- (NSString*)playersLeftText {
    NSArray* seats = [self seats];

    // calculate new value
    NSString* newText = @"-";

    if([seats count] > 0) {
        newText = [[self decimalFormatter] stringFromNumber:@([seats count])];
    }

    return newText;
}

- (NSString*)entriesText {
    NSArray* entries = [self entries];

    // calculate new value
    NSString* newText = @"-";

    if([entries count] > 0) {
        newText = [[self decimalFormatter] stringFromNumber:@([entries count])];
    }

    return newText;
}

- (NSString*)averageStackText {
    NSArray* seats = [self seats];
    NSUInteger totalChips = [[self totalChips] unsignedIntegerValue];

    // calculate new value
    NSString* newText = @"-";

    if([seats count] > 0) {
        newText = [[self decimalFormatter] stringFromNumber:@(totalChips / [seats count])];
    }

    return newText;
}

- (NSString*)buyinText {
    NSArray* fundingSources = [self fundingSources];

    // calculate new value
    for(NSDictionary* obj in fundingSources) {
        if([obj[@"type"] isEqualToNumber:kFundingTypeBuyin]) {
            NSMutableString* newText = [NSMutableString stringWithString:obj[@"name"]];
            NSString* costText = [[self costFormatter] stringFromNumber:obj[@"cost"]];
            if(obj[@"commission"] && ![obj[@"commission"] isEqualToNumber:@0]) {
                NSString* commissionText = [[self costFormatter] stringFromNumber:obj[@"commission"]];
                [newText appendFormat:@": %@+%@", costText, commissionText];
            } else {
                [newText appendFormat:@": %@", costText];
            }
            return newText;
        }
    }

    return NSLocalizedString(@"No buyin", @"No buyin configured");
}

- (NSArray*)results {
    NSMutableArray* newResults = [[NSMutableArray alloc] init];

    // payouts with empty player field, for seated players
    for(NSUInteger j=0; j<[[self seats] count]; j++) {
        NSMutableDictionary* item = [NSMutableDictionary dictionaryWithObjectsAndKeys:@(j+1), @"place", @"--", @"name", nil];
        if([[self payouts] count] > j) {
            item[@"payout"] = [self payouts][j];
        }
        [newResults addObject:item];
    }

    // include actual player for busted players
    for(NSUInteger i=0; i<[[self playersFinished] count]; i++) {
        NSUInteger j = [[self seats] count]+i;
        NSNumber* finished = [self playersFinished][i];
        NSMutableDictionary* item = [NSMutableDictionary dictionaryWithObjectsAndKeys:@(j+1), @"place", [self playersLookup][finished][@"name"], @"name", nil];
        if([[self payouts] count] > j) {
            item[@"payout"] = [self payouts][j];
        }
        [newResults addObject:item];
    }

    return newResults;
}

- (NSArray*)seatedPlayers {
    NSMutableDictionary* newDict = [[NSMutableDictionary alloc] init];
    // first, add seated players
    for(id seat in [self seats]) {
        id playerId = seat[@"player_id"];
        if(playerId) {
            BOOL buyin = [[self buyins] containsObject:playerId];
            NSDictionary* player = [self playersLookup][playerId];
            if(player) {
                // Add the player record to the seat dictionary
                NSMutableDictionary* seatAndPlayer = [[NSMutableDictionary alloc] initWithDictionary:seat];
                seatAndPlayer[@"player"] = player;
                seatAndPlayer[@"buyin"] = @(buyin);
                newDict[playerId] = seatAndPlayer;
            } else {
                NSLog(@"Seated player %@ not in player array", seat);
            }
        }
    }

    // then, add any other players (not seated)
    for(id player in [self players]) {
        id playerId = player[@"player_id"];
        if(playerId) {
            BOOL buyin = [[self buyins] containsObject:playerId];
            NSDictionary* seat = newDict[playerId];
            if(seat == nil) {
                // not seated
                NSMutableDictionary* seatAndPlayer = [[NSMutableDictionary alloc] init];
                seatAndPlayer[@"player"] = player;
                seatAndPlayer[@"buyin"] = @(buyin);
                newDict[playerId] = seatAndPlayer;
            }
        }
    }
    return [newDict allValues];
}

- (NSDictionary*)playersLookup {
    NSMutableArray* keys = [NSMutableArray array];

    // all player_id
    for(NSDictionary* player in [self players]) {
        [keys addObject:player[@"player_id"]];
    }

    // create the player_id -> player lookup table
    id dict = [NSDictionary dictionaryWithObjects:[self players] forKeys:keys];

    return dict;
}

#pragma mark Number Formatters

- (NSNumberFormatter*)costFormatter {
    TBCurrencyNumberFormatter* formatter = [[TBCurrencyNumberFormatter alloc] init];
    [formatter setNumberStyle:NSNumberFormatterCurrencyStyle];
    [formatter setMinimumFractionDigits:0];
    [formatter setCurrencyCode:[self costCurrency]];
    return formatter;
}

- (NSNumberFormatter*)equityFormatter {
    TBCurrencyNumberFormatter* formatter = [[TBCurrencyNumberFormatter alloc] init];
    [formatter setNumberStyle:NSNumberFormatterCurrencyStyle];
    [formatter setMinimumFractionDigits:0];
    [formatter setCurrencyCode:[self equityCurrency]];
    return formatter;
}

#pragma mark Tournament Messages

- (void)handleMessage:(id)json fromConnection:(TournamentConnection*)tc {
    // look for command key
    NSNumber* cmdkey = json[@"echo"];
    if(cmdkey) {
        // remove command key response
        [json removeObjectForKey:@"echo"];

        // look up block for command key
        void (^block)(id,NSString*) = [self blocksForCommands][cmdkey];
        if(block) {
            // if it's a command with a handler block, call that block
            block(json, json[@"error"]);

            // remove it from our dictionary
            [[self blocksForCommands] removeObjectForKey:cmdkey];
        }
    } else {
        // any configuration or state to update, split into calls to set properties
        [json enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
            @try {
                if(![obj isEqual:[self valueForKey:[key asCamelCaseFromUnderscore]]]) {
                    [self setValue:obj forKey:[key asCamelCaseFromUnderscore]];
                }
            }
            @catch (NSException* exception) {
                NSLog(@"Session has no key %@", key);
            }
        }];
    }
}

#pragma mark TournamentConnectionDelegate

- (void)tournamentConnectionDidConnect:(TournamentConnection*)tc {
    NSAssert([self connection] == tc, @"Unexpected connection from %@", tc);
    [self willChangeValueForKey:NSStringFromSelector(@selector(isConnected))];
    [self didChangeValueForKey:NSStringFromSelector(@selector(isConnected))];

    // always check if we're authorized right away
    [self checkAuthorizedWithBlock:nil];
}

- (void)tournamentConnectionDidDisconnect:(TournamentConnection*)tc {
    NSAssert([self connection] == tc, @"Unexpected disconnection from %@", tc);
    [self disconnect];
}

- (void)tournamentConnectionDidClose:(TournamentConnection*)tc {
    NSAssert([self connection] == tc, @"Unexpected close from %@", tc);
    [self willChangeValueForKey:NSStringFromSelector(@selector(isConnected))];
    [self didChangeValueForKey:NSStringFromSelector(@selector(isConnected))];
}

- (void)tournamentConnection:(TournamentConnection*)tc didReceiveData:(id)json {
    NSAssert([self connection] == tc, @"Unexpected data from %@", tc);
    [self handleMessage:json fromConnection:tc];
}

- (void)tournamentConnection:(TournamentConnection*)tc error:(NSError*)error {
    NSAssert([self connection] == tc, @"Unexpected error from %@", tc);
    [self disconnect];
}

@end
