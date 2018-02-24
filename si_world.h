/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _L_WORLD_H_
#define _L_WORLD_H_


#include "../common/ServerDefine/login_world.h"
#include "si_world_mgr.h"
#include "user_mgr.h"
#include "user.h"

class user;

class si_world
{

public:
    si_world();
    ~si_world();

	BOOL			init(INT n_index);
	VOID			update();
	VOID			destroy();
	
	//! 登入
	BOOL			world_login(DWORD dw_ip, INT n_port, DWORD* p_ol_account_ids, INT n_ol_account_id_num);
	//! 登出
	BOOL			world_logout();

	//--------------------------------------------------------------------------
	DWORD			get_id()				{ return data_.dw_name_crc; }
	
	DWORD			get_world_id()		{ return data_.dw_world_id; }
	E_world_status	get_status()			{ return data_.e_status; }
	INT				get_max_online_num()	{ return data_.n_max_online_num; }
	VOID			set_max_online_num(INT n){ data_.n_max_online_num = n; }
	INT				get_curr_online_num()	{ return data_.n_cur_online_num; }
	LPCTSTR			get_name()			{ return data_.sz_name; }
	INT				get_list_queue()		{ return list_queue_.size(); }
	DWORD			get_db_update_time()	{ return dw_db_update_time_;	}
	DWORD			get_db_insert_time()	{ return dw_db_insert_time;	}

	//--------------------------------------------------------------------------
	VOID			set_db_update_time(DWORD dw_time)	{ dw_db_update_time_ = dw_time;}
	VOID			set_db_insert_time(DWORD dw_time)	{ dw_db_insert_time = dw_time;}

	BOOL			is_valid()			{ return data_.b_valid; }
	BOOL			is_auto_seal()		{ return data_.b_auto_seal; }

	//--------------------------------------------------------------------------
	VOID			update_status(E_world_status e_status, INT n_cur_online, INT n_max_online);


	//--------------------------------------------------------------------------
	VOID			kick_player(user* p_player);
	VOID			add_player(user* p_player);
	

	//!玩家登入结果
	VOID			player_will_login_ret(DWORD dw_account_id, DWORD dw_error_code);

	//! 告诉游戏服务器踢人
	VOID			send_kick_player_msg(NET_L2W_kick_player& send) { g_fpWorldMgr.send_msg(data_.dw_name_crc, &send, send.dw_size); }

	FLOAT			getLoginLimit() { return f_login_lime; }
private:

	VOID			update_session();
	VOID			update_insert_player();
	VOID			update_queued_player();
	VOID			update_waiting_player();
	VOID			update_kicked_player();
	VOID			update_player_queue_limit();


	VOID			add_into_queue(user* p_player);
	VOID			add_into_waiting_map(user* p_player);

private:
   
	

	s_world_data					data_;
	DWORD							dw_time_;	
	DWORD							dw_queue_time_;

	DWORD							dw_db_update_time_;			 
	DWORD							dw_db_insert_time;			 

	package_list<user*>					list_insert_player_;			//! 插队的人
	package_list<user*>					list_queue_;				//! 排队的人
	package_map<DWORD, user*>			map_waitting_;				//!等待进入的人
	package_list<user*>					list_kick_player;			//! 被踢掉的人

	FLOAT							f_login_lime;				// 排队限制
	DWORD							dw_login_time;				// 排队进入时间
	INT								n_Index;					// 服务器编号
	DWORD							dw_begin_time;		
	file_container*					p_temp_config;

};

#endif






