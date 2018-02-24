/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "StdAfx.h"
#include "user_net_cmd_mgr.h"


user_net_cmd_mgr::user_net_cmd_mgr()
{
	map_proc.clear();
}


user_net_cmd_mgr::~user_net_cmd_mgr()
{
	//destroy();
}


VOID user_net_cmd_mgr::destroy()
{
}


BOOL user_net_cmd_mgr::register_cmd(LPCSTR sz_cmd, NETMSGHANDLER fp, LPCTSTR sz_desc)
{
	DWORD dw_id = get_tool()->crc32(sz_cmd);

	s_user_cmd* p_cmd = map_proc.find(dw_id);

	if( VALID_POINT(p_cmd) )
	{
		if( p_cmd->str_cmd != sz_cmd )
		{
			ASSERT(0);	
			return FALSE;
		}
	}
	else
	{
		p_cmd = new s_user_cmd;
		p_cmd->n_times = 0;
		p_cmd->handler = fp;
		p_cmd->str_cmd = sz_cmd;
		p_cmd->str_desc = sz_desc;
		map_proc.add(dw_id, p_cmd);
	}

	return TRUE;
}


VOID user_net_cmd_mgr::unregister_all()
{
	map_proc.reset_iterator();
	s_user_cmd* p_cmd = NULL;
	while( map_proc.find_next(p_cmd) )
	{
		SAFE_DELETE(p_cmd);
	}

	map_proc.clear();
}


NETMSGHANDLER user_net_cmd_mgr::get_handler(tag_net_message* p_msg, UINT32 n_msg_size)
{
	s_user_cmd* p_cmd = map_proc.find(p_msg->dw_message_id);
	if( !VALID_POINT(p_cmd) )
	{
		print_message(_T("Unknow player command recved[<cmdid>%d <size>%d]\r\n"), p_msg->dw_message_id, n_msg_size);
		return NULL;
	}

	if( p_msg->dw_size != n_msg_size || n_msg_size > GET_MAX_PACKAGE_LENGTH )
	{
		print_message(_T("Invalid net command size[<cmd>%u <size>%d]\r\n"), p_msg->dw_message_id, p_msg->dw_size);
		return NULL;
	}

	++p_cmd->n_times;

	return p_cmd->handler;
}


BOOL user_net_cmd_mgr::handle_cmd(tag_net_message* p_msg, DWORD n_msg_size, user* p_session)
{
	if( !VALID_POINT(p_session) ) return FALSE;

	NETMSGHANDLER fp = get_handler(p_msg, n_msg_size);
	if( NULL == fp ) return FALSE;

	(p_session->*fp)(p_msg);

	return TRUE;
}
