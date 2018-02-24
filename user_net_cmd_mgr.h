/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _USER_NET_CMD_MGR_H_
#define _USER_NET_CMD_MGR_H_


//-----------------------------------------------------------------------------
//! ���������
//-----------------------------------------------------------------------------

class user;

typedef DWORD (user::*NETMSGHANDLER)(tag_net_message*);

class user_net_cmd_mgr
{
public:
	user_net_cmd_mgr();
	~user_net_cmd_mgr();

	VOID destroy();

	BOOL register_cmd(LPCSTR sz_cmd, NETMSGHANDLER fp, LPCTSTR sz_desc);
	VOID unregister_all();

	NETMSGHANDLER get_handler(tag_net_message* p_msg, UINT32 n_msg_size);

	BOOL handle_cmd(tag_net_message* p_msg, DWORD n_msg_size, user* p_session);
protected:
	struct s_user_cmd
	{
		std::string				str_cmd;	//! ������
		tstring					str_desc;	//! ����
		NETMSGHANDLER			handler;	//! ����ָ��
		UINT32					n_times;	//! �յ�������Ĵ���
	};

	package_map<DWORD, s_user_cmd*>	map_proc;
};

#endif