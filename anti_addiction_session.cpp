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
#include "anti_addiction_session.h"
#include "../common/ServerDefine/anti_addiction_server_define.h"
#include "../common/ServerDefine/base_server_define.h"
#include "si_login_server.h"
#include "user_mgr.h"
#include "anti_addiction_mgr.h"
#include "database.h"

anti_addiction_session g_anti_addiction_session;

BOOL anti_addiction_session::init()
{
	
	if( !init_config() )
	{
		return FALSE;
	}


	p_transport = new few_connect_client;
	if( !VALID_POINT(p_transport) ) return FALSE;
	p_transport->init();

	if(!login_server::p_thread->create_thread(_T("AntiAddictionSessionConnectServer"), &anti_addiction_session::static_thread_connect_server, this))
	{
		return FALSE;
	}

	while(!login_server::p_thread->is_thread_active(_T("AntiAddictionSessionConnectServer")))
	{
		continue;
	}

	return TRUE;
}

VOID anti_addiction_session::destroy()
{

	Interlocked_Exchange((LPLONG)&b_terminate_connect, TRUE);
	login_server::p_thread->waitfor_thread_destroy( _T("AntiAddictionSessionConnectServer"), INFINITE);


	p_transport->destory();
	SAFE_DELETE(p_transport);
	
	
}

UINT anti_addiction_session::thread_connect_server()
{
//#ifdef _DEBUG
	THROW_EXCEPTION_START;
//#endif

	while( FALSE == b_terminate_connect )
	{
		if( !p_transport->is_connect() )
		{
			if( !p_transport->is_trying_create_connect() )
			{
				p_transport->try_create_connect(_sz_ip, n_port);
			}

			Sleep(100);
			continue;	
		}

		print_message(_T("Contected to AntiAddiction server at %s: %d\r\n"), get_tool()->unicode8_to_unicode(_sz_ip), n_port);


		tagNC_LOGIN_CM send;
		send.dw_section_id = g_login.get_section_id();
		p_transport->send_msg(&send, send.dw_size);

		break;
	}

//#ifdef _DEBUG
	THROW_EXCEPTION_END;
//#endif
	_endthreadex(0);

	return 0;

}

UINT anti_addiction_session::static_thread_connect_server(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	anti_addiction_session* p_this = (anti_addiction_session*)p_data;
	return p_this->thread_connect_server();
}

BOOL anti_addiction_session::init_config()
{

	file_container* p_var = new file_container;
	if( !VALID_POINT(p_var) ) return FALSE;
	
	TCHAR t_sz_path[MAX_PATH];
	ZeroMemory(t_sz_path, sizeof(t_sz_path));

	if (!get_file_io_mgr()->get_ini_path(t_sz_path, _T("server_config/login/center_proof"))||
		!p_var->load(g_login.get_file_system(), t_sz_path))
	{
		ERROR_CLUE_ON(_T("配置文件未找到"));
		return FALSE;
	}

	//p_var->load(g_login.get_file_system(), _T("server_config/login/center_proof.ini"));

	//! 读取端口和IP
	TCHAR sz_ip[X_IP_LEN];
	_tcsncpy(sz_ip, p_var->get_string(_T("ip anti_addiction_server")), cal_tchar_array_num(sz_ip) - 1);
	memcpy(_sz_ip, get_tool()->unicode_to_unicode8(sz_ip), sizeof(_sz_ip) - 1);

	n_port = p_var->get_int(_T("port anti_addiction_server"));

	SAFE_DELETE(p_var);

	return TRUE;
}

VOID anti_addiction_session::update()
{
	if( NULL == p_transport )
		return;

	if(!p_transport->is_connect() && !login_server::p_thread->is_thread_active(_T("AntiAddictionSessionConnectServer")))
	{
		Interlocked_Exchange((LONG*)&b_terminate_connect, TRUE);
		Interlocked_Exchange((LONG*)&b_connected, FALSE);
		p_transport->disconnect();

		login_server::p_thread->waitfor_thread_destroy(_T("AntiAddictionSessionConnectServer"), INFINITE);

		Interlocked_Exchange((LONG*)&b_terminate_connect, FALSE);
		login_server::p_thread->create_thread(_T("AntiAddictionSessionConnectServer"), &anti_addiction_session::static_thread_connect_server, this);

		while(FALSE == login_server::p_thread->is_thread_active(_T("AntiAddictionSessionConnectServer")))
		{
			continue;
		}

		return;
	}

	//! 处理中心服务器的信息
	DWORD	dw_size = 0;
	LPBYTE	p_receive = p_transport->recv_msg(dw_size);

	while( VALID_POINT(p_receive) )
	{
		tag_net_message* p_cmd = (tag_net_message*)p_receive;
		if(p_cmd->dw_message_id == get_tool()->crc32("NS_ANTIENTHRALL"))
			handle_guard_account_notify(p_receive);

		p_transport->free_recv_msg(p_receive);

	
		p_receive = p_transport->recv_msg(dw_size);
	}
}

VOID anti_addiction_session::send_query_status(DWORD dw_account)
{
	const s_account_data* p_account_data = g_PlayMgr.get_cached_account_data(dw_account);
	if (VALID_POINT(p_account_data))
	{
		tagNC_ANTIQUERYSTATE send;
		memcpy(send.sz_account_name, p_account_data->sz_account_name, 16);
		p_transport->send_msg(&send, send.dw_size);
	}
}

VOID anti_addiction_session::send_online_guard_accounts( DWORD *p_ol_accounts, DWORD dw_num )
{
	if (dw_num < 0)
		return;

	if (dw_num == 0)
	{
		tagNC_ANTIENTHRALL send;
		send.dw_account_num = 0;
		p_transport->send_msg(&send, send.dw_size);

		return;
	}

	DWORD dwSize = sizeof(tagNC_ANTIENTHRALL) + dw_num * sizeof(tag_Antienthrall) - sizeof(tag_Antienthrall);

	BYTE* p_byte = new BYTE[dwSize];
	tagNC_ANTIENTHRALL* pSend = (tagNC_ANTIENTHRALL*)p_byte;
	pSend->dw_message_id = get_tool()->crc32("NC_ANTIENTHRALL");
	pSend->dw_size = dwSize;

	pSend->dw_account_num = 0;
	for (int i=0; i<(INT)dw_num; ++i)
	{
		const s_account_data* p_account_data = g_PlayMgr.get_cached_account_data(p_ol_accounts[i]);
		if (!VALID_POINT(p_account_data))
		{
			continue;
		}

		//LPSTR szName = pSend->sz_account_name + 16 * pSend->dw_account_num;
		memcpy(pSend->st_Antienthrall[i].sz_account_name, p_account_data->sz_account_name, sizeof(CHAR) * 16);
		pSend->st_Antienthrall[i].b_guard = p_account_data->b_guard;
		++pSend->dw_account_num;
	}

	p_transport->send_msg(pSend, pSend->dw_size);

	SAFE_DELETE_ARRAY(pSend);
}

DWORD anti_addiction_session::handle_guard_account_notify( PVOID p_msg )
{
	M_trans_pointer(p_receive, p_msg, tagNS_ANTIENTHRALL);

	s_anti_addiction_notify Notify;
	Notify.dw_account_id = g_PlayMgr.get_account_id_by_account_name(p_receive->sz_account_name);
	Notify.dw_state = (DWORD)p_receive->by_color - 1;
	Notify.dw_account_online_time_min = p_receive->dw_account_online_seconds / 60;

	g_anti_mgr.notify_player(&Notify);

	return 0;
}

