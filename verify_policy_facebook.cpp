/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/

#include "stdafx.h"

#include "verify_policy_facebook.h"
#include "si_login_server.h"

BOOL verify_policy_facebook_database::init()
{
	if( !VALID_POINT(login_server::p_var_config) ) return FALSE;

	BOOL b_ret = database_.init_db(login_server::p_var_config->get_string(_T("ip database")), 
		login_server::p_var_config->get_string(_T("user database")),
		login_server::p_var_config->get_string(_T("psd database")), 
		login_server::p_var_config->get_string(_T("name database")),
		(INT)login_server::p_var_config->get_dword(_T("port database")) );

	return b_ret;
}

DWORD verify_policy_facebook_database::query_account_id(LPCSTR szGUID, LPCSTR szCode, INT nType)
{
	if( !VALID_POINT(szGUID) || !VALID_POINT(szCode) ) 
		return INVALID_VALUE;

	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return INVALID_VALUE;

	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	p_stream->select_item("account_third", "account_id, code");
	p_stream->where_item();
	p_stream->write_string("third_id=") << _atoi64(szGUID);
	p_stream->write_string(" and third_type=") << nType;

	database_.return_use_connect(p_con);

	execute_result* p_result = database_.sql_query(p_stream);

	database_.return_io(p_stream);


	if( !VALID_POINT(p_result) || p_result->get_row_count() <= 0)
	{	
		database_.free_result_query(p_result);
		return INVALID_VALUE;
	}

	DWORD dwAccountID = INVALID_VALUE;
	if(strcmp((*p_result)[1].get_string(), szCode) == 0){
		dwAccountID = (*p_result)[0].get_dword();
	}
	

	database_.free_result_query(p_result);

	return dwAccountID;
}

BOOL verify_policy_facebook_database::query_account_by_id(DWORD dwAccountID, s_proof_result* p_proof_result, CHAR *accountName)
{
	if( !VALID_POINT(p_proof_result) || !VALID_POINT(accountName) ) return FALSE;

	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	char sz_ip[X_IP_LEN] = "";
	char sz_date_time[X_DATATIME_LEN + 1]= "";

	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	p_stream->select_item("account", "id,forbid_mask,privilege,guard,login_status,world_name_crc,mibao,ip,time,special_flag, name");
	p_stream->where_item();
	p_stream->write_string("id=") << dwAccountID;


	database_.return_use_connect(p_con);

	execute_result* p_result = database_.sql_query(p_stream);


	database_.return_io(p_stream);


	if( !VALID_POINT(p_result) || p_result->get_row_count() <= 0)
	{	
		database_.free_result_query(p_result);
		return FALSE;
	}


	p_proof_result->dw_account_id		=	(*p_result)[0].get_dword();
	p_proof_result->dw_frobid_mask		=	(*p_result)[1].get_dword();
	p_proof_result->by_privilege		=	(*p_result)[2].get_byte();
	p_proof_result->b_guard				=	(*p_result)[3].get_bool();
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

	memcpy(accountName, (*p_result)[10].get_string(), sizeof(CHAR)*16);

	database_.free_result_query(p_result);

	return TRUE;
}
/**  验证策略 */
BOOL verify_policy_facebook::init(PROOFCALLBACK fn)
{

	set_proof_callback(fn);		


	if( FALSE == proof_database_.init() )
	{
		return FALSE;
	}


	b_terminate_update = FALSE;

	if( FALSE == login_server::p_thread->create_thread( _T("thread_update_proof_facebook"), 
		&verify_policy_facebook::static_thread_update, this) )
		return FALSE;

	return TRUE;
}


VOID verify_policy_facebook::destroy()
{

	Interlocked_Exchange((LPLONG)&b_terminate_update, TRUE);
	login_server::p_thread->waitfor_thread_destroy( _T("thread_update_proof_facebook"), INFINITE);


	s_user_proof_data* p_data = list_proof_data_.pop_front();

	while( VALID_POINT(p_data) )
	{
		SAFE_DELETE(p_data);

		p_data = list_proof_data_.pop_front();
	}
}


UINT verify_policy_facebook::thread_update()
{
	BOOL b_ret = FALSE;
	DWORD dw_time = 0, dwAccountID;
	INT n_ret = E_Success;

	_set_se_translator(serverdump::si_translation); 

	try
	{
		//#endif
		while( !b_terminate_update )
		{

			dw_time = timeGetTime();


			s_proof_result_third third;


			s_user_proof_data* p_data = list_proof_data_.pop_front();

			while( VALID_POINT(p_data) )
			{
				third.result.dw_client_id = p_data->dw_client_id;

				dwAccountID = proof_database_.query_account_id(p_data->strGUID.c_str(), p_data->strCode.c_str(), p_data->nType);

				if(!VALID_POINT(dwAccountID)){
					b_ret = FALSE;
				}else{
					b_ret = proof_database_.query_account_by_id(dwAccountID, &third.result, third.szAccount);
				}


				if( b_ret )
				{
					n_ret = E_Success;

					if( epls_off_line != third.result.e_login_status )
					{
						n_ret = E_ProofResult_Account_In_Use;
					}
					else if( third.result.dw_frobid_mask != 0 )
					{
						do{
							if(third.result.dw_frobid_mask & eplm_member_center)
							{
								n_ret = E_ProofResult_Forbid_MemberCenter;						
								break;
							}
							if(third.result.dw_frobid_mask & eplm_gm_tool)
							{

								n_ret = E_ProofResult_Forbid_GMTool;						
								break;
							}
							if(third.result.dw_frobid_mask & eplm_cell_phone)
							{
								n_ret = E_ProofResult_Forbid_CellPhone;						
								break;
							}
							if(third.result.dw_frobid_mask & eplm_mibao)
							{
								n_ret = E_ProofResult_Forbid_MiBao;						
								break;
							}
							if(third.result.dw_frobid_mask & eplm_waigua)
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

				(fn_callback_)(n_ret, &third);

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

UINT verify_policy_facebook::static_thread_update(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	verify_policy_facebook* p_this = (verify_policy_facebook*)p_data;
	return p_this->thread_update();
}


VOID verify_policy_facebook::proof(DWORD dw_client_id, LPCSTR szGUID, LPCSTR szCode, LPCSTR unuse, INT nType)
{
	if( !VALID_POINT(dw_client_id) || !VALID_POINT(szGUID) || !VALID_POINT(szCode)) return;

	s_user_proof_data* p_data = new s_user_proof_data(dw_client_id, szGUID,szCode, nType);

	list_proof_data_.push_back(p_data);
}