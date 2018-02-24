/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _PROOF_Dll_INTERFACE_H_
#define _PROOF_Dll_INTERFACE_H_

//Ѹ��   ���ڼ������ݣ�deDataΪ�����ܴ���nBufLenΪ�����ܴ��ĳ��ȣ�enDataΪ���ܺ󴮣�
typedef int ( *PROOF_XUNLEI_AESENCRYPT)(const char *deData, char *enData, int nBufLen);
//Ѹ��   ���ڽ������ݣ�enDataΪ����ǰ�Ĵ���deDataΪ���ܺ�Ĵ���
typedef void ( *PROOF_XUNLEI_AESDECRYPT)(const char *enData, char *deData);

extern void InitProofXunleiInterface(HINSTANCE);

extern PROOF_XUNLEI_AESENCRYPT        PROOF_XUNLEI_AesEncrypt;
extern PROOF_XUNLEI_AESDECRYPT        PROOF_XUNLEI_AesDecrypt;

#endif