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
#include "verify_policy.h"
#include "verify_policy_none_database.h"
#include "verify_policy_own_database.h"
#include "verify_policy_facebook.h"
#include "verify_policy_mgr.h"

verify_policy_mgr g_verify_policy_mgr;

verify_policy_mgr::~verify_policy_mgr()
{
	/*destroy();*/
}


BOOL verify_policy_mgr::init()
{
	register_auto_factory_policy();

	return TRUE;
}


VOID verify_policy_mgr::destroy()
{
	unregister_auto_factory_policy();
}


VOID verify_policy_mgr::register_auto_factory_policy()
{
	auto_factory_policy_.register_class(wrap<verify_policy_none>(),	epp_test);
	auto_factory_policy_.register_class(wrap<verify_policy_own>(),	epp_own);
	auto_factory_policy_.register_class(wrap<verify_policy_facebook>(),	epp_facebook);


}


VOID verify_policy_mgr::unregister_auto_factory_policy()
{
	auto_factory_policy_.unregister_class(wrap<verify_policy_none>(),	epp_test);
	auto_factory_policy_.unregister_class(wrap<verify_policy_own>(),	epp_own);
	auto_factory_policy_.unregister_class(wrap<verify_policy_facebook>(),	epp_facebook);
}


verify_policy* verify_policy_mgr::create_proof_policy(E_proof_policy e_policy)
{
	return auto_factory_policy_.create_class(e_policy);
}


VOID verify_policy_mgr::destroy_proof_policy(E_proof_policy e_policy, verify_policy* p_policy)
{
	if( !VALID_POINT(p_policy) ) return;

	auto_factory_policy_.destroy_class(p_policy, e_policy);
}