//
//  TournamentSession.m
//  TournamentKit
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TournamentSession.h"
#import "TournamentConnection.h"

@interface TournamentSession() <TournamentConnectionDelegate>

// record currently connected server
@property (nonatomic) NSNetService* currentService;

// YES if currently authorized with server
@property (nonatomic, assign) BOOL authorized;

// the connection object, handles networking and JSON serialization
@property (nonatomic) TournamentConnection* connection;

// mapping between unique command and block to handle the command's response
@property (nonatomic) NSMutableDictionary* blocksForCommands;

// number formatter
@property (nonatomic) NSNumberFormatter* decimalFormatter;

// tournament configuration
@property (nonatomic) NSString* name;
@property (nonatomic) NSMutableArray* players;
@property (nonatomic) NSArray* blindLevels;
@property (nonatomic) NSArray* availableChips;
@property (nonatomic) NSString* costCurrency;
@property (nonatomic) NSString* equityCurrency;
@property (nonatomic) NSNumber* percentSeatsPaid;
@property (nonatomic) NSNumber* roundPayouts;
@property (nonatomic) NSArray* fundingSources;
@property (nonatomic) NSNumber* tableCapacity;

// tournament state
@property (nonatomic, getter=isRunning) NSNumber* running;
@property (nonatomic) NSNumber* currentBlindLevel;
@property (nonatomic) NSNumber* timeRemaining;
@property (nonatomic) NSNumber* breakTimeRemaining;
@property (nonatomic) NSNumber* actionClockTimeRemaining;
@property (nonatomic) NSSet* buyins;
@property (nonatomic) NSArray* payouts;
@property (nonatomic) NSNumber* totalChips;
@property (nonatomic) NSNumber* totalCost;
@property (nonatomic) NSNumber* totalCommission;
@property (nonatomic) NSNumber* totalEquity;
@property (nonatomic) NSArray* seats;
@property (nonatomic) NSArray* playersFinished;
@property (nonatomic) NSArray* emptySeats;
@property (nonatomic) NSNumber* tables;

// derived tournament state
@property (nonatomic) NSString* clockText;
@property (nonatomic) NSString* currentGameText;
@property (nonatomic) NSString* currentRoundText;
@property (nonatomic) NSString* nextGameText;
@property (nonatomic) NSString* nextRoundText;
@property (nonatomic) NSString* playersLeftText;
@property (nonatomic) NSString* entriesText;
@property (nonatomic) NSString* averageStackText;

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

#pragma mark Serialization

// get current configuration
- (id)currentConfiguration {
    NSMutableDictionary* json = [NSMutableDictionary dictionary];

    if([self name]) {
        json[@"name"] = [self name];
    }

    if([self players]) {
        json[@"players"] = [self players];
    }

    if([self blindLevels]) {
        json[@"blind_levels"] = [self blindLevels];
    }

    if([self availableChips]) {
        json[@"available_chips"] = [self availableChips];
    }

    if([self costCurrency]) {
        json[@"cost_currency"] = [self costCurrency];
    }

    if([self equityCurrency]) {
        json[@"equity_currency"] = [self equityCurrency];
    }

    if([self percentSeatsPaid]) {
        json[@"percent_seats_paid"] = [self percentSeatsPaid];
    }

    if([self roundPayouts]) {
        json[@"round_payouts"] = [self roundPayouts];
    }

    if([self fundingSources]) {
        json[@"funding_sources"] = [self fundingSources];
    }

    if([self tableCapacity]) {
        json[@"table_capacity"] = [self tableCapacity];
    }

    return json;
}

#pragma mark Internal routines

