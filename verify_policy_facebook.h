/*******************************************************************************

	Copyright 2010 by Shengshi Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	Shengshi Interactive  Co., Ltd.

*******************************************************************************/

#ifndef __VERIFY_POLICY_FACEBOOK_H__
#define __VERIFY_POLICY_FACEBOOK_H__

#include "verify_policy.h"
//-----------------------------------------------------------------------------
//! 该验证策略的数据库
//-----------------------------------------------------------------------------
class verify_policy_facebook_database
{
public:
	verify_policy_facebook_database() {}
	~verify_policy_facebook_database() {}

	BOOL	init();

	BOOL	query_account(LPCSTR sz_account_name,LPCSTR sz_password, s_proof_result* p_result);
	DWORD	query_account_id(LPCSTR szGUID, LPCSTR szCode, INT nType);
	BOOL	query_account_by_id(DWORD dwAccountID, s_proof_result* p_result, CHAR *accountName);

private:
	db_interface		database_;

	few_connect_client	trans_ip;
};


//-----------------------------------------------------------------------------
//! 验证策略
//-----------------------------------------------------------------------------
class verify_policy_facebook : public verify_policy
{
public:

	verify_policy_facebook() {}
	virtual ~verify_policy_facebook() { }

public:

	//-------------------------------------------------------------------------
	BOOL	init(PROOFCALLBACK fn);
	VOID	destroy();


	//-------------------------------------------------------------------------
	VOID	proof(DWORD dw_client_id, LPCSTR szGUID, LPCSTR szCode, LPCSTR unuse, INT nType);


	//-------------------------------------------------------------------------
	INT		get_proof_server_status(){	return ews_well; }


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
		string		strGUID;
		string		strCode; 
		INT			nType;

		s_user_proof_data(DWORD _dw_client_id, LPCSTR szGUID,LPCSTR szCode, INT Type)
			:dw_client_id(_dw_client_id),strGUID(szGUID),strCode(szCode),nType(Type)
		{
		}
	};

	safe_mutex_list<s_user_proof_data*>	list_proof_data_;		

	//--------------------------------------------------------------------------
	verify_policy_facebook_database					proof_database_;				
};



#endif /** __VERIFY_POLICY_FACEBOOK_H__ */