/*
 *  TCPrefController.mm
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



#import "TCPrefController.h"

#import "TCMainController.h"
#import "TCBuddiesController.h"

#include "TCConfig.h"



/*
** Prototypes
*/
#pragma mark - Prototypes

NSString *	TCStringWithCPPString(const std::string &str);
std::string	TCCPPStringWithString(NSString *str);
NSString *	TCStringWithInt(int value);



/*
** TCPrefController - Private
*/
#pragma mark - TCPrefController - Private

@interface TCPrefController ()
{
	TCPrefView	*currentView;
}

- (void)loadViewIdentifier:(NSString *)identifier animated:(BOOL)animated;

@end



/*
** TCPrefView - Private
*/
#pragma mark - TCPrefView - Private

@interface TCPrefView ()

@property (assign, nonatomic) TCConfig *config;

- (void)loadConfig;
- (void)saveConfig;

@end



/*
** TCPrefView_Network - Private
*/
#pragma mark - TCPrefView_Network - Private

@interface TCPrefView_Network ()
{
	BOOL changes;
}

@end



/*
** TCPrefController
*/
#pragma mark - TCPrefController

@implementation TCPrefController

@synthesize mainWindow;

@synthesize generalView;
@synthesize networkView;
@synthesize buddiesView;



/*
** TCPrefController - Constructor & Destructor
*/
#pragma mark - TCPrefController - Constructor & Destructor

+ (TCPrefController *)sharedController
{
	static dispatch_once_t	pred;
	static TCPrefController	*instance = nil;
	
	dispatch_once(&pred, ^{
		instance = [[TCPrefController alloc] init];
	});
	
	return instance;
}

- (id)init
{
	self = [super init];
	
    if (self)
	{
        // Load the nib
		[NSBundle loadNibNamed:@"PreferencesWindow" owner:self];
    }
    
    return self;
}

- (void)dealloc
{
    // Clean-up code here.
    
    [super dealloc];
}

- (void)awakeFromNib
{	
	// Place Window
	[mainWindow center];
	[mainWindow setFrameAutosaveName:@"PreferencesWindow"];
	
	// Select the default view
	[self loadViewIdentifier:@"general" animated:NO];
}



/*
** TCPrefController - Tools
*/
#pragma mark - TCPrefController - Tools

- (void)showWindow
{
	// Show window
	[mainWindow makeKeyAndOrderFront:self];
}

- (void)loadViewIdentifier:(NSString *)identifier animated:(BOOL)animated
{
	TCPrefView	*view = nil;
	TCConfig	*config = [[TCMainController sharedController] config];

	if ([identifier isEqualToString:@"general"])
		view = generalView;
	else if ([identifier isEqualToString:@"network"])
		view = networkView;
	else if ([identifier isEqualToString:@"buddies"])
		view = buddiesView;
	
	if (!view)
		return;
	
	// Check if the toolbar item is well selected
	if ([[[mainWindow toolbar] selectedItemIdentifier] isEqualToString:identifier] == NO)
		[[mainWindow toolbar] setSelectedItemIdentifier:identifier];
	
	// Save current view config
	currentView.config = config;
	[currentView saveConfig];
	
	// Load new view config
	view.config = config;
	[view loadConfig];
		
	// Load view
	if (animated)
	{
		NSRect	rect = [mainWindow frame];
		NSSize	csize = [[mainWindow contentView] frame].size;
		NSSize	size = [view frame].size;
		CGFloat	previous = rect.size.height;
		
		rect.size.width = (rect.size.width - csize.width) + size.width;
		rect.size.height = (rect.size.height - csize.height) + size.height;
				
		rect.origin.y += (previous - rect.size.height);
		
		[NSAnimationContext beginGrouping];
		{
			[[NSAnimationContext currentContext] setDuration:0.125];
			
			[[[mainWindow contentView] animator] replaceSubview:currentView with:view];
			[[mainWindow animator] setFrame:rect display:YES];
		}
		[NSAnimationContext endGrouping];
	}
	else
	{
		[currentView removeFromSuperview];
		[[mainWindow contentView] addSubview:view];
	}
	
	// Hold the current view
	[view retain];
	[currentView release];
	
	currentView = view;
}



/*
** TCPrefController - IBAction
*/
#pragma mark - TCPrefController - IBAction

- (IBAction)doToolbarItem:(id)sender
{
	NSToolbarItem	*item = sender;
	NSString		*identifier = [item itemIdentifier];
	
	[self loadViewIdentifier:identifier animated:YES];
}



/*
** TCPrefController - NSWindow
*/
#pragma mark - TCPrefController - NSWindow

- (void)windowWillClose:(NSNotification *)notification
{
	TCConfig *config = [[TCMainController sharedController] config];

	currentView.config = config;
	
	[currentView saveConfig];
}

@end



