/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _LOGIN_SERVER_DEFINE_H_
#define _LOGIN_SERVER_DEFINE_H_


#include "../common/ServerDefine/login_server_define.h"

#define MIBAO_LENGTH		240			// 密保卡程度

const INT CON_LOST			= -1;	// 连接断开标志

//------------------------------------------------------------------------------
struct s_world_data 
{
	BOOL			b_valid;					//! 当前是否连接
	TCHAR			sz_name[X_SHORT_NAME];	    //! World名称
	DWORD			dw_name_crc;				//! 名字CRC值
	E_world_status	e_status;					//! 当前状态
	INT				n_port;						//! 端口
	DWORD			dw_ip;						//! IP
	INT				n_max_online_num;			//! 最大在线人数
	INT				n_cur_online_num;			//! 当前在线人数
	BOOL			b_auto_seal;				//! 是否自动
	DWORD			dw_world_id;				//! worldid

	s_world_data()
	{
		ZeroMemory(this, sizeof(*this));
	}
};


//----------------------------------------------------------------
struct s_player_data 
{
	DWORD				dw_client_id;						
	DWORD				dw_cd_index;						//! 网络ID
	DWORD				dw_account_id;						
	CHAR				sz_account_name[X_SHORT_NAME];		
	DWORD				dw_mibao_crc;						//! 密保crc
//	BOOL				bDisabled;							// 是否封停
	DWORD				dw_frobid_mask;						//! 封停掩码 每1位对应一种封停类型
	BOOL				b_guard;							//! 防沉迷用户
	INT					n_acc_online_sec;					//! 累计登录时间
	DWORD				dw_ip;								//! 客户端IP
	DWORD				dw_world_name_crc;					//! 游戏世界名称CRC值
	BYTE				by_privilege;						//! 权限
	BYTE				by_SpecialAccount;
	BYTE				by_reserved[2];						//! 对齐标志

	s_player_data()		{ ZeroMemory(this, sizeof(*this)); }
	VOID Reset()		{ ZeroMemory(this, sizeof(*this)); }
};


//----------------------------------------------------------------
struct s_account_data
{
	CHAR				sz_account_name[X_SHORT_NAME];
	DWORD				dw_ip;
	BOOL				b_guard;
};

#endif