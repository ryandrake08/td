//
//  TBTournamentsViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTournamentsViewController.h"
#import "TBAppDelegate.h"
#import "TournamentBrowser.h"
#import "TournamentSession.h"
#import "UIActionSheet+Blocks.h"
#import "UIAlertView+Blocks.h"
#import "UIResponder+PresentingErrors.h"

#import "NSObject+FBKVOController.h"

@interface TBTournamentsViewController () <UITableViewDelegate,
                                           UITableViewDataSource,
                                           TournamentBrowserDelegate>

@property (nonatomic, strong) TournamentSession* session;
@property (nonatomic, strong) IBOutlet TournamentBrowser* tournamentBrowser;
@property (nonatomic, strong) NSNetService* currentService;
@property (nonatomic, copy) NSArray* netServices;
@end

@implementation TBTournamentsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // initialize tournament browser
    [[self tournamentBrowser] search];

    // register for KVO
    [[self KVOController] observe:self keyPaths:@[@"session.state.connected", @"session.state.authorized"] options:0 block:^(id observer, TBTournamentsViewController* object, NSDictionary *change) {
        // update table view cell
        [observer reloadTableRowForService:[self currentService]];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc {
    // stop service browser
    [self setTournamentBrowser:nil];
}


#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return [[self netServices] count];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ServiceCell" forIndexPath:indexPath];

    NSNetService* cellService = [self netServices][[indexPath row]];
    BOOL connected = [[[self session] state][@"connected"] boolValue];
    BOOL authorized = [[[self session] state][@"authorized"] boolValue];

    // always set name
    [[cell textLabel] setText:[cellService name]];

    // set checkmark and accessory text if connected
    if(cellService == [self currentService] && connected) {
        if(authorized) {
            [[cell detailTextLabel] setText:NSLocalizedString(@"Admin", nil)];
        } else {
            [[cell detailTextLabel] setText:NSLocalizedString(@"Connected", nil)];
        }
        [cell setAccessoryType:UITableViewCellAccessoryCheckmark];
    } else {
        [[cell detailTextLabel] setText:@""];
        [cell setAccessoryType:UITableViewCellAccessoryNone];
    }

    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    NSNetService* cellService = [self netServices][[indexPath row]];
    BOOL connected = [[[self session] state][@"connected"] boolValue];
    BOOL authorized = [[[self session] state][@"authorized"] boolValue];

    if(cellService == [self currentService] && connected) {
        NSArray* otherButtons = nil;
        if(!authorized) {
            otherButtons = @[NSLocalizedString(@"Administer Game", nil)];
        }

        // pop actionsheet
        [UIActionSheet showInView:[self view]
                        withTitle:nil
                cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
           destructiveButtonTitle:NSLocalizedString(@"Leave Game", nil)
                otherButtonTitles:otherButtons
                         tapBlock:^(UIActionSheet * _Nonnull actionSheet, NSInteger buttonIndex) {
                             if(buttonIndex == [actionSheet destructiveButtonIndex]) {
                                 [[self session] disconnect];
                                 [self setCurrentService:nil];
                             } else if(buttonIndex == [actionSheet firstOtherButtonIndex]) {
                                 NSString* message = [NSString stringWithFormat:NSLocalizedString(@"The tournament director needs to authorize this code: %@", nil), [TournamentSession clientIdentifier]];

                                 // present alert
                                 [UIAlertView showWithTitle:NSLocalizedString(@"Administer Game", nil)
                                                    message:message
                                          cancelButtonTitle:NSLocalizedString(@"Cancel", nil)
                                          otherButtonTitles:@[NSLocalizedString(@"I'm Ready", nil)]
                                                   tapBlock:^(UIAlertView* alertView, NSInteger alertBbuttonIndex) {
                                                       if(alertBbuttonIndex == [alertView firstOtherButtonIndex]) {
                                                           // check authorization
                                                           [[self session] checkAuthorizedWithBlock:^(BOOL nowAuthorized) {
                                                               if(nowAuthorized) {
                                                                   // switch to seating screen automatically
                                                                   [[[self navigationController] tabBarController] setSelectedIndex:1];
                                                               }
                                                           }];
                                                       }
                                                   }];
                             }
                         }];
    } else {
        // connect
        NSError* error;
        if([[self session] connectToNetService:cellService error:&error]) {
            // store as current service
            [self setCurrentService:cellService];

            // switch to clock screen automatically
            [[[self navigationController] tabBarController] setSelectedIndex:2];

        } else {
            [self presentError:error];
        }
    }

    // deselect either way
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)reloadTableRowForService:(NSNetService*)service {
    NSUInteger i = [[self netServices] indexOfObject:service];
    if(i != NSNotFound) {
        NSIndexPath* indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [[self tableView] reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

#pragma mark TournamentBroswerDelegate

- (void)tournamentBrowser:(TournamentBrowser*)tournamentBroswer didUpdateServices:(NSArray*)services {
    // filter out all but real NSNetServices
    NSMutableArray* newArray = [[NSMutableArray alloc] init];
    for(TournamentService* ts in services) {
        if([ts netService] != nil) {
            [newArray addObject:[ts netService]];
        }
    }
    [self setNetServices:newArray];

    // if just one, automatically connect
    if([newArray count] == 1) {
        // connect
        NSError* error;
        if([[self session] connectToNetService:newArray[0] error:&error]) {
            // store as current service
            [self setCurrentService:newArray[0]];

            TBAppDelegate* appDelegate = (TBAppDelegate*)[[UIApplication sharedApplication] delegate];
            [(UITabBarController*)[[appDelegate window] rootViewController] setSelectedIndex:1];
        } else {
            [self presentError:error];
        }
    }
    // reload
    [[self tableView] reloadData];
}

@end
