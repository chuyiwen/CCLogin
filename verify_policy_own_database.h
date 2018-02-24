/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _VERIFY_POLICY_OWN_DATABASE_H_
#define _VERIFY_POLICY_OWN_DATABASE_H_


#include "verify_policy.h"

//-----------------------------------------------------------------------------
//! 验证策略的服务器接口
//-----------------------------------------------------------------------------
class verify_policy_own_database
{
public:
	verify_policy_own_database() {}
	~verify_policy_own_database() {}

	BOOL	init();

	BOOL	query_account(LPCSTR sz_account_name, string& str_passward, s_proof_result* p_result);
	BOOL	insert_accout(LPCSTR sz_account_name, LPCSTR sz_passward, LPCSTR sz_mibao, BOOL b_guard);
	BOOL	update_accout(LPCSTR sz_account_name, LPCSTR sz_passward, LPCSTR sz_mibao, BOOL b_guard);
	BOOL	update_password(LPCSTR sz_account_name, LPCSTR sz_new_password);
	BOOL	update_mibao(LPCSTR sz_account_name, LPCSTR sz_mibao);

private:
	db_interface		database_interface_;

	few_connect_client	trans_ip;
};


class verify_policy_own : public verify_policy
{
public:

	verify_policy_own() : n_port(0), b_terminate_update(FALSE), b_terminate_connect(FALSE), b_connected(FALSE) 
	{
		
	}
	~verify_policy_own() 
	{
		
	}

public:

	BOOL	init(PROOFCALLBACK fn);
	VOID	destroy();

	VOID	proof(DWORD dw_client_id, LPCSTR sz_accout_name, LPCSTR sz_passward, LPCSTR sz_guid, INT nType = 0);

	INT		get_proof_server_status();

private:
	BOOL	init_config();

	UINT	thread_update();
	static UINT WINAPI static_thread_update(LPVOID p_data);

	UINT	thread_connect_server();
	static UINT WINAPI static_thread_connect_server(LPVOID p_data);

	VOID	update_proof_list();
	VOID	update_session();


	VOID	register_proof_msg();
	VOID	unregister_proof_msg();


	DWORD	handle_login(tag_net_message* p_cmd, DWORD);
	DWORD	handle_set_chenmi(tag_net_message* p_cmd, DWORD);
	DWORD	handle_user_login(tag_net_message* p_cmd, DWORD);
	DWORD	handle_user_update_password(tag_net_message* p_cmd, DWORD);
	DWORD	handle_block_account(tag_net_message* p_cmd, DWORD);
	DWORD	handle_unblock_account(tag_net_message* p_cmd, DWORD);
	DWORD	handle_user_bind_mibao(tag_net_message* p_cmd, DWORD);
	DWORD	handle_user_unbind_mibao(tag_net_message* p_cmd, DWORD);

private:
	
	few_connect_client*				p_transport;


	volatile BOOL				b_terminate_update;
	volatile BOOL				b_terminate_connect;

	CHAR						sz_ip[X_IP_LEN];	//! IP
	INT							n_port;				//! 端口
	volatile BOOL				b_connected;		


	struct s_player_proof_data
	{
		DWORD		dw_client_id;
		string		str_accout_name;
		string		str_password;
		string		str_guid;

		s_player_proof_data(DWORD dw_client_id, LPCSTR sz_accout_name, LPCSTR sz_password, LPCSTR sz_guid)
		{
			dw_client_id		=	dw_client_id;
			str_accout_name		=	sz_accout_name;
			str_password		=	sz_password;
			str_guid			=	sz_guid;
		}
	};

	safe_mutex_list<s_player_proof_data*>	list_proof_data_;		//! 用户数据

	verify_policy_own_database					proof_database_;	
};

#endif