/*
** TCPrefView
*/
#pragma mark - TCPrefView

@implementation TCPrefView


/*
** TCPrefView - Properties
*/
#pragma mark - TCPrefView - Properties

@synthesize config;

- (void)setConfig:(TCConfig *)aconfig
{
	if (aconfig)
		aconfig->retain();
	
	if (config)
		config->release();
	
	config = aconfig;
}



/*
** TCPrefView - Instance
*/
#pragma mark - TCPrefView - Instance

- (void)dealloc
{
    if (config)
		config->release();
	
    [super dealloc];
}



/*
** TCPrefView - Config
*/
#pragma mark - TCPrefView - Config

- (void)loadConfig
{
	// Must be redefined
}

- (void)saveConfig
{
	// Must be redefined
}

@end



/*
** TCPrefView_General
*/
#pragma mark - TCPrefView_General

@implementation TCPrefView_General

@synthesize downloadField;

@synthesize clientNameField;
@synthesize clientVersionField;


/*
** TCPrefView_General - IBAction
*/
#pragma mark - TCPrefView_General - IBAction

- (IBAction)doDownload:(id)sender
{	
	NSOpenPanel	*openDlg = [NSOpenPanel openPanel];
	
	// Ask for a file
	[openDlg setCanChooseFiles:NO];
	[openDlg setCanChooseDirectories:YES];
	[openDlg setCanCreateDirectories:YES];
	[openDlg setAllowsMultipleSelection:NO];
	
	if ([openDlg runModal] == NSOKButton)
	{
		NSArray		*urls = [openDlg URLs];
		NSURL		*url = [urls objectAtIndex:0];
		
		if (self.config)
		{
			[downloadField setStringValue:[[url path] lastPathComponent]];	
			
			self.config->set_download_folder([[url path] UTF8String]);
		}
		else
			NSBeep();
	}
}



/*
** TCPrefView_General - Config
*/
#pragma mark - TCPrefView_General - IBAction

- (void)loadConfig
{
	if (!self.config)
		return;
		
	[downloadField setStringValue:[TCStringWithCPPString(self.config->get_download_folder()) lastPathComponent]];
	
	[[clientNameField cell] setPlaceholderString:TCStringWithCPPString(self.config->get_client_name(tc_config_get_default))];
	[[clientVersionField cell] setPlaceholderString:TCStringWithCPPString(self.config->get_client_version(tc_config_get_default))];

	[clientNameField setStringValue:TCStringWithCPPString(self.config->get_client_name(tc_config_get_defined))];
	[clientVersionField setStringValue:TCStringWithCPPString(self.config->get_client_version(tc_config_get_defined))];
}

- (void)saveConfig
{
	self.config->set_client_name(TCCPPStringWithString([clientNameField stringValue]));
	self.config->set_client_version(TCCPPStringWithString([clientVersionField stringValue]));
}

@end



/*
** TCPrefView_Network
*/
#pragma mark - TCPrefView_Network

@implementation TCPrefView_Network

@synthesize imAddressField;
@synthesize imPortField;

@synthesize torAddressField;
@synthesize torPortField;



/*
** TCPrefView_Network - TextField Delegate
*/
#pragma mark - TCPrefView_Network - TextField Delegate

- (void)controlTextDidChange:(NSNotification *)aNotification
{
	changes = YES;
}



/*
** TCPrefView_Network - Config
*/
#pragma mark - TCPrefView_Network - IBAction

- (void)loadConfig
{
	tc_config_mode mode;
	
	if (!self.config)
		return;
		
	mode = self.config->get_mode();
	
	// Set mode
	if (mode == tc_config_basic)
	{		
		[imAddressField setEnabled:NO];
		[imPortField setEnabled:NO];
		[torAddressField setEnabled:NO];
		[torPortField setEnabled:NO];
	}
	else if (mode == tc_config_advanced)
	{		
		[imAddressField setEnabled:YES];
		[imPortField setEnabled:YES];
		[torAddressField setEnabled:YES];
		[torPortField setEnabled:YES];
	}
	
	// Set value field
	[imAddressField setStringValue:TCStringWithCPPString(self.config->get_self_address())];
	[imPortField setStringValue:TCStringWithInt(self.config->get_client_port())];
	[torAddressField setStringValue:TCStringWithCPPString(self.config->get_tor_address())];
	[torPortField setStringValue:TCStringWithInt(self.config->get_tor_port())];
}

- (void)saveConfig
{	
	 if (!self.config)
		 return;
	 
	if (self.config->get_mode() == tc_config_advanced)
	{
		// Set config value
		self.config->set_self_address(TCCPPStringWithString([imAddressField stringValue]));
		self.config->set_client_port((uint16_t)[[imPortField stringValue] intValue]);
		self.config->set_tor_address(TCCPPStringWithString([torAddressField stringValue]));
		self.config->set_tor_port((uint16_t)[[torPortField stringValue] intValue]);
		
		// Reload config
		if (changes)
		{		
			[[TCBuddiesController sharedController] stop];
			[[TCBuddiesController sharedController] startWithConfig:self.config];
			
			changes = NO;
		}
	 }
}

