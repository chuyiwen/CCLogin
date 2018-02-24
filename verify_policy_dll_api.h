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

//迅雷   用于加密数据，deData为待加密串，nBufLen为待加密串的长度，enData为加密后串；
typedef int ( *PROOF_XUNLEI_AESENCRYPT)(const char *deData, char *enData, int nBufLen);
//迅雷   用于解密数据，enData为解密前的串，deData为解密后的串。
typedef void ( *PROOF_XUNLEI_AESDECRYPT)(const char *enData, char *deData);

extern void InitProofXunleiInterface(HINSTANCE);

extern PROOF_XUNLEI_AESENCRYPT        PROOF_XUNLEI_AesEncrypt;
extern PROOF_XUNLEI_AESDECRYPT        PROOF_XUNLEI_AesDecrypt;

#endif