/*
 *  TCControlClient.cpp
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



#include <errno.h>

#include "TCControlClient.h"

#include "TCTools.h"
#include "TCBuddy.h"
#include "TCConfig.h"
#include "TCString.h"



/*
** TCControlClient - Instance
*/
#pragma mark - TCControlClient - Instance

TCControlClient::TCControlClient(TCConfig *_conf, int _sock)
{
	// Hold config
	_conf->retain();
	config = _conf;
	
	ctrl = NULL;
	
	// Build queue
	mainQueue = dispatch_queue_create("com.torchat.core.controllclient.main", DISPATCH_QUEUE_SERIAL);

	
	// Init vars
	running = false;
	
	// Hold socket
	sockd = _sock;
	sock = NULL;
}

TCControlClient::~TCControlClient()
{
	TCDebugLog("TCControlClient Destructor");
	
	if (ctrl)
		ctrl->release();
	
	config->release();
	
	if (sock)
	{
		sock->stop();
		sock->release();
	}
		
	// Release Queue
	dispatch_release(mainQueue);
}



/*
** TCControlClient - Running
*/
#pragma mark - TCControlClient - Running


void TCControlClient::start(TCController *controller)
{
	if (!controller)
		return;
	
	controller->retain();
	
	dispatch_async_cpp(this, mainQueue, ^{
		
		if (!running && sockd > 0)
		{
			ctrl = controller;
			running = true;
			
			// Build a socket
			sock = new TCSocket(sockd);
			
			sock->setDelegate(mainQueue, this);
			sock->scheduleOperation(tcsocket_op_line, 1, 0);
			
			// Notify
			_notify(tcctrl_notify_client_started, "core_cctrl_note_started");
		}
		else
			controller->release();
	});
}

void TCControlClient::stop()
{
	dispatch_async_cpp(this, mainQueue, ^{
		
		if (!running)
			return;
		
		running = false;
		
		// Clean socket
		if (sock)
		{
			sock->stop();
			sock->release();
			
			sock = NULL;
		}
		
		// Clean socket descriptor
		sockd = -1;

		// Notify
		_notify(tcctrl_notify_client_stoped, "core_cctrl_note_stoped");
		
		// Release controller
		ctrl->release();
		ctrl = NULL;
	});
}



/*
** TCControlClient(TCParser) - Overwrite
*/
#pragma mark - TCControlClient(TCParser) - Overwrite

// == Handle Ping ==
void TCControlClient::doPing(const std::string &caddress, const std::string &crandom)
{
	TCBuddy *abuddy = NULL;

	// Reschedule a line read
	sock->scheduleOperation(tcsocket_op_line, 1, 0);
	
	// Check blocked list
	abuddy = ctrl->getBuddyAddress(caddress);
	
	if (abuddy)
	{
		bool blocked = abuddy->blocked();

		abuddy->release();
		
		if (blocked)
			return;
	}

	// first a little security check to detect mass pings
	// with faked host names over the same connection
	
	if (last_ping_address.size() != 0)
	{
		if (caddress.compare(last_ping_address) != 0)
		{
			// DEBUG
			fprintf(stderr, "(1) Possible Attack: in-connection sent fake address '%s'\n", caddress.c_str());
			fprintf(stderr, "(1) Will disconnect incoming connection from fake '%s'\n", caddress.c_str());
			
			// Notify
			_error(tcctrl_error_client_cmd_ping, "core_cctrl_err_fake_ping", true);
			return;
		}
	}
	else
		last_ping_address = caddress;
	
	
	// another check for faked pings: we search all our already
	// *connected* buddies and if there is one with the same address
	// but another incoming connection then this one must be a fake.
	
	if (ctrl)
		abuddy = ctrl->getBuddyAddress(caddress);
	
	if (abuddy && abuddy->isPonged())
	{
		_error(tcctrl_error_client_cmd_ping, "core_cctrl_err_already_pinged", true);
		abuddy->release();
		
		return;
	}
	
	
	
	// if someone is pinging us with our own address and the
	// random value is not from us, then someone is definitely 
	// trying to fake and we can close.
	
	if (caddress.compare(config->get_self_address()) == 0 && abuddy && abuddy->brandom().content().compare(crandom) != 0)
	{
		_error(tcctrl_error_client_cmd_ping, "core_cctrl_err_masquerade", true);
		abuddy->release();
		
		return;
	}
	
	
	// if the buddy don't exist, add it on the buddy list
	if (!abuddy)
	{
		if (ctrl)
			ctrl->addBuddy(config->localized("core_cctrl_new_buddy"), caddress);
		
		abuddy = ctrl->getBuddyAddress(caddress);
		
		if (!abuddy)
		{
			_error(tcctrl_error_client_cmd_ping, "core_cctrl_err_add_buddy", true);
			return;
		}
	}
	
	
	// ping messages must be answered with pong messages
	// the pong must contain the same random string as the ping.
	TCImage		*avatar = ctrl->profileAvatar();
	TCString	*trandom = new TCString(crandom);
	TCString	*pname = ctrl->profileName();
	TCString	*ptext = ctrl->profileText();
	
	abuddy->startHandshake(trandom, ctrl->status(), avatar, pname, ptext);
	
	// Release
	abuddy->release();
	avatar->release();
	trandom->release();
	pname->release();
	ptext->release();
}

