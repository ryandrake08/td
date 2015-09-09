//
//  TBSettingsViewController.m
//  td
//
//  Created by Ryan Drake on 9/9/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSettingsViewController.h"
#import "TournamentSession.h"
#import "TBAppDelegate.h"

@interface TBSettingsViewController () <UITableViewDelegate, UITableViewDataSource>

@property (nonatomic, strong) TournamentSession* session;

@end

@implementation TBSettingsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark UITableViewDataSource

- (NSString*)tableView:(UITableView*)tableView titleForHeaderInSection:(NSInteger)section {
    NSString* sectionName;
    if(section == 0) {
        sectionName = NSLocalizedString(@"Tournament Setup", nil);
    } else if(section == 1) {
        sectionName = NSLocalizedString(@"Seat Planning", nil);
    }
    return sectionName;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 2;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    if(section == 0) {
        return 6;
    } else if(section == 1) {
        return 2;
    } else {
        return 0;
    }
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell;
    NSString* text;
    NSString* detail;

    if(indexPath.section == 0) {
        // create a cell
        cell = [tableView dequeueReusableCellWithIdentifier:@"SetupCell" forIndexPath:indexPath];
        switch(indexPath.row) {
            case 0:
                text = NSLocalizedString(@"Name", nil);
                detail = [[self session] state][@"name"];
                break;
            case 1:
                text = NSLocalizedString(@"Players", nil);
                detail = [[self session] state][@"name"];
                break;
            case 2:
                text = NSLocalizedString(@"Equipment", nil);
                detail = [[self session] state][@"name"];
                break;
            case 3:
                text = NSLocalizedString(@"Funding", nil);
                detail = [[self session] state][@"name"];
                break;
            case 4:
                text = NSLocalizedString(@"Rounds", nil);
                detail = [[self session] state][@"name"];
                break;
            case 5:
                text = NSLocalizedString(@"Devices", nil);
                detail = [[self session] state][@"name"];
                break;
        }
    } else if(indexPath.section == 1) {
        cell = [tableView dequeueReusableCellWithIdentifier:@"SetupCell" forIndexPath:indexPath];
        switch(indexPath.row) {
            case 0:
                text = NSLocalizedString(@"Seating For", nil);
                detail = [[self session] state][@"name"];
                break;
            case 1:
                text = NSLocalizedString(@"Re-plan", nil);
                detail = nil;
                break;
        }
    }

    [[cell textLabel] setText:text];
    [[cell detailTextLabel] setText:detail];
    return cell;
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
