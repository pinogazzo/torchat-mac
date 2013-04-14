/*
 *  TCController.h
 *
 *  Copyright 2010 Avérous Julien-Pierre
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



#ifndef _TCController_H_
# define _TCController_H_

# include <dispatch/dispatch.h>
# include <vector>

# include "TCInfo.h"
# include "TCObject.h"



/*
** Forward
*/
#pragma mark -
#pragma mark Forward

class TCBuddy;
class TCConfig;
class TCControlClient;
class TCController;



/*
** Types
*/
#pragma mark -
#pragma mark Types

// == Controller Status ==
typedef enum
{
	tccontroller_available,
	tccontroller_away,
	tccontroller_xa
} tccontroller_status;

// == Info Code ==
typedef enum
{
	// -- Notify --
	tcctrl_notify_started,
	tcctrl_notify_stoped,
	
	tcctrl_notify_buddy_new,
	
	tcctrl_notify_client_new,
	tcctrl_notify_client_started,
	tcctrl_notify_client_stoped,
	
	// -- Errors --
	tcctrl_error_serv_socket,
	tcctrl_error_serv_accept,
	
	tcctrl_error_socket,
	
	tcctrl_error_client_read,
	tcctrl_error_client_read_closed,
	tcctrl_error_client_read_full,
	
	tcctrl_error_client_unknown_command,
	
	tcctrl_error_client_cmd_ping,
	tcctrl_error_client_cmd_pong,
	tcctrl_error_client_cmd_status,
	tcctrl_error_client_cmd_version,
	tcctrl_error_client_cmd_message,
	tcctrl_error_client_cmd_addme,
	tcctrl_error_client_cmd_removeme,
	tcctrl_error_client_cmd_filename,
	tcctrl_error_client_cmd_filedata,
	tcctrl_error_client_cmd_filedataok,
	tcctrl_error_client_cmd_filedataerror,
	tcctrl_error_client_cmd_filestopsending,
	tcctrl_error_client_cmd_filestopreceiving,
} tcctrl_info;

// == Controller Notify Block ==
typedef void (^tcctrl_event)(TCController *controller, const TCInfo *info);



/*
** TCController
*/
#pragma mark -
#pragma mark TCController

class TCController : public TCObject
{
public:
	// -- Constructor & Destructor --
	TCController(TCConfig *config);
	~TCController();
	
	// -- Runing --
	void					start();
	void					stop();
	
	// -- Delegate --
	void					setDelegate(dispatch_queue_t queue, tcctrl_event event);
	
	// -- Status --
	void					setStatus(tccontroller_status status);
	tccontroller_status		status();
	
	// -- Buddies --
	void					addBuddy(const std::string &name, const std::string &address);
	void					addBuddy(const std::string &name, const std::string &address, const std::string &comment);
	void					removeBuddy(const std::string &address);
    TCBuddy *				getBuddyAddress(const std::string &address);
	TCBuddy *				getBuddyRandom(const std::string &random);
	
	// -- TCControlClient --
	void					cc_error(TCControlClient *client, TCInfo *info);
	void					cc_notify(TCControlClient *client, TCInfo *info);

private:
	
	// -- Tools --
	void					_addClient(int sock);
	
	// -- Helpers --
	void					_error(tcctrl_info code, const std::string &info, bool fatal);
	void					_error(tcctrl_info code, const std::string &info, TCObject *ctx, bool fatal);
	
	void					_notify(tcctrl_info notice, const std::string &info);
	void					_notify(tcctrl_info notice, const std::string &info, TCObject *ctx);
	
	void					_send_event(TCInfo *info);


	
	// -- Vars --
	// > Main Queue
	dispatch_queue_t		mainQueue;
	
	// > Timer
	dispatch_source_t		timer;
	
	// > Accept Socket
	dispatch_queue_t		socketQueue;
	dispatch_source_t		socketAccept;
	int						sock;
	
	// > Buddies
	bool					buddiesLoaded;
	std::vector<TCBuddy *>	buddies;
	
	// > Config
	TCConfig				*config;
	
	// > Clients
	std::vector<TCControlClient *> clients;
	
	// > Status
	bool					running;
	tccontroller_status		mstatus;
	
	// > Delegate
	dispatch_queue_t		nQueue;
	tcctrl_event			nBlock;
	
	// -- Friends --
	//friend class			TCControlClient;
};

#endif