/*
 *  TCLogsController.h
 *
 *  Copyright 2012 Avérous Julien-Pierre
 *
 *  This file is part of TorChat.
 *
 *  TorChat is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TorChat is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TorChat.  If not, see <http://www.gnu.org/licenses/>.
 *
 */



#import <Cocoa/Cocoa.h>



/*
** TCLogsController
*/
#pragma mark - TCLogsController

@interface TCLogsController : NSObject

@property (assign) IBOutlet NSWindow	*mainWindow;
@property (assign) IBOutlet NSTableView	*entriesView;
@property (assign) IBOutlet NSTableView	*logsView;

// -- Singleton --
+ (TCLogsController *)sharedController;

// -- Interface --
- (IBAction)showWindow:(id)sender;

// -- Tools --
- (void)addBuddyLogEntryFromAddress:(NSString *)address alias:(NSString *)alias andText:(NSString *)log, ...;
- (void)addGlobalLogEntry:(NSString *)log, ...;
- (void)addGlobalAlertLog:(NSString *)log, ...;

// -- Observer --
- (void)setObserver:(id)object withSelector:(SEL)selector forKey:(NSString *)key;
- (void)removeObserverForKey:(NSString *)key;


@end
