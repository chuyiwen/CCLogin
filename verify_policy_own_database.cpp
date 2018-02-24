/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#pragma once
#include "stdafx.h"

#include "../common/ServerDefine/base_server_define.h"
#include "verify_policy_own_define.h"
#include "verify_policy.h"
#include "verify_policy_own_database.h"
#include "si_login_server.h"
#include "database.h"


BOOL verify_policy_own_database::init()
{
	if( !VALID_POINT(login_server::p_var_config) ) return FALSE;

	BOOL b_ret = database_interface_.init_db(login_server::p_var_config->get_string(_T("ip database")), 
		login_server::p_var_config->get_string(_T("user database")),
		login_server::p_var_config->get_string(_T("psd database")), 
		login_server::p_var_config->get_string(_T("name database")),
		(INT)login_server::p_var_config->get_dword(_T("port database")) );

	return b_ret;
}

BOOL verify_policy_own_database::query_account(LPCSTR sz_account_name, string& str_passward, s_proof_result* pProofResult)
{
	if( !VALID_POINT(sz_account_name) || !VALID_POINT(pProofResult) ) return FALSE;

	sql_language_disposal* p_stream = database_interface_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	char sz_ip[X_IP_LEN] = "";
	char sz_date_time[X_DATATIME_LEN + 1]= "";

	tag_mysql_connect* p_con = database_interface_.get_idlesse_connect();

	p_stream->select_item("account", "id,forbid_mask,privilege,guard,login_status,worldname_crc,psd,mibao,ip,time, special_flag");
	p_stream->where_item();
	p_stream->write_string("name='").write_string(sz_account_name, p_con).write_string("'");


	database_interface_.return_use_connect(p_con);

	execute_result* p_result = database_interface_.sql_query(p_stream);


	database_interface_.return_io(p_stream);


	if( !VALID_POINT(p_result) || p_result->get_row_count() <= 0) return FALSE;

	pProofResult->dw_account_id		=	(*p_result)[0].get_dword();
	pProofResult->dw_frobid_mask		=	(*p_result)[1].get_dword();
	pProofResult->by_privilege		=	(*p_result)[2].get_byte();
	pProofResult->b_guard			=	(*p_result)[3].get_bool();
	pProofResult->n_guard_accu_time	=	0;
	pProofResult->e_login_status		=	(E_player_login_status)(*p_result)[4].get_int();
	pProofResult->dw_world_name_crc	=	(*p_result)[5].get_dword();
	pProofResult->by_SpecialAccount = (*p_result)[10].get_byte();

	
	str_passward = (*p_result)[6].get_string();

	
	CHAR sz_mibao[MIBAO_LENGTH] = {'\0'};
	BOOL b_ret = (*p_result)[7].get_blob(sz_mibao, MIBAO_LENGTH);
	if( '\0' == sz_mibao[0] )
	{
		pProofResult->b_need_mibao = FALSE;
	}
	else
	{
		pProofResult->b_need_mibao = TRUE;
	}

	
	memcpy(sz_ip,(*p_result)[8].get_string(),(*p_result)[8].get_length());
	memcpy(sz_date_time,(*p_result)[9].get_string(),(*p_result)[9].get_length());

	pProofResult->dw_pre_login_ip = trans_ip.stringip_to_ip(sz_ip);
	DataTime2DwordTime(pProofResult->dw_pre_login_time,sz_date_time,(*p_result)[9].get_length());

	
	database_interface_.free_result_query(p_result);

	return TRUE;
}

BOOL verify_policy_own_database::insert_accout(LPCSTR sz_account_name, LPCSTR sz_passward, LPCSTR sz_mibao, BOOL b_guard)
{
	if( !VALID_POINT(sz_account_name) || !VALID_POINT(sz_passward) ) return FALSE;

	sql_language_disposal* p_stream = database_interface_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	
	tag_mysql_connect* p_con = database_interface_.get_idlesse_connect();

	p_stream->insert_item("account");
	p_stream->write_string("name='").write_string(sz_account_name, p_con).write_string("',");
	p_stream->write_string("psd='").write_string(sz_passward, p_con).write_string("',");
	p_stream->write_string("mibao='").write_blob(sz_mibao, MIBAO_LENGTH, p_con).write_string("',");
	p_stream->write_string("guard=") << b_guard;

	
	database_interface_.return_use_connect(p_con);

	
	BOOL b_ret = database_interface_.sql_execute(p_stream);

	
	database_interface_.return_io(p_stream);

	return b_ret;
}