// client identifier (used for authenticating with servers)
+ (NSNumber*)clientIdentifier {
#if defined(DEBUG)
    return @31337;
#else
    NSString* key = @"clientIdentifier";
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    // if we don't already have one
    if([defaults objectForKey:key] == nil) {
        // generate a new identifier
        u_int32_t cid = arc4random() % 90000 + 10000;
        [defaults setObject:[NSNumber numberWithInteger:cid] forKey:key];
    }
    return [defaults objectForKey:key];
#endif
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

- (void)authorize:(NSNumber*)clientId withBlock:(void(^)(NSNumber*))block {
    [self sendCommand:@"authorize" withData:@{@"authorize" : clientId} andBlock:^(id json, NSString* error) {
        if(error != nil) {
            NSLog(@"authorizeWithBlock: %@\n", error);
        } else {
            // handle client authorization
            if(block != nil) {
                block(json[@"authorized_client"]);
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

- (void)fundPlayer:(NSNumber*)playerId withFunding:(NSNumber*)sourceId {
    [self sendCommand:@"fund_player" withData:@{@"player_id" : playerId, @"source_id" : sourceId} andBlock:nil];
}

- (void)planSeatingFor:(NSNumber*)expectedPlayers {
    [self sendCommand:@"plan_seating" withData:@{@"max_expected_players" : expectedPlayers} andBlock:nil];
}

- (void)seatPlayer:(NSNumber*)playerId withBlock:(void(^)(NSNumber*,NSNumber*,NSNumber*))block {
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

- (void)unseatPlayer:(NSNumber*)playerId {
    [self sendCommand:@"unseat_player" withData:@{@"player_id" : playerId} andBlock:nil];
}

- (void)bustPlayer:(NSNumber*)playerId withBlock:(void(^)(NSArray*))block {
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

- (void)updateClock {
    BOOL running = [[self isRunning] boolValue];
    NSUInteger timeRemaining = [[self timeRemaining] unsignedIntegerValue];
    NSUInteger breakTimeRemaining = [[self breakTimeRemaining] unsignedIntegerValue];

    if(running) {
        if(timeRemaining == 0 && breakTimeRemaining != 0) {
            // on break
            [self setClockText:[self formatDuration:breakTimeRemaining]];
        } else {
            [self setClockText:[self formatDuration:timeRemaining]];
        }
    } else {
        [self setClockText:NSLocalizedString(@"PAUSED", nil)];
    }
}

- (void)updateBlinds {
    NSUInteger currentBlindLevel = [[self currentBlindLevel] unsignedIntegerValue];
    NSArray* blindLevels = [self blindLevels];
    NSUInteger timeRemaining = [[self timeRemaining] unsignedIntegerValue];
    NSUInteger breakTimeRemaining = [[self breakTimeRemaining] unsignedIntegerValue];

    if(currentBlindLevel == 0) {
        [self setCurrentGameText:@""];
        [self setCurrentRoundText:NSLocalizedString(@"PLANNING", nil)];
        [self setNextGameText:@""];
        [self setNextRoundText:NSLocalizedString(@"-", nil)];
    } else {
        if(timeRemaining == 0 && breakTimeRemaining != 0) {
            [self setCurrentGameText:@""];
            [self setCurrentRoundText:NSLocalizedString(@"BREAK", nil)];
        } else if(currentBlindLevel < [blindLevels count]) {
            NSDictionary* thisBlindLevel = blindLevels[currentBlindLevel];
            [self setCurrentGameText:thisBlindLevel[@"game_name"]];
            [self setCurrentRoundText:[self formatBlindLevel:thisBlindLevel]];
        }

        if(currentBlindLevel+1 < [blindLevels count]) {
            NSDictionary* nextBlindLevel = blindLevels[currentBlindLevel+1];
            [self setNextGameText:nextBlindLevel[@"game_name"]];
            [self setNextRoundText:[self formatBlindLevel:nextBlindLevel]];
        }
    }
}

- (void)updatePlayers {
    NSArray* seats = [self seats];

    if([seats count] > 0) {
        NSNumber* numSeats = [NSNumber numberWithUnsignedInteger:[seats count]];
        NSString* numSeatsText = [[self decimalFormatter] stringFromNumber:numSeats];
        [self setPlayersLeftText:numSeatsText];
    } else {
        [self setPlayersLeftText:@"-"];
    }
}

- (void)updateEntries {
    NSArray* entries = [self entries];

    if([entries count] > 0) {
        NSNumber* numEntries = [NSNumber numberWithUnsignedInteger:[entries count]];
        NSString* numEntriesText = [[self decimalFormatter] stringFromNumber:numEntries];
        [self setEntriesText:numEntriesText];
    } else {
        [self setEntriesText:@"-"];
    }
}

- (void)updateAverageStack {
    NSArray* seats = [self seats];
    NSUInteger totalChips = [[self totalChips] unsignedIntegerValue];

    if([seats count] > 0) {
        NSNumber* avgChips = [NSNumber numberWithUnsignedInteger:totalChips / [seats count]];
        NSString* avgChipsText = [[self decimalFormatter] stringFromNumber:avgChips];
        [self setAverageStackText:avgChipsText];
    } else {
        [self setAverageStackText:@"-"];
    }
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
    }

    // any configuration or state to update, split into calls to set properties
    id value;

    // tournament configuration
    if((value = json[@"name"])) {
        [self setName:value];
    }

    if((value = json[@"players"])) {
        [self setPlayers:value];
    }

    if((value = json[@"blind_levels"])) {
        [self setBlindLevels:value];
        [self updateBlinds];
    }

    if((value = json[@"available_chips"])) {
        [self setAvailableChips:value];
    }

    if((value = json[@"cost_currency"])) {
        [self setCostCurrency:value];
    }

    if((value = json[@"equity_currency"])) {
        [self setEquityCurrency:value];
    }

    if((value = json[@"percent_seats_paid"])) {
        [self setPercentSeatsPaid:value];
    }

    if((value = json[@"round_payouts"])) {
        [self setRoundPayouts:value];
    }

    if((value = json[@"funding_sources"])) {
        [self setFundingSources:value];
    }

    if((value = json[@"table_capacity"])) {
        [self setTableCapacity:value];
    }

    // tournament state
    if((value = json[@"running"])) {
        [self setRunning:value];
        [self updateClock];
        [self updateBlinds];
    }

    if((value = json[@"current_blind_level"])) {
        [self setCurrentBlindLevel:value];
        [self updateBlinds];
    }

    if((value = json[@"time_remaining"])) {
        [self setTimeRemaining:value];
        [self updateClock];
        [self updateBlinds];
    }

    if((value = json[@"break_time_remaining"])) {
        [self setBreakTimeRemaining:value];
        [self updateClock];
        [self updateBlinds];
    }

    if((value = json[@"action_clock_remaining"])) {
        [self setActionClockTimeRemaining:value];
    }

    if((value = json[@"buyins"])) {
        [self setBuyins:value];
    }

    if((value = json[@"payouts"])) {
        [self setPayouts:value];
    }

    if((value = json[@"total_chips"])) {
        [self setTotalChips:value];
        [self updateAverageStack];
    }

    if((value = json[@"total_cost"])) {
        [self setTotalCost:value];
    }

    if((value = json[@"total_commission"])) {
        [self setTotalCommission:value];
    }

    if((value = json[@"total_equity"])) {
        [self setTotalEquity:value];
    }

    if((value = json[@"seats"])) {
        [self setSeats:value];
        [self updatePlayers];
        [self updateAverageStack];
    }

    if((value = json[@"entries"])) {
        [self updateEntries];
    }

    if((value = json[@"players_finished"])) {
        [self setPlayersFinished:value];
    }

    if((value = json[@"empty_seats"])) {
        [self setEmptySeats:value];
    }

    if((value = json[@"tables"])) {
        [self setTables:value];
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
