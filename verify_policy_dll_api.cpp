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
#include "verify_policy_dll_api.h"

PROOF_XUNLEI_AESENCRYPT        PROOF_XUNLEI_AesEncrypt;

PROOF_XUNLEI_AESDECRYPT        PROOF_XUNLEI_AesDecrypt;

//初始化验证接口
void InitProofXunleiInterface(HINSTANCE hInst)
{
	PROOF_XUNLEI_AesEncrypt	   = (PROOF_XUNLEI_AESENCRYPT)GetProcAddress(hInst,"AesEncrypt");

	PROOF_XUNLEI_AesDecrypt	   = (PROOF_XUNLEI_AESDECRYPT)GetProcAddress(hInst, "AesDecrypt");	
}