BOOL verify_policy_own_database::update_accout(LPCSTR sz_account_name, LPCSTR sz_passward, LPCSTR sz_mibao, BOOL b_guard)
{
	if( !VALID_POINT(sz_account_name) || !VALID_POINT(sz_passward) ) return FALSE;

	sql_language_disposal* p_stream = database_interface_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	
	tag_mysql_connect* p_con = database_interface_.get_idlesse_connect();

	p_stream->update_item("account");
	p_stream->write_string("psd='").write_string(sz_passward, p_con).write_string("',");
	p_stream->write_string("mibao='").write_blob(sz_mibao, MIBAO_LENGTH, p_con).write_string("',");
	p_stream->write_string("guard=") << b_guard;
	p_stream->where_item();
	p_stream->write_string("name='").write_string(sz_account_name, p_con).write_string("'");

	
	database_interface_.return_use_connect(p_con);

	
	BOOL b_ret = database_interface_.sql_execute(p_stream);

	
	database_interface_.return_io(p_stream);

	return b_ret;
}


//-------------------------------------------------------------------------
BOOL verify_policy_own_database::update_password(LPCSTR sz_account_name, LPCSTR sz_new_password)
{
	if( !VALID_POINT(sz_account_name) || !VALID_POINT(sz_new_password) ) return FALSE;

	sql_language_disposal* p_stream = database_interface_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	
	tag_mysql_connect* p_con = database_interface_.get_idlesse_connect();

	p_stream->update_item("account");
	p_stream->write_string("psd='").write_string(sz_new_password, p_con).write_string("'");
	p_stream->where_item();
	p_stream->write_string("name='").write_string(sz_account_name, p_con).write_string("'");

	
	database_interface_.return_use_connect(p_con);

	
	BOOL b_ret = database_interface_.sql_execute(p_stream);

	
	database_interface_.return_io(p_stream);

	return b_ret;
}


//-------------------------------------------------------------------------
BOOL verify_policy_own_database::update_mibao(LPCSTR sz_account_name, LPCSTR sz_mibao)
{
	if( !VALID_POINT(sz_account_name) ) return FALSE;

	sql_language_disposal* p_stream = database_interface_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	
	tag_mysql_connect* p_con = database_interface_.get_idlesse_connect();

	p_stream->update_item("account");
	p_stream->write_string("mibao='").write_blob(sz_mibao, MIBAO_LENGTH, p_con).write_string("'");
	p_stream->where_item();
	p_stream->write_string("name='").write_string(sz_account_name, p_con).write_string("'");

	
	database_interface_.return_use_connect(p_con);


	BOOL b_ret = database_interface_.sql_execute(p_stream);

	
	database_interface_.return_io(p_stream);

	return b_ret;
}

