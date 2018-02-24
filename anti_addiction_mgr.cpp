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
#include "anti_addiction_mgr.h"
#include "../common/ServerDefine/anti_addiction_server_define.h"
#include "si_login_server.h"
#include "si_world_mgr.h"
#include "database.h"
#include "anti_addiction_session.h"
#include "anti_addiction_server.h"

anti_addiction_mgr g_anti_mgr;

VOID anti_addiction_mgr::update()
{
	n_min_counter -= TICK_TIME;
	if (n_min_counter <= 0)
	{
		n_min_counter = n_circle_unit;
		notify_anti_addiction_server();
	}

	send_world_notifys();
}

BOOL anti_addiction_mgr::init()
{
	file_container* p_var = new file_container;
	if( !VALID_POINT(p_var) ) return FALSE;

	TCHAR t_sz_path[MAX_PATH];
	ZeroMemory(t_sz_path, sizeof(t_sz_path));

	if (!get_file_io_mgr()->get_ini_path(t_sz_path, _T("server_config/login/center_proof"))||
		!p_var->load(g_login.get_file_system(), t_sz_path))
	{
		ERROR_CLUE_ON(_T("ÅäÖÃÎÄ¼þÎ´ÕÒµ½"));
		return FALSE;
	}

	//p_var->load(g_login.get_file_system(), _T("server_config/login/center_proof.ini"));
	n_circle_unit = p_var->get_int(_T("circle anti_addiction_server"));

	SAFE_DELETE(p_var);

	n_min_counter = n_circle_unit;


	return load_all_accounts();
}

BOOL anti_addiction_mgr::load_all_accounts()
{
	set_lock.lock();
	BOOL bRtv = g_datebase.load_online_guard_account_IDs(map_account_to_name_crc);
	set_lock.un_lock();

	return TRUE;
}

VOID anti_addiction_mgr::notify_anti_addiction_server()
{
	if (!g_anti_addiction_session.is_connected())
		return;

	std::list<DWORD> list_online_accounts;
	set_lock.lock();
	map_account_to_name_crc.copy_key_to_list(list_online_accounts);
	set_lock.un_lock();

	DWORD dwNum = list_online_accounts.size();

	if (dwNum <= 0)
		return;

	if (g_login.is_sim_fatigue_server())
	{
		DWORD *dw_accounts = new DWORD[dwNum];
		std::copy(list_online_accounts.begin(), list_online_accounts.end(), dw_accounts);
		g_anti_addiction_session.send_online_guard_accounts(dw_accounts, dwNum);
		SAFE_DELETE_ARRAY(dw_accounts);
	}
	else
	{
#define MAX_SEND_SIZE 500
		DWORD dw_accounts[MAX_SEND_SIZE];
		INT n_num=0;
		BOOL b_notify = FALSE;
		std::list<DWORD>::iterator itr = list_online_accounts.begin();
		while (itr != list_online_accounts.end())
		{
			dw_accounts[n_num++] = *itr;
			++itr;
			if (n_num == MAX_SEND_SIZE)
			{
				b_notify = TRUE;
				g_anti_addiction_session.send_online_guard_accounts(dw_accounts, n_num);
				n_num = 0;
			}
		}
		if (!b_notify || b_notify && n_num != 0)
		{
			g_anti_addiction_session.send_online_guard_accounts(dw_accounts, n_num);
		}

#undef MAX_SEND_SIZE
	}
}

VOID anti_addiction_mgr::login_notify( DWORD dw_account, DWORD dw_worldNameCrc, BOOL b_guard )
{
	//if (!b_guard)	return;

	set_lock.lock();
	map_account_to_name_crc.modify_value(dw_account, dw_worldNameCrc);
	set_lock.un_lock();

	g_anti_addiction_session.send_query_status(dw_account);

}

VOID anti_addiction_mgr::logout_notify( DWORD dw_account )
{
	set_lock.lock();
	map_account_to_name_crc.erase(dw_account);
	set_lock.un_lock();
}

VOID anti_addiction_mgr::notify_player( const s_anti_addiction_notify* p_notify )
{
	DWORD dw_world_name_crc = map_account_to_name_crc.find(p_notify->dw_account_id);
	if (!VALID_VALUE(dw_world_name_crc)) 
		return;

	s_anti_addiction_notify* p_new_notify = new s_anti_addiction_notify;
	*p_new_notify = *p_notify;

	package_list<s_anti_addiction_notify*>* p_list_notifys = map_notify_lists.find(dw_world_name_crc);
	if (!VALID_POINT(p_list_notifys))
	{
		p_list_notifys = new package_list<s_anti_addiction_notify*>;
		map_notify_lists.add(dw_world_name_crc, p_list_notifys);
	}

	p_list_notifys->push_back(p_new_notify);
}

VOID anti_addiction_mgr::send_world_notifys()
{
	static BYTE buffer[5000 * 8] = {0};
	tagNLW_FatigueNotify* p_send = reinterpret_cast<tagNLW_FatigueNotify*>(buffer);
	p_send->dw_message_id = get_tool()->crc32("NLW_FatigueNotify");
	p_send->n_num = 0;


	package_map<DWORD, package_list<s_anti_addiction_notify*>* >::map_iter itr = map_notify_lists.begin();
	DWORD dw_world_name_crc = INVALID_VALUE;
	package_list<s_anti_addiction_notify*>* p_list_notifies = NULL;
	while(map_notify_lists.find_next(itr, dw_world_name_crc, p_list_notifies))
	{
		if (!VALID_POINT(p_list_notifies)) continue;

		p_send->n_num = 0;
		s_anti_addiction_notify* p_notify = NULL;
		while (!p_list_notifies->empty())
		{
			p_notify = p_list_notifies->pop_front();
			p_send->notify[p_send->n_num++] = *p_notify;
			SAFE_DELETE(p_notify);
		}

		SAFE_DELETE(p_list_notifies);

		if (p_send->n_num > 0)
		{
			p_send->dw_size = sizeof(tagNLW_FatigueNotify) - sizeof(s_anti_addiction_notify) + sizeof(s_anti_addiction_notify) * p_send->n_num;
		}
		else
		{
			p_send->dw_size = sizeof(tagNLW_FatigueNotify);
		}

		g_fpWorldMgr.send_msg(dw_world_name_crc, p_send, p_send->dw_size);
	}

	map_notify_lists.clear();
}

VOID anti_addiction_mgr::reset_world_accounts( DWORD dw_world_name_crc_reset, DWORD *p_ol_account_ids, INT n_olaccount_id_num )
{
	set_lock.lock();
	
	package_map<DWORD, DWORD>::map_iter itr = map_account_to_name_crc.begin();
	DWORD dw_account_id = INVALID_VALUE;
	DWORD dw_world_name_crc = INVALID_VALUE;
	while (map_account_to_name_crc.find_next(itr, dw_account_id, dw_world_name_crc))
	{
		if (VALID_VALUE(dw_account_id) && VALID_VALUE(dw_world_name_crc) && dw_world_name_crc == dw_world_name_crc_reset )
		{
			map_account_to_name_crc.erase(dw_account_id);
		}
	}

	if (!VALID_POINT(p_ol_account_ids) || 0 == n_olaccount_id_num)
	{
		set_lock.un_lock();
		return;
	}

	for (INT i=0; i<n_olaccount_id_num; ++i)
	{
		if (!VALID_VALUE(map_account_to_name_crc.find(p_ol_account_ids[i])))
		{
			map_account_to_name_crc.add(p_ol_account_ids[i], dw_world_name_crc_reset);
		}
	}

	set_lock.un_lock();
}
