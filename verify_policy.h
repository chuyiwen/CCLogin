/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _VERFY_POLICY_H_
#define _VERFY_POLICY_H_


//-----------------------------------------------------------------------------
// ÑéÖ¤²ßÂÔ
//-----------------------------------------------------------------------------
class verify_policy
{
protected:

	verify_policy() {}
	virtual ~verify_policy() {}

public:

	virtual BOOL	init(PROOFCALLBACK fn) = 0;
	virtual VOID	destroy() = 0;


	virtual INT get_proof_server_status() = 0;

	virtual VOID	proof(DWORD dw_client_id, LPCSTR sz_accout_name, LPCSTR sz_password, LPCSTR sz_guid, INT nType) = 0;

	VOID	set_proof_callback(PROOFCALLBACK fn)		{ fn_callback_ = fn; }


protected:
	PROOFCALLBACK			fn_callback_;					
};

#endif