BOOL verify_policy_own::init_config()
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

	
	TCHAR szIP[X_IP_LEN];
	_tcsncpy(szIP, p_var->get_string(_T("ip server")), cal_tchar_array_num(szIP) - 1);
	memcpy(sz_ip, get_tool()->unicode_to_unicode8(szIP), sizeof(sz_ip) - 1);

	n_port = p_var->get_int(_T("port server"));

	
	SAFE_DELETE(p_var);

	return TRUE;
}
UINT verify_policy_own::thread_update()
{
	DWORD dw_time = 0;

	//#ifdef DEBUG
	_set_se_translator(serverdump::si_translation); 

	try
	{
		//#endif
		while( !b_terminate_update )
		{
			
			dw_time = timeGetTime();

			
			update_proof_list();

			
			update_session();

		
			WaitForSingleObject(list_proof_data_.get_event(), 50);
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

UINT verify_policy_own::static_thread_update(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	verify_policy_own* p_this = (verify_policy_own*)p_data;
	return p_this->thread_update();
}


BOOL verify_policy_own::init(PROOFCALLBACK fn)
{
	fn_callback_ = fn;

	
	if( !init_config() )
	{
		return FALSE;
	}

	
	if( !proof_database_.init() )
	{
		return FALSE;
	}

	p_transport = new few_connect_client;
	if( !VALID_POINT(p_transport) ) return FALSE;
	p_transport->init();

	register_proof_msg();

	
	if(!login_server::p_thread->create_thread(_T("ProofPolicyOwnConnectServer"), 
		&verify_policy_own::static_thread_connect_server, 
		this))
	{
		return FALSE;
	}

	while(!login_server::p_thread->is_thread_active(_T("ProofPolicyOwnConnectServer")))
	{
		continue;
	}


	if(!login_server::p_thread->create_thread(_T("ProofPolicyOwnThreadUpdate"),
		&verify_policy_own::static_thread_update,
		this))
	{
		return FALSE;
	}

	while(!login_server::p_thread->is_thread_active(_T("ProofPolicyOwnThreadUpdate")))
	{
		continue;
	}

	return TRUE;
}


VOID verify_policy_own::destroy()
{


	Interlocked_Exchange((LPLONG)&b_terminate_update, TRUE);
	login_server::p_thread->waitfor_thread_destroy( _T("ProofPolicyOwnThreadUpdate"), INFINITE);

	Interlocked_Exchange((LPLONG)&b_terminate_connect, TRUE);
	login_server::p_thread->waitfor_thread_destroy( _T("ProofPolicyOwnConnectServer"), INFINITE);


	p_transport->destory();
	SAFE_DELETE(p_transport);


	unregister_proof_msg();


	s_player_proof_data* p_data = list_proof_data_.pop_front();

	while( VALID_POINT(p_data) )
	{
		SAFE_DELETE(p_data);

		p_data = list_proof_data_.pop_front();
	}
}

VOID verify_policy_own::register_proof_msg()
{
	REGISTER_NET_MSG("NS_LOGIN",		verify_policy_own,		handle_login,				_T("login return") );
	REGISTER_NET_MSG("NS_USERLOGIN",	verify_policy_own,		handle_user_login,			_T("accout login return") );
	REGISTER_NET_MSG("NS_USERUPDATEPWD",	verify_policy_own,	handle_user_update_password,		_T("accout update password") );
	REGISTER_NET_MSG("NS_USERBINDMIBAO",	verify_policy_own,	handle_user_bind_mibao,		_T("accout bind mibao") );
	REGISTER_NET_MSG("NS_USERUNBINDMIBAO",verify_policy_own,	handle_user_unbind_mibao,	_T("accout unbind mibao") );

	REGISTER_NET_MSG("NS_BLOCKACCOUNT",		verify_policy_own,handle_block_account,	_T("block accout ") );
	REGISTER_NET_MSG("NS_UNBLOCKACCOUNT",	verify_policy_own,handle_unblock_account,	_T("unblock accout ") );
	REGISTER_NET_MSG("NS_USERUPDATECHENMI",	verify_policy_own,handle_set_chenmi,			_T("set chenmi") );

}

VOID verify_policy_own::unregister_proof_msg()
{
	UNREGISTER_NET_MSG("NS_LOGIN",		verify_policy_own,		handle_login);
	UNREGISTER_NET_MSG("NS_USERLOGIN",	verify_policy_own,		handle_user_login);
	UNREGISTER_NET_MSG("NS_USERUPDATEPWD",	verify_policy_own,	handle_user_update_password );
	UNREGISTER_NET_MSG("NS_USERBINDMIBAO",	verify_policy_own,	handle_user_bind_mibao);
	UNREGISTER_NET_MSG("NS_USERUNBINDMIBAO",verify_policy_own,	handle_user_unbind_mibao);

	UNREGISTER_NET_MSG("NS_BLOCKACCOUNT",		verify_policy_own,handle_block_account);
	UNREGISTER_NET_MSG("NS_UNBLOCKACCOUNT",	verify_policy_own,handle_unblock_account);
	UNREGISTER_NET_MSG("NS_USERUPDATECHENMI",	verify_policy_own,handle_set_chenmi);
}



//-------------------------------------------------------------------------
DWORD verify_policy_own::handle_login(tag_net_message* p_cmd, DWORD)
{
	tagNS_LOGIN* p_recv = (tagNS_LOGIN*)p_cmd;

	if( 0 != p_recv->byResult )
	{
		ASSERT(0);
	}

	Interlocked_Exchange((LONG*)&b_connected, TRUE);	// 设置连接

	return 0;
}

VOID verify_policy_own::update_proof_list()
{
	//! 从列表中取出验证数据
	s_proof_result result;
	string str_passward;

	s_player_proof_data* p_data = list_proof_data_.pop_front();

	while( VALID_POINT(p_data) )
	{
		result.dw_client_id = p_data->dw_client_id;
		BOOL b_ret = proof_database_.query_account(p_data->str_accout_name.c_str(), str_passward, &result);

		
		BOOL b_need_center_proof = FALSE;

		INT n_ret = E_Success;
		if( b_ret )
		{
			//! 密码是否正确
			if( str_passward != p_data->str_password )
			{
				n_ret = E_ProofResult_Account_No_Match;
				b_need_center_proof = TRUE;
			}
			else if( epls_off_line != result.e_login_status )
			{
				n_ret = E_ProofResult_Account_In_Use;
			}
			else if( result.dw_frobid_mask != 0  )
			{
				do{
					if(result.dw_frobid_mask & eplm_member_center)
					{
						n_ret = E_ProofResult_Forbid_MemberCenter;						
						break;
					}
					if(result.dw_frobid_mask & eplm_gm_tool)
					{
						
						n_ret = E_ProofResult_Forbid_GMTool;						
						break;
					}
					if(result.dw_frobid_mask & eplm_cell_phone)
					{
						n_ret = E_ProofResult_Forbid_CellPhone;						
						break;
					}
					if(result.dw_frobid_mask & eplm_mibao)
					{
						n_ret = E_ProofResult_Forbid_MiBao;						
						break;
					}
					if(result.dw_frobid_mask & eplm_waigua)
					{
						n_ret = E_ProofResult_Disabled;						
						break;
					}
				}while(0);
			}
		}
		else
		{
			n_ret = E_ProofResult_Account_No_Match;
			b_need_center_proof = TRUE;
		}

		//! 如果需要激活但现在没有连接
		if( b_need_center_proof && !b_connected )
		{
			n_ret = E_SelectWorld_Server_Maintenance;
			b_need_center_proof = FALSE;
		}

		if( b_need_center_proof )
		{
			
			tagNC_USERLOGIN send;

			send.dwLoginID	=	g_login.get_section_id();
			send.dw_client_id	=	p_data->dw_client_id;
			strncpy(send.sz_guid, p_data->str_guid.c_str(), X_STRING_LENTH);
			strncpy(send.sz_account_name, p_data->str_accout_name.c_str(), X_STRING_LENTH);
			strncpy(send.sz_passward, p_data->str_password.c_str(), X_STRING_LENTH);

			p_transport->send_msg(&send, send.dw_size);
		}
		else
		{
			
			(fn_callback_)(n_ret, &result);
		}
		
		SAFE_DELETE(p_data);
		p_data = list_proof_data_.pop_front();		
	}
}


VOID verify_policy_own::update_session()
{
	if( NULL == p_transport )
		return;

	
	if(!p_transport->is_connect() && !login_server::p_thread->is_thread_active(_T("ProofPolicyOwnConnectServer")))
	{
		Interlocked_Exchange((LONG*)&b_terminate_connect, TRUE);
		Interlocked_Exchange((LONG*)&b_connected, FALSE);
		p_transport->disconnect();

		login_server::p_thread->waitfor_thread_destroy(_T("ProofPolicyOwnConnectServer"), INFINITE);

		
		Interlocked_Exchange((LONG*)&b_terminate_connect, FALSE);
		login_server::p_thread->create_thread(_T("ProofPolicyOwnConnectServer"), &verify_policy_own::static_thread_connect_server, this);

		while(FALSE == login_server::p_thread->is_thread_active(_T("ProofPolicyOwnConnectServer")))
		{
			login_server::p_thread->create_thread(_T("ProofPolicyOwnConnectServer"), &verify_policy_own::static_thread_connect_server, this);
			continue;
		}

		return;
	}

	
	DWORD	dw_size = 0;
	LPBYTE	p_recv = p_transport->recv_msg(dw_size);

	while( VALID_POINT(p_recv) )
	{
		
		serverframe::net_command_manager::get_singleton().handle_message((tag_net_message*)p_recv, dw_size, INVALID_VALUE);
		p_transport->free_recv_msg(p_recv);

		
		p_recv = p_transport->recv_msg(dw_size);
	}
}


UINT verify_policy_own::thread_connect_server()
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
				p_transport->try_create_connect(sz_ip, n_port);
			}

			Sleep(100);
			continue;	
		}

		print_message(_T("Contected to center server at %s: %d\r\n"), get_tool()->unicode8_to_unicode(sz_ip), n_port);

		
		tagNC_LOGIN	send;
		send.dwLoginID = g_login.get_section_id();
		p_transport->send_msg(&send, send.dw_size);

		break;
	}

