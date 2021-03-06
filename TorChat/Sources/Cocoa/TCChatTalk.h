/*
 *  TCChatTalk.h
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
** Forward
*/
#pragma mark - Forward

@class TCChatPage;



/*
** Types
*/
#pragma mark - Types

// == Local or remote buddy ==
typedef enum
{
	tcchat_local,
	tcchat_remote
} tcchat_user;



/*
** TCChatTalk
*/
#pragma mark - TCChatTalk

// == Class ==
@interface TCChatTalk : NSScrollView

// -- Actions --
- (void)addStatusMessage:(NSString *)msg fromUserName:(NSString *)userName;
- (void)addErrorMessage:(NSString *)msg;
- (void)appendToConversation:(NSString *)text fromUser:(tcchat_user)user;
- (void)addTimeStamp;

- (void)setLocalAvatar:(NSImage *)image;
- (void)setRemoteAvatar:(NSImage *)image;

@end
