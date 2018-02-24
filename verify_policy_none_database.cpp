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

#include "verify_policy_none_database.h"
#include "user_mgr.h"
#include "si_login_server.h"


BOOL verify_policy_none_database::init()
{
	if( !VALID_POINT(login_server::p_var_config) ) return FALSE;

	BOOL b_ret = database_.init_db(login_server::p_var_config->get_string(_T("ip database")), 
							login_server::p_var_config->get_string(_T("user database")),
							login_server::p_var_config->get_string(_T("psd database")), 
							login_server::p_var_config->get_string(_T("name database")),
							(INT)login_server::p_var_config->get_dword(_T("port database")) );

	return b_ret;
}

//-----------------------------------------------------------------------------
//! 数据库查询玩家
//-----------------------------------------------------------------------------
BOOL verify_policy_none_database::query_account(LPCSTR sz_account_name,LPCSTR sz_password, s_proof_result* p_proof_result)
{
	if( !VALID_POINT(sz_account_name) || !VALID_POINT(p_proof_result) ) return FALSE;

	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	char sz_ip[X_IP_LEN] = "";
	char sz_date_time[X_DATATIME_LEN + 1]= "";
	
	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	p_stream->select_item("account", "id,forbid_mask,privilege,guard,login_status,world_name_crc,mibao,ip,time,special_flag");
	p_stream->where_item();
	p_stream->write_string("name='").write_string(sz_account_name, p_con).write_string("'");
	p_stream->write_string(" AND password='").write_string(sz_password, p_con).write_string("'");

	
	database_.return_use_connect(p_con);

	execute_result* p_result = database_.sql_query(p_stream);

	
	database_.return_io(p_stream);

	
	if( !VALID_POINT(p_result) || p_result->get_row_count() <= 0) return FALSE;

	
	p_proof_result->dw_account_id		=	(*p_result)[0].get_dword();
	p_proof_result->dw_frobid_mask		=	(*p_result)[1].get_dword();
	p_proof_result->by_privilege		=	(*p_result)[2].get_byte();
	p_proof_result->b_guard			=	(*p_result)[3].get_bool();
	p_proof_result->n_guard_accu_time	=	0;
	p_proof_result->e_login_status		=	(E_player_login_status)(*p_result)[4].get_int();
	p_proof_result->dw_world_name_crc	=	(*p_result)[5].get_dword();

	
	CHAR sz_mibao[MIBAO_LENGTH] = {'\0'};
	(*p_result)[6].get_blob(sz_mibao, MIBAO_LENGTH);
	if( '\0' == sz_mibao[0] )
	{
		p_proof_result->b_need_mibao		=	false;
	}
	else
	{
		p_proof_result->b_need_mibao		=	true;
	}

	
	memcpy(sz_ip,(*p_result)[7].get_string(),(*p_result)[7].get_length());
	memcpy(sz_date_time,(*p_result)[8].get_string(),(*p_result)[8].get_length());

	p_proof_result->dw_pre_login_ip = trans_ip.stringip_to_ip(sz_ip);
	DataTime2DwordTime(p_proof_result->dw_pre_login_time,sz_date_time,(*p_result)[8].get_length());

	p_proof_result->by_SpecialAccount = (*p_result)[9].get_byte();
	
	database_.free_result_query(p_result);

	return TRUE;
}

BOOL verify_policy_none_database::regist_account( LPCSTR sz_account_name, LPCSTR sz_password )
{
	if( !VALID_POINT(sz_account_name)) return FALSE;

	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;


	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	p_stream->insert_item("account");
	p_stream->write_string("name='").write_string(sz_account_name, p_con).write_string("'");
	p_stream->write_string(",password='").write_string(sz_password, p_con).write_string("'");


	database_.return_use_connect(p_con);

	BOOL b_ret = database_.sql_execute(p_stream);


	database_.return_io(p_stream);


	return b_ret;
}


BOOL verify_policy_none::init(PROOFCALLBACK fn)
{

	set_proof_callback(fn);		

	
	if( FALSE == proof_database_.init() )
	{
		return FALSE;
	}

	
	b_terminate_update = FALSE;

	if( FALSE == login_server::p_thread->create_thread( _T("thread_update_proof_none"), 
		&verify_policy_none::static_thread_update, this) )
		return FALSE;

	return TRUE;
}


VOID verify_policy_none::destroy()
{
	
	Interlocked_Exchange((LPLONG)&b_terminate_update, TRUE);
	login_server::p_thread->waitfor_thread_destroy( _T("thread_update_proof_none"), INFINITE);

	
	s_user_proof_data* p_data = list_proof_data_.pop_front();

	while( VALID_POINT(p_data) )
	{
		SAFE_DELETE(p_data);

		p_data = list_proof_data_.pop_front();
	}


	s_user_regist_data* p_data2 = list_regist_data_.pop_front();
	while ( VALID_POINT(p_data2))
	{
		SAFE_DELETE(p_data2);

		p_data2 = list_regist_data_.pop_front();
	}
}


UINT verify_policy_none::thread_update()
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
			
			dw_time = timeGetTime();

			
			s_proof_result result;

			s_user_proof_data* p_data = list_proof_data_.pop_front();

			while( VALID_POINT(p_data) )
			{
				result.dw_client_id = p_data->dw_client_id;

				BOOL b_ret = proof_database_.query_account(p_data->strAccoutName.c_str(),p_data->strPsd.c_str(), &result);
				

				if (!b_ret && g_login.isAutoRegist())
				{
					bool b_insert = proof_database_.regist_account(p_data->strAccoutName.c_str(),p_data->strPsd.c_str());
					if( b_insert )
					{
						b_ret = proof_database_.query_account(p_data->strAccoutName.c_str(),p_data->strPsd.c_str(), &result);
					}
				}
				
				INT n_ret = E_Success;
				if( b_ret )
				{
					if( epls_off_line != result.e_login_status )
					{
						n_ret = E_ProofResult_Account_In_Use;
					}
					else if( result.dw_frobid_mask != 0 )
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

				
				(fn_callback_)(n_ret, &result);

				p_data = list_proof_data_.pop_front();		// 取出下一个
			}



			WaitForSingleObject(list_proof_data_.get_event(), 500);
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

UINT verify_policy_none::static_thread_update(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	verify_policy_none* p_this = (verify_policy_none*)p_data;
	return p_this->thread_update();
}


VOID verify_policy_none::proof(DWORD dw_client_id, LPCSTR sz_accout_name, LPCSTR sz_password, LPCSTR sz_guid, INT nType)
{
	if( !VALID_POINT(dw_client_id) || !VALID_POINT(sz_accout_name) ) return;

	s_user_proof_data* p_data = new s_user_proof_data(dw_client_id, sz_accout_name,sz_password);

	list_proof_data_.push_back(p_data);
}


INT verify_policy_none::get_proof_server_status()
{
	return ews_well;
}

VOID verify_policy_none::regist( DWORD dw_client_id, LPCSTR sz_account_name, LPCSTR sz_password )
{
	if( !VALID_POINT(dw_client_id) || !VALID_POINT(sz_account_name) ) return;

	s_user_regist_data* p_data = new s_user_regist_data(dw_client_id, sz_account_name,sz_password);

	list_regist_data_.push_back(p_data);
}
