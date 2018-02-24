/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/


#include "stdafx.h"

#include "../../common/WorldDefine/login_protocol.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/login_world.h"

#include "si_world.h"
#include "si_world_mgr.h"
#include "user_mgr.h"
#include "user.h"
#include "database.h"
#include "si_login_server.h"
#include "anti_addiction_mgr.h"


const INT	ALLOW_PLAYER_LOGIN	=	2;		//! 玩家进入游戏的间隔
const INT	UPDATE_QUEUE_TIME	=	2000;	//! 更新排队列表间隔
const FLOAT	LOGIN_LIMIT			=	0.9f;	//! 排队人数比例
#define CONFIG_INI  "server_config\\login\\login"


si_world::si_world() : data_(), dw_time_(0)
{
	dw_begin_time = GetCurrentDWORDTime();
}


si_world::~si_world()
{
	destroy();
}


BOOL si_world::init(INT n_index)
{
	if( n_index < 0 ) return FALSE;

	dw_time_	=	timeGetTime();
	dw_queue_time_ = timeGetTime();
	dw_db_update_time_ = timeGetTime();
	dw_db_insert_time = timeGetTime();


	TCHAR sz_temp[X_SHORT_NAME] = {_T('\0')};
	_stprintf(sz_temp, _T("zone%d"), n_index);

	tstring str_world_name = login_server::p_var_config->get_string(_T("name"), sz_temp);
	if( str_world_name.empty() ) return FALSE;

	data_.b_valid			=	FALSE;
	data_.e_status			=	ews_init_not_done;
	data_.n_cur_online_num	=	0;
	data_.n_max_online_num	=	0;
	data_.dw_name_crc		=	get_tool()->crc32(str_world_name.c_str());
	data_.dw_ip				=	0;
	data_.n_port			=	0;
	data_.b_auto_seal		=	login_server::p_var_config->get_dword(_T("auto_seal"), sz_temp);
	_tcsncpy(data_.sz_name, str_world_name.c_str(), X_SHORT_NAME);
	
  	data_.dw_world_id		=	login_server::p_var_config->get_dword(_T("id"), sz_temp); 

	this->n_Index = n_index;
	f_login_lime = login_server::p_var_config->get_float(_T("login_limit"), sz_temp);
	dw_login_time = login_server::p_var_config->get_dword(_T("queue_time"), sz_temp);
    return TRUE;
}

VOID si_world::update()
{
	if( is_valid() )
	{
		
		update_session();
		update_insert_player();
		update_queued_player();
		update_waiting_player();
		update_kicked_player();
		update_player_queue_limit();
	}
	else
	{
		update_kicked_player();
	}
}


VOID si_world::update_session()
{
	LPBYTE	p_receive		=	NULL;
	DWORD	dw_size			=	0;
	INT		n_un_receive	=	0;
	DWORD	dw_time			=	0;


	p_receive = g_fpWorldMgr.receive_msg(data_.dw_name_crc, dw_size, n_un_receive);

	while( VALID_POINT(p_receive) )
	{

		g_fpWorldMgr.handle_cmd((tag_net_message*)p_receive, dw_size, (DWORD)this);
		g_fpWorldMgr.return_msg(data_.dw_name_crc, p_receive);
		p_receive = g_fpWorldMgr.receive_msg(data_.dw_name_crc, dw_size, n_un_receive);
	}
}

VOID si_world::update_player_queue_limit()
{
	if(CalcTimeDiff(GetCurrentDWORDTime(), dw_begin_time) >= 60)
	{
		p_temp_config = new file_container;

		TCHAR t_sz_path[MAX_PATH];
		ZeroMemory(t_sz_path, sizeof(t_sz_path));
		if (!get_file_io_mgr()->get_ini_path(t_sz_path, _T(CONFIG_INI))||
			!p_temp_config->load(g_login.get_file_system(), t_sz_path))
		{
			//ERROR_CLUE_ON(_T("配置文件未找到"));
			return;
		}

		TCHAR sz_temp[X_SHORT_NAME] = {_T('\0')};
		_stprintf(sz_temp, _T("zone%d"), this->n_Index);
		f_login_lime = p_temp_config->get_float(_T("login_limit"), sz_temp);
		dw_login_time = p_temp_config->get_dword(_T("queue_time"), sz_temp);
		dw_begin_time = GetCurrentDWORDTime();

		SAFE_DELETE(p_temp_config);
	}
}


