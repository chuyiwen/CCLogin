/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

/**
*	@file		database_operator.h
*	@author		lc
*	@date		2011/01/18	initial
*	@version	0.0.1.0
*	@brief		封装数据操作
*/


#ifndef _DATABASE_H_
#define _DATABASE_H_


#include "../common/ServerDefine/login_server_define.h"

class user;
struct s_anti_addiction_info;


class database_operator /*: public Singleton<database_operator>*/
{
public:
	database_operator() { }
	~database_operator() { /*destroy();*/ }

    BOOL	init();
    VOID	destroy();
    
//! 一些玩家事件

	//!玩家登入
	VOID	login(DWORD dw_account_id, DWORD dw_world_name_crc);
	//!玩家登出
	VOID	logout(DWORD dw_account_id);
	//!玩家进入游戏世界
	VOID	into_world(DWORD dw_account_id, DWORD dw_world_name_crc);
	//!玩家退出游戏世界
	VOID	exit_world(DWORD dw_account_id, DWORD dw_world_name_crc);
	
	//!修正玩家登入标志
	VOID	fixed_login_status(DWORD dw_account_id, E_player_login_status e_dest);
	//!修正指定世界玩家登入标志
	VOID	fixed_world_login_status( DWORD dw_world_name_crc, E_player_login_status e_src, E_player_login_status e_dest );
	//!重置登入标志
	VOID	reset_login_status(DWORD* p_dwaccount_id, INT n_num, E_player_login_status e_dest);
	//!重置所有登入标志
	VOID	reset_all_login_status(E_player_login_status e_src, E_player_login_status e_dest);
	
	//!得到某个用户密保
	BOOL	get_mibao(DWORD dw_account_id, CHAR sz_mibao[MIBAO_LENGTH]);

	BOOL	load_all_fatigue_info(package_map<DWORD, s_anti_addiction_info*> &map_fatigue_info);
	BOOL	load_cache_account_data(package_safe_map<DWORD, s_account_data*> &map_account_data, package_safe_map<DWORD, DWORD> &map_name_crc_to_account_id);
	BOOL	load_online_guard_account_IDs(package_map<DWORD, DWORD> &map_accounts);

	VOID	save_fatigue_info(s_anti_addiction_info* p_fatigue_info);
	BOOL	load_fatigue_info(DWORD dw_account_name_crc, s_anti_addiction_info* p_fatigue_info);
	BOOL	if_world_forbid(DWORD dw_account_id,DWORD dw_world_name_crc);
	BOOL	if_have_world_forbid(DWORD dw_account_id);
	BOOL	if_ip_forbid(DWORD dw_ip);

	DWORD	forbid_account(DWORD dw_account_id,DWORD dw_forbidM,DWORD dw_world_name_crc = 0);
	DWORD	forbid_account(LPCTSTR sz_account_name,DWORD dw_forbidM,DWORD dw_world_name_crc = 0,  DWORD start = 0, DWORD end = 0);

	DWORD	remove_account_forbid(DWORD dw_account_id,DWORD dw_forbidM,DWORD dw_world_name_crc = 0);
	DWORD	remove_account_forbid(LPCTSTR sz_account_name,DWORD dw_forbidM,DWORD dw_world_name_crc = 0);
	
	DWORD	forbid_ip(DWORD dw_ip);
	DWORD	remove_ip_forbid(DWORD dw_ip);

	VOID	update_account_login_info( DWORD dw_account_id, DWORD &dw_ip);
	VOID	log_action( DWORD dw_account_id, LPCSTR sz_account_name, DWORD dw_ip, LPCSTR sz_action );
	VOID	insert_kick_log(const CHAR* p_role_name, const CHAR* p_world_name, const CHAR* p_account_name, DWORD dw_account_id, DWORD dw_time, UINT16 u_16err_code, BOOL b_seal, DWORD dwRoleLevel);

	BOOL    update_world_state(DWORD dw_world_id,INT i_role_num, SHORT s_state);
	BOOL    insert_world_state(DWORD dw_world_id);

	DWORD	set_account_chenmi(DWORD dw_account_id,DWORD dw_chen_mi,DWORD dw_world_name_crc = 0);

	BOOL    clear_world_state_table();

	BOOL	insert_world_state_log(DWORD dw_world_id,INT i_role_num, SHORT s_state);


	INT		get_available_stream_num()			{ return database_.get_idlesse_io_num(); }
	INT		get_all_stream_num()				{ return database_.get_all_io_num(); }
	INT		get_unhandled_asyn_stream_num()		{ return database_.get_asynchronism_queue_size(); }
	DWORD   get_world_state_update_time()		{ return dw_world_state_update_time_;}
	DWORD   get_world_state_insert_time()		{ return dw_world_state_insert_time_;}

private:
	db_interface		database_;					

	DWORD			dw_world_state_update_time_;		

	DWORD			dw_world_state_insert_time_;	

};

VOID call_back(db_interface* p_database, INT nReason, INT nParam);


extern database_operator g_datebase;

#endif



