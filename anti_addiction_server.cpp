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
#include "anti_addiction_server.h"
#include "../common/ServerDefine/anti_addiction_server_define.h"
#include "database.h"
#include "si_login_server.h"

anti_addiction_server g_anti_server;

VOID anti_addiction_timer::updata()
{
	anti_addiction_info_map::map_iter itr = map_all_fatigue.begin();
	s_anti_addiction_info* p_fatigue_info = NULL;
	// 对所有防沉迷玩家
	while(map_all_fatigue.find_next(itr, p_fatigue_info))
	{
		if (!VALID_POINT(p_fatigue_info))
			break;

		p_fatigue_info->Update();

		if (p_fatigue_info->need_save())
		{
			g_datebase.save_fatigue_info(p_fatigue_info);
			p_fatigue_info->set_saved();
		}

		if (p_fatigue_info->need_notify())
		{
			notify_ol_guard_users(p_fatigue_info);
		}

	}

}

VOID anti_addiction_timer::reset()
{
	anti_addiction_info_map::map_iter itr = map_all_fatigue.begin();
	s_anti_addiction_info* p_fatigue_info = NULL;
	//! 对所有防沉迷玩家
	while(map_all_fatigue.find_next(itr, p_fatigue_info))
	{
		if (!VALID_POINT(p_fatigue_info))
			break;

		p_fatigue_info->b_last_online = p_fatigue_info->b_online;
		p_fatigue_info->b_online = FALSE;

	}

}


VOID anti_addiction_timer::record_ol_guard_accounts( tagNC_ANTIENTHRALL* p_ol_accounts )
{

	for (INT i=0; i<(INT)p_ol_accounts->dw_account_num; ++i)
	{
		if(!p_ol_accounts->st_Antienthrall[i].b_guard)
			continue;

		LPSTR sz_account_name = p_ol_accounts->st_Antienthrall[i].sz_account_name/* + i * 16 * sizeof(CHAR)*/;
		DWORD dw_account_name_crc = get_tool()->crc32(sz_account_name);

		
		LPSTR sz_account_name_to_cache = map_crc_name.find(dw_account_name_crc);
		if (!VALID_POINT(sz_account_name_to_cache))
		{
			sz_account_name_to_cache = new CHAR[16];
			memcpy(sz_account_name_to_cache, sz_account_name, sizeof(CHAR)*16);
			map_crc_name.add(dw_account_name_crc, sz_account_name_to_cache);
		}					

		s_anti_addiction_info* p_fatigue_info = map_all_fatigue.find(dw_account_name_crc);
		if (!VALID_POINT(p_fatigue_info))
		{
			p_fatigue_info = new s_anti_addiction_info;

			if (!g_datebase.load_fatigue_info(dw_account_name_crc, p_fatigue_info))
			{
				p_fatigue_info->dw_account_name_crc = dw_account_name_crc;
				p_fatigue_info->n_account_online_time_min = 0;
				p_fatigue_info->n_account_offline_time_min = 0;
			}

			map_all_fatigue.add(p_fatigue_info->dw_account_name_crc, p_fatigue_info);
		}

		p_fatigue_info->b_online = TRUE;
	}
}

VOID anti_addiction_timer::notify_ol_guard_users( s_anti_addiction_info* p_snti_addiction_info )
{
	LPSTR sz_account_name = map_crc_name.find(p_snti_addiction_info->dw_account_name_crc);
	if (!VALID_POINT(sz_account_name))
		return;

	tagNS_ANTIENTHRALL send;
	memcpy(send.sz_account_name, sz_account_name, sizeof(CHAR) * 16);
	send.by_color = p_snti_addiction_info->status() + 1;
	send.dw_account_online_seconds = p_snti_addiction_info->n_account_online_time_min * 60;
	p_fatigue_server->send_message(&send, send.dw_size);
}

VOID anti_addiction_timer::notify_login_user( LPCSTR sz_account_name )
{
	DWORD dw_account_name_crc = get_tool()->crc32(sz_account_name);
	s_anti_addiction_info* p_fatigue_info = map_all_fatigue.find(dw_account_name_crc);
	if (VALID_POINT(p_fatigue_info))
	{
		p_fatigue_info->b_last_online = FALSE;
	}
}

anti_addiction_timer::~anti_addiction_timer()
{
	std::list<LPSTR> listNames;
	map_crc_name.copy_value_to_list(listNames);

	while(!listNames.empty())
	{
		LPSTR pToDel = listNames.front();
		listNames.pop_front();
		SAFE_DELETE(pToDel);
	}

	listNames.clear();
	map_crc_name.clear();
}