VOID si_world::update_insert_player()
{
	if( !data_.b_valid ) return;
	if( ews_well != data_.e_status ) return;

	INT n_begin_wait_num = data_.n_max_online_num * f_login_lime;			
	INT n_temp = data_.n_cur_online_num;// + map_waitting_.size();	

	user* p_player	= list_insert_player_.pop_front();
	while( VALID_POINT(p_player) )
	{

		if( list_queue_.empty() )
		{
			if( n_temp >= n_begin_wait_num )
			{
				add_into_queue(p_player);
			}
			else
			{
				add_into_waiting_map(p_player);
				++n_temp;		
			}
		}
		else
		{
			if(p_player->get_privilege() > 0)
			{
				add_into_waiting_map(p_player);
			}
			else
			{
				add_into_queue(p_player);
			}
		}

		p_player = list_insert_player_.pop_front();
	}
}


VOID si_world::update_queued_player()
{
	if( !data_.b_valid ) return;
	if( ews_well != data_.e_status ) return;

	if( list_queue_.empty() ) return;

	BOOL b_update_queue = FALSE;
	if( timeGetTime() - dw_time_ > UPDATE_QUEUE_TIME )
	{
		dw_time_ = timeGetTime();
		b_update_queue = TRUE;
	}

	BOOL b_slow_enter = TRUE;

	INT n_begin_wait_num = data_.n_max_online_num * f_login_lime;							
	INT n_no_wait_num = n_begin_wait_num - data_.n_cur_online_num - map_waitting_.size();	

	/*INT n_have_player = data_.n_max_online_num - data_.n_cur_online_num - map_waitting_.size();
	if( n_have_player <= 0 ) b_slow_enter = TRUE;*/
	if(list_queue_.size() > 0 && data_.n_cur_online_num < g_login.get_login_limit_num()*data_.n_max_online_num)
	{
		if(timeGetTime() - dw_queue_time_ > dw_login_time)
		{
			b_slow_enter = FALSE;
		}

	}

	list_queue_.reset_iterator();
	user* p_player = NULL;

	INT n_index_in_queue = 0;		
	while( list_queue_.find_next(p_player) )
	{
		if( p_player->is_conn_lost() )		
		{
			list_queue_.erase(p_player);
			g_PlayMgr.player_logout(p_player);
			continue;
		}


		if( b_update_queue )
		{
			if( n_no_wait_num > 0 )			
			{
				list_queue_.erase(p_player);

				NET_L2C_queue send;
				send.dwPosition = 0;
				send.dw_time = 0;
				p_player->send_message(&send, send.dw_size);

				add_into_waiting_map(p_player);
				--n_no_wait_num;
			}
			else if( !b_slow_enter )			
			{
				list_queue_.erase(p_player);

				NET_L2C_queue send;
				send.dwPosition = 0;
				send.dw_time = 0;

				p_player->send_message(&send, send.dw_size);

				add_into_waiting_map(p_player);
				b_slow_enter = TRUE;
				dw_queue_time_ = timeGetTime();
			}
			else							
			{
				NET_L2C_queue send;
				send.dwPosition = ++n_index_in_queue;
				send.dw_time = n_index_in_queue * ALLOW_PLAYER_LOGIN;
				p_player->send_message(&send, send.dw_size);
			}
		}
	}
}

VOID si_world::update_waiting_player()
{
	if( map_waitting_.empty() ) return;

	user* p_player = NULL;
	map_waitting_.reset_iterator();

	while( map_waitting_.find_next(p_player) )
	{
		if( p_player->is_conn_lost() )				
		{
			map_waitting_.erase(p_player->get_account_id());
			g_PlayMgr.player_logout(p_player);
		}
		else if( p_player->is_select_world_ok() )		
		{
			if( CalcTimeDiff(GetCurrentDWORDTime(), p_player->get_kick_time()) > 5 )	
			{
				map_waitting_.erase(p_player->get_account_id());
				kick_player(p_player);
			}
		}
	}
}


VOID si_world::update_kicked_player()
{
	if( list_kick_player.empty() ) return;

	user* p_player = NULL;
	list_kick_player.reset_iterator();

	while( list_kick_player.find_next(p_player) )
	{
		if( p_player->is_conn_lost() )		
		{
			list_kick_player.erase(p_player);
			g_PlayMgr.player_logout(p_player);
		}
	}
}


VOID si_world::destroy()
{
	
}

VOID si_world::add_into_queue(user* p_player)
{
	if( !VALID_POINT(p_player) ) return;

	list_queue_.push_back(p_player);


	NET_L2C_proof_result send;
	send.bGuard			=	p_player->is_guard();
	send.dw_error_code	=	E_ProofResult_Queue;

	p_player->send_message(&send, send.dw_size);
}