//#ifdef _DEBUG
	THROW_EXCEPTION_END;
//#endif
	_endthreadex(0);

	return 0;
}

UINT verify_policy_own::static_thread_connect_server(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	verify_policy_own* p_this = (verify_policy_own*)p_data;
	return p_this->thread_update();
}


//-------------------------------------------------------------------------
DWORD verify_policy_own::handle_user_login(tag_net_message* p_cmd, DWORD)
{
	tagNS_USERLOGIN* p_recv = (tagNS_USERLOGIN*)p_cmd;

	s_proof_result result;
	result.dw_client_id = p_recv->dw_client_id;

	
	INT n_ret = E_Success;

	//! 帐号不存在
	if( 2 == p_recv->byResult )	
	{
		n_ret = E_ProofResult_Account_No_Match;
	}
	//! 密码错误
	else if( 1 == p_recv->byResult )	
	{
		n_ret = E_ProofResult_Account_No_Match;
	
		//! 只尝试更新数据库，而不插入，因为如果玩家是第一次登陆，此时中央激活并没有将玩家账号放入激活列表，可能会导致划拨时没有该区记录
		proof_database_.update_accout(p_recv->sz_account_name, p_recv->sz_passward, p_recv->sz_mibao, p_recv->byGuard);
	}
	//! 密码正确
	else if( 0 == p_recv->byResult )	
	{
		//! 尝试插入，如果插入失败则尝试更新
		BOOL b_ret = proof_database_.insert_accout(p_recv->sz_account_name, p_recv->sz_passward, p_recv->sz_mibao, p_recv->byGuard);
		if( !b_ret )
		{
			b_ret = proof_database_.update_accout(p_recv->sz_account_name, p_recv->sz_passward, p_recv->sz_mibao, p_recv->byGuard);
		}
	}
	else
	{
		n_ret = E_ProofResult_Account_No_Match;
	}

	//! 是否再需要重新查询
	if( E_ProofResult_Account_No_Match != n_ret )
	{
		string str_passward;
		BOOL b_ret = proof_database_.query_account(p_recv->sz_account_name, str_passward, &result);

		if( b_ret )
		{
			if( epls_off_line != result.e_login_status )
			{
				n_ret = E_ProofResult_Account_In_Use;
			}
			else if( p_recv->byState == 1 )
			{
				//! 被封停了
				g_datebase.forbid_account(result.dw_account_id,p_recv->byBlocktype);
				
				proof_database_.query_account(p_recv->sz_account_name, str_passward, &result);
			}
			//! 如果封停掩码不为零 且 当前登录状态并未改变成E_ProofResult_Account_In_Use
			if( (result.dw_frobid_mask != 0)  && (n_ret != E_ProofResult_Account_In_Use) )
			{
				do{
					if(result.dw_frobid_mask & eplm_member_center)
					{
						n_ret = E_ProofResult_Forbid_MemberCenter;						
						break;
					}
					if(result.dw_frobid_mask & eplm_gm_tool)
					{
						
						n_ret = E_ProofResult_Forbid_GMTool;						
						break;
					}
					if(result.dw_frobid_mask & eplm_cell_phone)
					{
						n_ret = E_ProofResult_Forbid_CellPhone;						
						break;
					}
					if(result.dw_frobid_mask & eplm_mibao)
					{
						n_ret = E_ProofResult_Forbid_MiBao;						
						break;
					}
					if(result.dw_frobid_mask & eplm_waigua)
					{
						n_ret = E_ProofResult_Disabled;						
						break;
					}
				}while(0);
			}
		}
		else
		{
			n_ret = E_ProofResult_Account_No_Match;
		}
	}

	
	(fn_callback_)(n_ret, &result);



	return 0;
}