@end



/*
** TCPrefView_Buddies
*/
#pragma mark - TCPrefView_Buddies

@implementation TCPrefView_Buddies

@synthesize tableView;
@synthesize removeButton;

@synthesize addBlockedWindow;
@synthesize addBlockedField;


/*
** TCPrefView_Buddies - Config
*/
#pragma mark - TCPrefView_Buddies - Config

- (void)loadConfig
{
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(buddyBlockedChanged:) name:TCCocoaBuddyChangedBlockedNotification object:nil];

}

- (void)saveConfig
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}



/*
** TCPrefView_Buddies - IBAction
*/
#pragma mark - TCPrefView_Buddies - IBAction

- (IBAction)doAddBlockedUser:(id)sender
{
	if (!self.config)
		return;
	
	// Show add window
	[addBlockedField setStringValue:@""];
	
	[[NSApplication sharedApplication] beginSheet:addBlockedWindow modalForWindow:self.window modalDelegate:nil didEndSelector:NULL contextInfo:NULL];
}


- (IBAction)doAddBlockedCancel:(id)sender
{
	// Close
	[[NSApplication sharedApplication] endSheet:addBlockedWindow];
	[addBlockedWindow orderOut:self];
}

- (IBAction)doAddBlockedOK:(id)sender
{
	NSString	*address;
	
	if (!self.config)
		return;
	
	address = [addBlockedField stringValue];
	
	// Add on controller
	// XXX Here, we break the fact that config is local to this view,
	//	and it will not necessarily the one used by the controller in the future.
	//	Try to find a better solution.
	if ([[TCBuddiesController sharedController] addBlockedBuddy:address] == NO)
	{
		NSBeep();
	}
	else
	{
		// Reload
		[tableView reloadData];
		
		// Close
		[[NSApplication sharedApplication] endSheet:addBlockedWindow];
		[addBlockedWindow orderOut:self];
	}
}

- (IBAction)doRemoveBlockedUser:(id)sender
{
	if (!self.config)
		return;
	
	const tc_sarray	&blocked = self.config->blocked_buddies();
	NSIndexSet		*set = [tableView selectedRowIndexes];
	NSMutableArray	*removes = [NSMutableArray arrayWithCapacity:[set count]];
	NSUInteger		index = [set firstIndex];
	
	// Resolve indexes
	while (index != NSNotFound)
	{
		const char	*caddress = blocked.at(index).c_str();
		NSString	*address;
		
		if (!caddress)
			continue;
		
		// Add to address to remove
		address = [[NSString alloc] initWithUTF8String:caddress];
		
		[removes addObject:address];
		
		[address release];
		
		// Next index
		index = [set indexGreaterThanIndex:index];
	}
	
	// Remove
	for (NSString *remove in removes)
	{
		// Remove on controller
		// XXX Here, we break the fact that config is local to this view,
		//	and it will not necessarily the one used by the controller in the future.
		//	Try to find a better solution.
		[[TCBuddiesController sharedController] removeBlockedBuddy:remove];
	}
	
	// Reload list
	[tableView reloadData];
}

- (void)buddyBlockedChanged:(NSNotification *)notice
{
	// Reload list
	[tableView reloadData];
}



/*
** TCPrefView_Buddies - TableView Delegate
*/
#pragma mark - TCPrefView_Buddies - TableView Delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	if (!self.config)
		return 0;
	
	return (NSInteger)(self.config->blocked_buddies().size());
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{	
	if (!self.config)
		return nil;
	
	const tc_sarray &blocked = self.config->blocked_buddies();
	
	if (rowIndex < 0 || rowIndex >= blocked.size())
		return nil;
	
	return [NSString stringWithUTF8String:blocked.at((size_t)rowIndex).c_str()];
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	NSIndexSet *set = [tableView selectedRowIndexes];
	
	if ([set count] > 0)
		[removeButton setEnabled:YES];
	else
		[removeButton setEnabled:NO];
}

- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	return NO;
}

@end



/*
** C Tools
*/
#pragma mark - C Tools

NSString *	TCStringWithCPPString(const std::string &str)
{
	const char *cstr = str.c_str();
	
	if (!cstr)
		return nil;
	
	return [NSString stringWithUTF8String:cstr];
}

std::string	TCCPPStringWithString(NSString *str)
{
	const char *cstr = [str UTF8String];
	
	if (!str)
		return "";
	
	return cstr;
}

NSString *	TCStringWithInt(int value)
{
	return [NSString stringWithFormat:@"%i", value];
}