VOID si_world::add_into_waiting_map(user* p_player)
{
	if( !VALID_POINT(p_player) ) return;

	map_waitting_.add(p_player->get_account_id(), p_player);

	
	NET_L2W_player_will_login send;
	send.dw_verify_code	=	p_player->get_verify_code();
	send.dw_account_id	=	p_player->get_account_id();
	send.by_privilege	=	p_player->get_privilege();
	send.b_guard			=	p_player->get_player_data().b_guard;
	send.dw_account_online_sec		=	p_player->get_player_data().n_acc_online_sec;
	
	send.dw_pre_login_ip		=	p_player->get_pre_login_ip();
	send.dw_pre_login_time		=	p_player->get_pre_login_time();

	strncpy_s(send.sz_account, p_player->get_accout_name(), X_SHORT_NAME);

	g_fpWorldMgr.send_msg(data_.dw_name_crc, &send, send.dw_size);
}


BOOL si_world::world_login(DWORD dw_ip, INT n_port, DWORD* p_ol_account_ids, INT n_ol_account_id_num)
{
	if( is_valid() ) return FALSE;	


	data_.b_valid			=	TRUE;
	data_.dw_ip				=	dw_ip;
	data_.n_port			=	n_port;
	data_.e_status			=	ews_init_not_done;
	data_.n_cur_online_num	=	0;
	data_.n_max_online_num	=	0;

	
	g_datebase.fixed_world_login_status(data_.dw_name_crc, epls_online, epls_unknown);

	
	g_datebase.reset_login_status(p_ol_account_ids, n_ol_account_id_num, epls_online);

	
	g_datebase.fixed_world_login_status(data_.dw_name_crc, epls_unknown, epls_off_line);

	g_anti_mgr.reset_world_accounts(data_.dw_name_crc, p_ol_account_ids, n_ol_account_id_num);

	return TRUE;
}


BOOL si_world::world_logout()
{
	if( !is_valid() ) return FALSE;	


	data_.b_valid			=	FALSE;
	data_.dw_ip				=	0;
	data_.n_port			=	0;
	data_.e_status			=	ews_init_not_done;
	data_.n_cur_online_num	=	0;
	data_.n_max_online_num	=	0;

	user* p_player = NULL;


	p_player = list_insert_player_.pop_front();
	while( VALID_POINT(p_player) )
	{
		kick_player(p_player);

		p_player = list_insert_player_.pop_front();
	}


	p_player = list_queue_.pop_front();
	while( VALID_POINT(p_player) )
	{
		kick_player(p_player);

		p_player = list_queue_.pop_front();
	}


	map_waitting_.reset_iterator();
	while( map_waitting_.find_next(p_player) )
	{
		kick_player(p_player);
	}
	map_waitting_.clear();

	g_anti_mgr.reset_world_accounts(data_.dw_name_crc, NULL, 0);

	return TRUE;
}


VOID si_world::player_will_login_ret(DWORD dw_account_id, DWORD dw_error_code)
{

	user* p_player = map_waitting_.find(dw_account_id);
	if( VALID_POINT(p_player) )
	{

		if( E_Success == dw_error_code )
		{
			p_player->set_select_world_ok();
		}

		NET_L2C_proof_result send;
		send.bGuard			=	p_player->is_guard();
		send.dwIndex		=	0;
		send.dw_ip			=	data_.dw_ip;
		send.dwPort			=	data_.n_port;
		send.dw_account_id	=	dw_account_id;
		send.dwVerifyCode	=	p_player->get_verify_code();
		send.special_account=	p_player->get_specialacc( ); 

		if( E_Success == dw_error_code )
			send.dw_error_code	=	E_Success;
		else
			send.dw_error_code	=	E_ProofResult_Account_No_Match;

		p_player->send_message(&send, send.dw_size);

		p_player->set_kick_time(GetCurrentDWORDTime());
	}
}

VOID si_world::kick_player(user* p_player)
{
	if( !VALID_POINT(p_player) ) return;

	g_PlayMgr.kick(p_player->get_cd_index());
	list_kick_player.push_back(p_player);
}

VOID si_world::add_player(user* p_player)
{
	if( VALID_POINT(p_player) )
	{
		list_insert_player_.push_front(p_player);
	}
}

VOID si_world::update_status(E_world_status e_status, INT n_cur_online, INT n_max_online)
{
	data_.e_status			=	e_status;
	data_.n_max_online_num	=	n_max_online;
	data_.n_cur_online_num	=	n_cur_online;
}


