/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _VERFY_POLICY_MGR_H_
#define _VERFY_POLICY_MGR_H_

//-----------------------------------------------------------------------------
//! 验证策略管理器
//-----------------------------------------------------------------------------
class verify_policy_mgr
{
public:

	verify_policy_mgr() {}
	~verify_policy_mgr();

public:
	BOOL			init();
	VOID			destroy();

	verify_policy*	create_proof_policy(E_proof_policy e_policy);
	VOID			destroy_proof_policy(E_proof_policy e_policy, verify_policy* p_policy);

private:
	VOID			register_auto_factory_policy();
	VOID			unregister_auto_factory_policy();

private:
	class_template_factory<verify_policy>			auto_factory_policy_;		// 验证策略工厂
};

extern verify_policy_mgr g_verify_policy_mgr;

#endif