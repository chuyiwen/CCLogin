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

#define X_STRING_LENTH	50			// 字符串长度

//-----------------------------------------------------------------------------
// 初始登陆ID
//-----------------------------------------------------------------------------
NET_CMD_BEGIN(NC_LOGIN)
	DWORD	dwLoginID;				// login ID
NET_CMD_END

NET_CMD_BEGIN(NS_LOGIN)
	BYTE	byResult;				// 返回结果
NET_CMD_END

//------------------------------------------------------------------------------
// 心跳消息
//------------------------------------------------------------------------------
NET_CMD_BEGIN(NC_HEARTBEAT)
	DWORD	dwLoginID;					// login ID
NET_CMD_END

//------------------------------------------------------------------------------
// 玩家登入
//------------------------------------------------------------------------------
NET_CMD_BEGIN(NC_USERLOGIN)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// 帐号名
	CHAR	sz_passward[X_STRING_LENTH];			// 密码
	DWORD	dw_client_id;						// 序列号
NET_CMD_END

NET_CMD_BEGIN(NS_USERLOGIN)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	CHAR	sz_passward[X_STRING_LENTH];			// 新的密码
	BYTE	byResult;						// 验证结果
	BYTE	byState;						// 0 用户状态正常   1用户账号被封停
	BYTE	byBlocktype;					// 1、会员中心封停4、手机锁账号游戏封停 8、密保卡账号封停
	BYTE	byGuard;						// 防沉迷
	DWORD	dw_client_id;						// 序列号
	CHAR	sz_mibao[MIBAO_LENGTH];				// 密保
NET_CMD_END

//-------------------------------------------------------------------------------
// 用户更改密码
//-------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_USERUPDATEPWD)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	CHAR	sz_passward[X_STRING_LENTH];			// 新密码
NET_CMD_END

NET_CMD_BEGIN(NC_USERUPDATEPWD)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// 结果
NET_CMD_END

//---------------------------------------------------------------------------------
// 用户绑定密保
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_USERBINDMIBAO)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	CHAR	sz_mibao[MIBAO_LENGTH];				// 密保
NET_CMD_END

NET_CMD_BEGIN(NC_USERBINDMIBAO)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// 结果
NET_CMD_END

//---------------------------------------------------------------------------------
// 用户取消密保
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_USERUNBINDMIBAO)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	CHAR	sz_mibao[MIBAO_LENGTH];				// 密保
NET_CMD_END

NET_CMD_BEGIN(NC_USERUNBINDMIBAO)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// 结果
NET_CMD_END


//---------------------------------------------------------------------------------
// 账号封停
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_BLOCKACCOUNT)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	BYTE    byBlockType;					//1、会员中心封停4、手机锁账号游戏封停 8、密保卡账号封停
NET_CMD_END

NET_CMD_BEGIN(NC_BLOCKACCOUNT)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// 结果  0 正确  1失败 
NET_CMD_END


//---------------------------------------------------------------------------------
// 账号解封
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_UNBLOCKACCOUNT)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	BYTE    byUnBlockType;					//1、会员中心解封  4、手机锁账号游戏解封  8、密保卡账号解封
NET_CMD_END

NET_CMD_BEGIN(NC_UNBLOCKACCOUNT)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// 结果
NET_CMD_END

//---------------------------------------------------------------------------------
// 账号设置防沉迷
//---------------------------------------------------------------------------------
NET_CMD_BEGIN(NS_USERUPDATECHENMI)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	DWORD	dwLoginID;						// login ID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	BYTE    byChenMi;						//0非防沉迷 1防沉迷
NET_CMD_END

NET_CMD_BEGIN(NC_USERUPDATECHENMI)
	CHAR	sz_guid[X_STRING_LENTH];			// GUID
	CHAR	sz_account_name[X_STRING_LENTH];	// 用户名
	DWORD	dwLoginID;						// login ID
	BYTE	byResult;						// 结果
NET_CMD_END



#pragma pack(pop)

#endif