BOOL anti_addiction_server::init()
{

	if(!init_config())
		return FALSE;

	g_datebase.load_all_fatigue_info(anti_addiction_timer_.get_fatigue_info_map());

	p_session = new few_connect_server;
	if( !VALID_POINT(p_session) ) return FALSE;
	p_session->init(fastdelegate::MakeDelegate(this, &anti_addiction_server::login_call_back),
		fastdelegate::MakeDelegate(this, &anti_addiction_server::logout_call_back), n_port);

	if(!login_server::p_thread->create_thread(_T("AntiAddictionServerThreadUpdate"), &anti_addiction_server::static_thread_update, this))
	{
		return FALSE;
	}

	while(!login_server::p_thread->is_thread_active(_T("AntiAddictionServerThreadUpdate")))
	{
		continue;
	}

	return TRUE;
}


BOOL anti_addiction_server::init_config()
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

	n_port = p_var->get_int(_T("port anti_addiction_server"));


	SAFE_DELETE(p_var);

	return TRUE;
}


VOID anti_addiction_server::destroy()
{

	Interlocked_Exchange((LPLONG)&b_terminate_update, TRUE);
	login_server::p_thread->waitfor_thread_destroy( _T("AntiAddictionServerThreadUpdate"), INFINITE);


	p_session->destory();
	SAFE_DELETE(p_session);

	
}


UINT anti_addiction_server::thread_update()
{
	DWORD dw_time = 0;
	//modify mmz at 2010.9.17 release也记dump
//#ifdef DEBUG
	_set_se_translator(serverdump::si_translation); 

	try
	{
//#endif
		while( !b_terminate_update )
		{
			anti_addiction_timer_.reset();

			update_session();

			anti_addiction_timer_.updata();
		}
//#ifdef DEBUG
	}
	catch(serverdump::throw_exception)
	{
		if( get_tool()->is_debug_present() )
		{
			throw;
		}
		else
		{
			exit(1);
		}
	}
//#endif

	_endthreadex(0);
	return 0;
}

UINT anti_addiction_server::static_thread_update(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	anti_addiction_server* p_this = (anti_addiction_server*)p_data;
	return p_this->thread_update();
}

//-------------------------------------------------------------------------
VOID anti_addiction_server::update_session()
{
	if( NULL == p_session || !VALID_VALUE(dw_section_id))
		return;

	DWORD	dw_size = 0;
	INT		n_un_recved = 0;
	tag_net_message* p_cmd = (tag_net_message*)p_session->recv_msg(dw_section_id, dw_size, n_un_recved);
	

	while (!VALID_POINT(p_cmd) || p_cmd->dw_message_id != get_tool()->crc32("NC_ANTIENTHRALL"))
	{
		if (!VALID_POINT(p_cmd))
		{
			Sleep(100);
		}
		else if (p_cmd->dw_message_id == get_tool()->crc32("NC_ANTIQUERYSTATE"))
		{
			handle_query_state(p_cmd);
			p_session->free_recv_msg(dw_section_id, (LPBYTE)p_cmd);
		}

		p_cmd = (tag_net_message*)p_session->recv_msg(dw_section_id, dw_size, n_un_recved);
	}

	if (p_cmd->dw_message_id == get_tool()->crc32("NC_ANTIENTHRALL"))
		handle_online_accounts(p_cmd);
	p_session->free_recv_msg(dw_section_id, (LPBYTE)p_cmd);

}

DWORD anti_addiction_server::handle_online_accounts( tag_net_message* p_cmd)
{
	tagNC_ANTIENTHRALL* p_receive = (tagNC_ANTIENTHRALL*)p_cmd;

	anti_addiction_timer_.record_ol_guard_accounts(p_receive);

	return 0;
}

DWORD anti_addiction_server::handle_query_state( tag_net_message* p_cmd)
{
	tagNC_ANTIQUERYSTATE* p_receive = (tagNC_ANTIQUERYSTATE*)p_cmd;

	anti_addiction_timer_.notify_login_user(p_receive->sz_account_name);

	return 0;
}

DWORD anti_addiction_server::login_call_back( LPBYTE p_byte, DWORD dw_size )
{
	tagNC_LOGIN_CM* p_receive = (tagNC_LOGIN_CM*)p_byte;

	if (VALID_VALUE(dw_section_id) || get_tool()->crc32("NC_LOGIN_CM") != p_receive->dw_message_id)
		return INVALID_VALUE;
	else
		dw_section_id = p_receive->dw_section_id;
	return dw_section_id;
}

DWORD anti_addiction_server::logout_call_back( DWORD dw_section_id )
{

	if (VALID_VALUE(dw_section_id))
	{
		dw_section_id = INVALID_VALUE;
		return 0;
	}

	return INVALID_VALUE;	
}

anti_addiction_server::anti_addiction_server() : anti_addiction_timer_(this)
{
	n_port = 0;
	b_terminate_update = FALSE;
	b_terminate_connect = FALSE;
	dw_section_id = INVALID_VALUE;
}

VOID anti_addiction_server::send_message( PVOID p_msg, DWORD dw_size )
{
	if(!VALID_VALUE(dw_section_id))
		return;
	p_session->send_msg(dw_section_id, p_msg, dw_size);
}