// == Handle Pong ==
void TCControlClient::doPong(const std::string &crandom)
{
	TCBuddy *buddy = NULL;
	
	if (ctrl)
		buddy = ctrl->getBuddyRandom(crandom);
	
	if (buddy)
	{
		// Check blocked list
		if (buddy->blocked())
		{
			// Stop buffy
			buddy->stop();
			buddy->release();
			buddy = NULL;
			
			// Stop socket
			sock->stop();
			sock->release();
			sock = NULL;
		}
		else
		{
			// Give the baby to buddy
			buddy->setInputConnection(sock);
			
			// Release buddy (getBuddyRandom retained it)
			buddy->release();
			buddy = NULL;
			
			// Unhandle socket
			sock->release();
			sock = NULL;
		}
	}
	else
		_error(tcctrl_error_client_cmd_pong, "core_cctrl_err_pong", true);
}

// == Parsing Error ==
void TCControlClient::parserError(tcrec_error err, const std::string &info)
{	
	tcctrl_info nerr = tcctrl_error_client_unknown_command;
	
	// Convert parser error to controller errors
	switch (err)
	{
		case tcrec_unknown_command:
			nerr = tcctrl_error_client_unknown_command;
			break;
			
		case tcrec_cmd_ping:
			nerr = tcctrl_error_client_cmd_ping;
			break;
			
		case tcrec_cmd_pong:
			nerr = tcctrl_error_client_cmd_pong;
			break;
			
		case tcrec_cmd_status:
			nerr = tcctrl_error_client_cmd_status;
			break;
			
		case tcrec_cmd_version:
			nerr = tcctrl_error_client_cmd_version;
			break;
			
		case tcrec_cmd_client:
			nerr = tcctrl_error_client_cmd_client;
			break;
			
		case tcrec_cmd_profile_text:
			nerr = tcctrl_error_client_cmd_profile_text;
			break;
			
		case tcrec_cmd_profile_name:
			nerr = tcctrl_error_client_cmd_profile_name;
			break;
			
		case tcrec_cmd_profile_avatar:
			nerr = tcctrl_error_client_cmd_profile_avatar;
			break;
			
		case tcrec_cmd_profile_avatar_alpha:
			nerr = tcctrl_error_client_cmd_profile_avatar_alpha;
			break;
			
		case tcrec_cmd_message:
			nerr = tcctrl_error_client_cmd_message;
			break;
			
		case tcrec_cmd_addme:
			nerr = tcctrl_error_client_cmd_addme;
			break;
			
		case tcrec_cmd_removeme:
			nerr = tcctrl_error_client_cmd_removeme;
			break;
			
		case tcrec_cmd_filename:
			nerr = tcctrl_error_client_cmd_filename;
			break;
			
		case tcrec_cmd_filedata:
			nerr = tcctrl_error_client_cmd_filedata;
			break;

		case tcrec_cmd_filedataok:
			nerr = tcctrl_error_client_cmd_filedataok;
			break;
			
		case tcrec_cmd_filedataerror:
			nerr = tcctrl_error_client_cmd_filedataerror;
			break;
			
		case tcrec_cmd_filestopsending:
			nerr = tcctrl_error_client_cmd_filestopsending;
			break;
			
		case tcrec_cmd_filestopreceiving:
			nerr = tcctrl_error_client_cmd_filestopreceiving;
			break;
	}
	
	// Parse error is fatal
	_error(nerr, info, true);
}



/*
** TCSocket - Delegate
*/
#pragma mark - TCSocket - Delegate

void TCControlClient::socketOperationAvailable(TCSocket *socket, tcsocket_operation operation, int tag, void *content, size_t size)
{
	if (operation == tcsocket_op_line)
	{
		std::vector <std::string *> *vect = static_cast< std::vector <std::string *> * > (content);
		size_t						i, cnt = vect->size();
				
		for (i = 0; i < cnt; i++)
		{
			std::string *line = vect->at(i);
			
			dispatch_async_cpp(this, mainQueue, ^{
				
				// Parse the line
				parseLine(*line);
				
				// Free memory
				delete line;
			});
		}
		
		// Clean
		delete vect;
	}
}

void TCControlClient::socketError(TCSocket *socket, TCInfo *err)
{
	// Localize the info
	err->setInfo(config->localized(err->info()));
	
	// Fallback Error
	_error(tcctrl_error_socket, "core_cctrl_err_socket", err, true);
}



/*
** TCSocket - Helpers
*/
#pragma mark - TCSocket - Helpers

void TCControlClient::_error(tcctrl_info code, const std::string &info, bool fatal)
{
	TCInfo *err = new TCInfo(tcinfo_error, code, config->localized(info));
	
	if (ctrl)
		ctrl->cc_error(this, err);
	
	err->release();
	
	if (fatal)
		stop();
}

void TCControlClient::_error(tcctrl_info code, const std::string &info, TCObject *ctx, bool fatal)
{
	TCInfo *err = new TCInfo(tcinfo_error, code, config->localized(info), ctx);
	
	if (ctrl)
		ctrl->cc_error(this, err);
	
	err->release();
	
	if (fatal)
		stop();
}

void TCControlClient::_error(tcctrl_info code, const std::string &info, TCInfo *serr, bool fatal)
{
	TCInfo *err = new TCInfo(tcinfo_error, code, config->localized(info), serr);
	
	if (ctrl)
		ctrl->cc_error(this, err);
	
	err->release();
	
	if (fatal)
		stop();
}

void TCControlClient::_notify(tcctrl_info notice, const std::string &info)
{
	TCInfo *ifo = new TCInfo(tcinfo_info, notice, config->localized(info));
	
	if (ctrl)
		ctrl->cc_notify(this, ifo);
	
	ifo->release();
}


bool TCControlClient::_isBlocked(const std::string &address)
{
	if (!config)
		return false;
	
	// XXX not thread safe
	const tc_sarray &blocked = config->blocked_buddies();
	size_t			i, cnt = blocked.size();
	
	for (i = 0; i < cnt; i++)
	{
		const std::string &item = blocked.at(i);
		
		if (item.compare(address) == 0)
			return true;
	}
	
	return false;
}
