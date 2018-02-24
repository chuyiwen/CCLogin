/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _VERFY_POLICY_NONE_DATABSET_H_
#define _VERFY_POLICY_NONE_DATABSET_H_

#include "verify_policy.h"

//-----------------------------------------------------------------------------
//! 该验证策略的数据库
//-----------------------------------------------------------------------------
class verify_policy_none_database
{
public:
	verify_policy_none_database() {}
	~verify_policy_none_database() {}

	BOOL	init();

	BOOL	query_account(LPCSTR sz_account_name,LPCSTR sz_password, s_proof_result* p_result);
	
	BOOL	regist_account(LPCSTR sz_account_name, LPCSTR sz_password);

private:
	db_interface		database_;

	few_connect_client	trans_ip;
};

//-----------------------------------------------------------------------------
//! 验证策略
//-----------------------------------------------------------------------------
class verify_policy_none : public verify_policy
{
public:

	verify_policy_none() {}
	virtual ~verify_policy_none() { }

public:

	//-------------------------------------------------------------------------
	BOOL	init(PROOFCALLBACK fn);
	VOID	destroy();


	//-------------------------------------------------------------------------
	// 账号验证
	VOID	proof(DWORD dw_client_id, LPCSTR sz_accout_name, LPCSTR sz_password, LPCSTR sz_guid, INT nType = 0);
	
	// 账号注册
	VOID	regist(DWORD dw_client_id, LPCSTR sz_account_name, LPCSTR sz_password);
	//-------------------------------------------------------------------------
	INT		get_proof_server_status();


	//-------------------------------------------------------------------------
	UINT	thread_update();
	static UINT WINAPI static_thread_update(LPVOID p_data);

private:
	


	//-------------------------------------------------------------------------
	volatile BOOL					b_terminate_update;


	//-------------------------------------------------------------------------
	struct s_user_proof_data
	{
		DWORD		dw_client_id;
		string		strAccoutName;
		string		strPsd; 

		s_user_proof_data(DWORD _dw_client_id, LPCSTR sz_accout_name,LPCSTR sz_password)
		{
			dw_client_id		=	_dw_client_id;
			strAccoutName	=	sz_accout_name;
			strPsd	=	sz_password;
		}
	};
	
	struct s_user_regist_data
	{
		DWORD		dw_client_id;
		string		strAccoutName;
		string		strPsd; 

		s_user_regist_data(DWORD _dw_client_id, LPCSTR sz_accout_name,LPCSTR sz_password)
		{
			dw_client_id		=	_dw_client_id;
			strAccoutName	=	sz_accout_name;
			strPsd	=	sz_password;
		}
	};


	safe_mutex_list<s_user_proof_data*>	list_proof_data_;		
	safe_mutex_list<s_user_regist_data*> list_regist_data_;
	//--------------------------------------------------------------------------
	verify_policy_none_database					proof_database_;				
};

#endif