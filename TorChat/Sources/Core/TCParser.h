/*
 *  TCParser.h
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



#ifndef _TCParser_H_
# define _TCParser_H_

# include <string>
# include <vector>



/*
** Forward
*/
#pragma mark - Forward

class TCInfo;



/*
** TCParser
*/
#pragma mark - TCParser

// == Types ==
typedef enum
{
	tcrec_unknown_command,
	tcrec_cmd_ping,
	tcrec_cmd_pong,
	tcrec_cmd_status,
	tcrec_cmd_version,
	tcrec_cmd_client,
	tcrec_cmd_profile_text,
	tcrec_cmd_profile_name,
	tcrec_cmd_profile_avatar,
	tcrec_cmd_profile_avatar_alpha,
	tcrec_cmd_message,
	tcrec_cmd_addme,
	tcrec_cmd_removeme,
	tcrec_cmd_filename,
	tcrec_cmd_filedata,
	tcrec_cmd_filedataok,
	tcrec_cmd_filedataerror,
	tcrec_cmd_filestopsending,
	tcrec_cmd_filestopreceiving
} tcrec_error;

// == Class ==
class TCParser
{
public:
	// -- Parsing --
	void			parseLine(const std::string &line);

	// -- Commands --
	virtual void	doPing(const std::string &address, const std::string &random);
	virtual void	doPong(const std::string &random);
	virtual void	doStatus(const std::string &status);
	virtual void	doMessage(const std::string &message);
	virtual void	doVersion(const std::string &version);
	virtual void	doClient(const std::string &client);
	virtual void	doProfileText(const std::string &text);
	virtual void	doProfileName(const std::string &name);
	virtual void	doProfileAvatar(const std::string &bitmap);
	virtual void	doProfileAvatarAlpha(const std::string &bitmap);
	virtual void	doAddMe();
	virtual void	doRemoveMe();
	virtual void	doFileName(const std::string &uuid, const std::string &fsize, const std::string &bsize, const std::string &filename);
	virtual void	doFileData(const std::string &uuid, const std::string &start, const std::string &hash, const std::string &data);
	virtual void	doFileDataOk(const std::string &uuid, const std::string &start);
	virtual void	doFileDataError(const std::string &uuid, const std::string &start);
	virtual void	doFileStopSending(const std::string &uuid);
	virtual void	doFileStopReceiving(const std::string &uuid);
	
	// -- Error --
	virtual void	parserError(TCInfo *info);
	
private:
	
	// -- Parser --
    void _parseCommand(std::vector<std::string> &items);
    
	void _parsePing(const std::vector<std::string> &args);
    void _parsePong(const std::vector<std::string> &args);
    void _parseStatus(const std::vector<std::string> &args);
    void _parseVersion(const std::vector<std::string> &args);
	void _parseClient(const std::vector<std::string> &args);
	void _parseProfileText(const std::vector<std::string> &args);
	void _parseProfileName(const std::vector<std::string> &args);
	void _parseProfileAvatar(const std::vector<std::string> &args);
	void _parseProfileAvatarAlpha(const std::vector<std::string> &args);
	void _parseMessage(const std::vector<std::string> &args);
	void _parseAddMe(const std::vector<std::string> &args);
	void _parseRemoveMe(const std::vector<std::string> &args);
	void _parseFileName(const std::vector<std::string> &args);
	void _parseFileData(const std::vector<std::string> &args);
	void _parseFileDataOk(const std::vector<std::string> &args);
	void _parseFileDataError(const std::vector<std::string> &args);
	void _parseFileStopSending(const std::vector<std::string> &args);
	void _parseFileStopReceiving(const std::vector<std::string> &args);
	
	void _parserError(tcrec_error error, const char *info);
};

#endif