//-------------------------------------------------------------------------
//! 玩家更新密码
//-------------------------------------------------------------------------
DWORD verify_policy_own::handle_user_update_password(tag_net_message* p_cmd, DWORD)
{
	tagNS_USERUPDATEPWD* p_recv = (tagNS_USERUPDATEPWD*)p_cmd;

	
	BOOL b_ret = proof_database_.update_password(p_recv->sz_account_name, p_recv->sz_passward);

	
	tagNC_USERUPDATEPWD send;
	send.dwLoginID = g_login.get_section_id();
	send.byResult = (b_ret ? 0 : 1);
	memcpy(send.sz_guid,  p_recv->sz_guid, X_STRING_LENTH);
	memcpy(send.sz_account_name, p_recv->sz_account_name, X_STRING_LENTH);

	p_transport->send_msg(&send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------
//! 玩家绑定密保
//-------------------------------------------------------------------------
DWORD verify_policy_own::handle_user_bind_mibao(tag_net_message* p_cmd, DWORD)
{
	tagNS_USERBINDMIBAO* p_recv = (tagNS_USERBINDMIBAO*)p_cmd;

	
	BOOL b_ret = proof_database_.update_mibao(p_recv->sz_account_name, p_recv->sz_mibao);

	
	tagNC_USERBINDMIBAO send;
	send.dwLoginID = g_login.get_section_id();
	send.byResult = (b_ret ? 0 : 1);
	memcpy(send.sz_guid, p_recv->sz_guid, X_STRING_LENTH);
	memcpy(send.sz_account_name, p_recv->sz_account_name, X_STRING_LENTH);

	p_transport->send_msg(&send, send.dw_size);


	return 0;
}

//-------------------------------------------------------------------------
//! 取消绑定密保
//-------------------------------------------------------------------------
DWORD verify_policy_own::handle_user_unbind_mibao(tag_net_message* p_cmd, DWORD)
{
	tagNS_USERUNBINDMIBAO* p_recv = (tagNS_USERUNBINDMIBAO*)p_cmd;

	
	BOOL b_ret = proof_database_.update_mibao(p_recv->sz_account_name, p_recv->sz_mibao);

	
	tagNC_USERUNBINDMIBAO send;
	send.dwLoginID = g_login.get_section_id();
	send.byResult = (b_ret ? 0 : 1);
	memcpy(send.sz_guid, p_recv->sz_guid, X_STRING_LENTH);
	memcpy(send.sz_account_name, p_recv->sz_account_name, X_STRING_LENTH);

	p_transport->send_msg(&send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------
//! 账号封停
//-------------------------------------------------------------------------
DWORD verify_policy_own::handle_block_account(tag_net_message* p_cmd, DWORD)
{
	tagNS_BLOCKACCOUNT* p_recv = (tagNS_BLOCKACCOUNT*)p_cmd;

	
	s_proof_result result;
	string str_passward;
	BOOL b_ret = proof_database_.query_account(p_recv->sz_account_name, str_passward, &result);

	if(b_ret)
	{
		DWORD dwRet = g_datebase.forbid_account(result.dw_account_id,p_recv->byBlockType);
		b_ret = ((dwRet==E_Success) ? 1 : 0);
	}

	tagNC_BLOCKACCOUNT send;
	send.dwLoginID = g_login.get_section_id();
	send.byResult = (b_ret ? 0 : 1);
	memcpy(send.sz_guid, p_recv->sz_guid, X_STRING_LENTH);
	memcpy(send.sz_account_name, p_recv->sz_account_name, X_STRING_LENTH);
	p_transport->send_msg(&send, send.dw_size);


	return 0;
}
//-------------------------------------------------------------------------
//! 账号解封
//-------------------------------------------------------------------------
DWORD verify_policy_own::handle_unblock_account(tag_net_message* p_cmd, DWORD)
{
	tagNS_UNBLOCKACCOUNT* p_recv = (tagNS_UNBLOCKACCOUNT *)p_cmd;

	
	s_proof_result result;
	string str_passward;
	BOOL b_ret = proof_database_.query_account(p_recv->sz_account_name, str_passward, &result);

	if(b_ret)
	{
		DWORD dwRet = g_datebase.remove_account_forbid(result.dw_account_id,p_recv->byUnBlockType);
		b_ret = ((dwRet==E_Success) ? 1 : 0);
	}

	tagNC_UNBLOCKACCOUNT send;
	send.dwLoginID = g_login.get_section_id();
	send.byResult = (b_ret ? 0 : 1);
	memcpy(send.sz_guid, p_recv->sz_guid, X_STRING_LENTH);
	memcpy(send.sz_account_name, p_recv->sz_account_name, X_STRING_LENTH);
	p_transport->send_msg(&send, send.dw_size);


	return 0;
}

//-------------------------------------------------------------------------
//! 设置防沉迷
//-------------------------------------------------------------------------
DWORD verify_policy_own::handle_set_chenmi(tag_net_message* p_cmd, DWORD)
{
	tagNS_USERUPDATECHENMI* p_recv = (tagNS_USERUPDATECHENMI*)p_cmd;

	
	s_proof_result result;
	string str_passward;
	BOOL b_ret = proof_database_.query_account(p_recv->sz_account_name, str_passward, &result);

	if(b_ret)
	{
		DWORD dwRet = g_datebase.set_account_chenmi(result.dw_account_id,p_recv->byChenMi);
		b_ret = ((dwRet==E_Success) ? 1 : 0);
	}

	tagNC_USERUPDATECHENMI send;
	send.dwLoginID = g_login.get_section_id();
	send.byResult = (b_ret ? 0 : 1);
	memcpy(send.sz_guid, p_recv->sz_guid, X_STRING_LENTH);
	memcpy(send.sz_account_name, p_recv->sz_account_name, X_STRING_LENTH);
	p_transport->send_msg(&send, send.dw_size);


	return 0;
}



//-------------------------------------------------------------------------
VOID verify_policy_own::proof(DWORD dw_client_id, LPCSTR sz_accout_name, LPCSTR sz_passward, LPCSTR szGUILD, INT nType)
{
	if( !VALID_POINT(dw_client_id) || !VALID_POINT(sz_accout_name) ) return;

	//! 生成验证信息
	s_player_proof_data* p_data = new s_player_proof_data(dw_client_id, sz_accout_name, sz_passward, szGUILD);

	list_proof_data_.push_back(p_data);
}


//-------------------------------------------------------------------------
INT verify_policy_own::get_proof_server_status()
{
	if (VALID_POINT(p_transport) && p_transport->is_connect())
	{
		return ews_well;
	}

	return ews_proof_error;
}
