/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _VERFY_POLICY_OWN_DEFINE_H_
#define _VERFY_POLICY_OWN_DEFINE_H_

#pragma pack(push, 1)

#define X_STRING_LENTH	50			// �ַ�������

//-----------------------------------------------------------------------------
// ��ʼ��½ID
//-----------------------------------------------------------------------------
NET_CMD_BEGIN(NC_LOGIN)
	DWORD	dwLoginID;				// login ID
NET_CMD_END

NET_CMD_BEGIN(NS_LOGIN)
	BYTE	byResult;				// ���ؽ��
NET_CMD_END

//------------------------------------------------------------------------------
// ������Ϣ
//------------------------------------------------------------------------------
NET_CMD_BEGIN(NC_HEARTBEAT)
	DWORD	dwLoginID;					// login ID
NET_CMD_END

//------------------------------------------------------------------------------
// ��ҵ���
//------------------------------------------------------------------------------
NET_CMD_BEGIN(NC_USERLOGIN)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// �ʺ���
	CHAR	sz_passward[X_STRING_LENTH];			// ����
	DWORD	dw_client_id;						// ���к�
NET_CMD_END

NET_CMD_BEGIN(NS_USERLOGIN)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	CHAR	sz_passward[X_STRING_LENTH];			// �µ�����
	BYTE	byResult;						// ��֤���
	BYTE	byState;						// 0 �û�״̬����   1�û��˺ű���ͣ
	BYTE	byBlocktype;					// 1����Ա���ķ�ͣ4���ֻ����˺���Ϸ��ͣ 8���ܱ����˺ŷ�ͣ
	BYTE	byGuard;						// ������
	DWORD	dw_client_id;						// ���к�
	CHAR	sz_mibao[MIBAO_LENGTH];				// �ܱ�
NET_CMD_END

//-------------------------------------------------------------------------------
// �û���������
//-------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_USERUPDATEPWD)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	CHAR	sz_passward[X_STRING_LENTH];			// ������
NET_CMD_END

NET_CMD_BEGIN(NC_USERUPDATEPWD)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// ���
NET_CMD_END

//---------------------------------------------------------------------------------
// �û����ܱ�
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_USERBINDMIBAO)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	CHAR	sz_mibao[MIBAO_LENGTH];				// �ܱ�
NET_CMD_END

NET_CMD_BEGIN(NC_USERBINDMIBAO)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// ���
NET_CMD_END

//---------------------------------------------------------------------------------
// �û�ȡ���ܱ�
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_USERUNBINDMIBAO)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	CHAR	sz_mibao[MIBAO_LENGTH];				// �ܱ�
NET_CMD_END

NET_CMD_BEGIN(NC_USERUNBINDMIBAO)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// ���
NET_CMD_END


//---------------------------------------------------------------------------------
// �˺ŷ�ͣ
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_BLOCKACCOUNT)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	BYTE    byBlockType;					//1����Ա���ķ�ͣ4���ֻ����˺���Ϸ��ͣ 8���ܱ����˺ŷ�ͣ
NET_CMD_END

NET_CMD_BEGIN(NC_BLOCKACCOUNT)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// ���  0 ��ȷ  1ʧ�� 
NET_CMD_END


//---------------------------------------------------------------------------------
// �˺Ž��
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_UNBLOCKACCOUNT)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	BYTE    byUnBlockType;					//1����Ա���Ľ��  4���ֻ����˺���Ϸ���  8���ܱ����˺Ž��
NET_CMD_END

NET_CMD_BEGIN(NC_UNBLOCKACCOUNT)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// ���
NET_CMD_END

//---------------------------------------------------------------------------------
// �˺����÷�����
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_USERUPDATECHENMI)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	BYTE    byChenMi;						//0�Ƿ����� 1������
NET_CMD_END

NET_CMD_BEGIN(NC_USERUPDATECHENMI)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// �û���
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// ���
NET_CMD_END



#pragma pack(pop)

#endif