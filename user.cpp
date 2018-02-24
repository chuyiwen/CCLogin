/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "stdafx.h"

#include "../../common/WorldDefine/login_protocol.h"

#include "user.h"
#include "user_mgr.h"
#include "user_net_cmd_mgr.h"
#include "si_world.h"
#include "si_world_mgr.h"
#include "database.h"
#include "si_login_server.h"

//-------------------------------------------------------------------------------
user_net_cmd_mgr user::player_net_mgr;


//-------------------------------------------------------------------------------
user::user(DWORD dw_client_id, DWORD dw_cd_index, DWORD dw_ip, DWORD dw_world_id) : 
				palyer_data_(), b_conn_lost_(FALSE), n_verify_code_(0), b_proof_end_(FALSE),
				b_proofing_(FALSE), b_need_mibao_(FALSE), dw_kick_time_(), b_select_world_ok_(FALSE)
{
	palyer_data_.dw_client_id		=		dw_client_id;
	palyer_data_.dw_cd_index		=		dw_cd_index;
	palyer_data_.dw_account_id		=		INVALID_VALUE;
	palyer_data_.dw_frobid_mask		=		0;
	palyer_data_.b_guard			=		FALSE;
	palyer_data_.n_acc_online_sec		=		0;
	palyer_data_.dw_ip				=		dw_ip;
	palyer_data_.dw_world_name_crc	=		dw_world_id;
	palyer_data_.by_privilege		=		0;
}


//-------------------------------------------------------------------------------
user::~user()
{
   
}


//-------------------------------------------------------------------------------
VOID user::proof_return(s_proof_result* p_result)
{
	palyer_data_.dw_account_id		=	p_result->dw_account_id;
	palyer_data_.b_guard			=	p_result->b_guard;
	palyer_data_.n_acc_online_sec		=	p_result->n_guard_accu_time;
	palyer_data_.by_privilege		=	p_result->by_privilege;
	palyer_data_.dw_frobid_mask		=	p_result->dw_frobid_mask;
	palyer_data_.by_SpecialAccount	= p_result->by_SpecialAccount;

	

	b_need_mibao_			=	p_result->b_need_mibao;

	set_proof_end();

	
	g_PlayMgr.cache_account_name(get_account_id(), get_accout_name());
	g_PlayMgr.cache_guard(get_account_id(), get_player_data().b_guard);
	g_PlayMgr.map_account_name_to_account_id(get_accout_name(), get_account_id());
}


//-------------------------------------------------------------------------------
VOID user::register_player_msg()
{
	player_net_mgr.register_cmd("NET_C2L_proof_third",	&gandle_proof_third,	_T("NET_C2L_proof_third"));
	player_net_mgr.register_cmd("NET_C2L_proof",		&gandle_proof,			_T("NET_C2L_proof"));
	player_net_mgr.register_cmd("NET_C2L_mibao",		&handle_mibao,			_T("NET_C2L_mibao"));
	player_net_mgr.register_cmd("NET_C2L_regist_account",&handle_regist_account,_T("NET_C2L_regist_account"));
}

//-------------------------------------------------------------------------------
VOID user::unregister_player_msg()
{
	 player_net_mgr.unregister_all();
}


//-------------------------------------------------------------------------------
INT user::handle_message()
{
    
    if( b_conn_lost_ )
        return CON_LOST;

    
	DWORD	dw_size		=	0;
	INT		n_un_recved	=	0;

	LPBYTE p_message = recv_msg(dw_size, n_un_recved);
	if( !VALID_POINT(p_message) ) return 0;

	
    tag_net_message* p_cmd = (tag_net_message*)p_message;

    NETMSGHANDLER p_handler = player_net_mgr.get_handler(p_cmd, dw_size);

    if( NULL != p_handler )
        (this->*p_handler)(p_cmd);
    
	
    return_msg(p_message);

	return 0;
}


//-------------------------------------------------------------------------------
VOID user::send_message(LPVOID p_msg, DWORD dw_size)
{
	if( !VALID_POINT(p_msg) || dw_size == 0 )
		return;

	
	if( b_conn_lost_ ) return;

	send_msg((LPBYTE)p_msg, dw_size);
}


//-------------------------------------------------------------------------------
LPBYTE user::recv_msg(DWORD& dw_size, INT& n_unrecved)
{
	return g_PlayMgr.recv_msg(dw_size, n_unrecved, palyer_data_.dw_cd_index);
}

//-------------------------------------------------------------------------------
VOID user::return_msg(LPBYTE p_msg)
{
	g_PlayMgr.return_msg(p_msg);
}


//-------------------------------------------------------------------------------
VOID user::send_msg(LPBYTE p_msg, DWORD dw_size)
{
	g_PlayMgr.send_msg(palyer_data_.dw_cd_index, p_msg, dw_size);
}



//-----------------------------------------------------------------------------
DWORD user::gandle_proof(tag_net_message* p_cmd)
{
    NET_C2L_proof* p_receive = (NET_C2L_proof*)p_cmd;

	//! 防止字符串溢出
	p_receive->szUserName[X_SHORT_NAME-1] = '\0';
	p_receive->szPsd[MAX_MD5_ARRAY-1] = '\0';
	p_receive->szGUID[X_LONG_NAME-1] = '\0';

	
	INT nRet = E_Success;

	//! 验证玩家要登录的游戏世界是否可用
	si_world* p_world = g_fpWorldMgr.get_world(p_receive->dw_world_name_crc);
	if( !VALID_POINT(p_world) )
	{
		nRet = E_SelectWorld_GameWorldName_Wrong;
	}

	//! 查看玩家要登录的GameServer是否正常
	else if( p_world->get_status() != ews_well )
	{
		nRet = E_SelectWorld_Server_Maintenance;
	}

	// 查看验证策略是否一致
	//mmz
	/*else if( p_receive->dwType != PlayerMgr::getSingleton().GetProofPolicy() )
	{
		nRet = E_ProofResult_Wrong_Type;
	}*/

	// 查看版本号是否一致
	else if( p_receive->dwCurVersionID != g_PlayMgr.get_curr_ver() )
	{
		nRet = E_ProofResult_Wrong_Build_Number;
	}

	// 服务器拥挤
	//else if(p_world->get_curr_online_num() >= (INT)(p_world->get_max_online_num() * p_world->getLoginLimit()))
	//{
	//	nRet = E_SelectWorld_Server_Full;
	//}

	// 如果正在验证，或者已经验证完毕
	else if( is_proofing() || is_proof_end() )
	{
		nRet = E_ProofResult_Proofing;
	}
	//mmz
	// 检查用户名合法
	else
	{
		string strUserName = p_receive->szUserName;

		if( check_name(strUserName) )
		{
			nRet = E_ProofResult_Account_No_Match;
		}
		else
		{
			strUserName.copy(palyer_data_.sz_account_name, X_SHORT_NAME);
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP,NULL,palyer_data_.sz_account_name,-1,sz_account_name_t,X_SHORT_NAME);
#else
			strcpy(sz_account_name_t,palyer_data_.sz_account_name);
#endif
		}
	}


	if( E_Success != nRet )
	{
		NET_L2C_proof_result send;
		send.dw_error_code = nRet;
		send_message(&send, send.dw_size);
	}
	
	else
	{
		g_PlayMgr.proof(get_client_id(), get_accout_name(), p_receive->szPsd, p_receive->szGUID);
	}

	return 0;
}

DWORD user::gandle_proof_third(tag_net_message* p_cmd)
{

	NET_C2L_proof_third* p_receive = (NET_C2L_proof_third*)p_cmd;

	//! 防止字符串溢出
	p_receive->szGUID[X_SHORT_NAME-1] = '\0';
	p_receive->szCode[MAX_MD5_ARRAY-1] = '\0';


	INT nRet = E_Success;

	//! 验证玩家要登录的游戏世界是否可用
	si_world* p_world = g_fpWorldMgr.get_world(p_receive->dw_world_name_crc);
	if( !VALID_POINT(p_world) )
	{
		nRet = E_SelectWorld_GameWorldName_Wrong;
	}

	//! 查看玩家要登录的siGameServer是否正常
	else if( p_world->get_status() != ews_well )
	{
		nRet = E_SelectWorld_Server_Maintenance;
	}

	// 查看版本号是否一致
	else if( p_receive->dwCurVersionID != g_PlayMgr.get_curr_ver() )
	{
		nRet = E_ProofResult_Wrong_Build_Number;
	}

	// 如果正在验证，或者已经验证完毕
	else if( is_proofing() || is_proof_end() )
	{
		nRet = E_ProofResult_Proofing;
	}


	if( E_Success != nRet )
	{
		NET_L2C_proof_result send;
		send.dw_error_code = nRet;
		send_message(&send, send.dw_size);
	}

	else
	{
		//if(p_receive->eType == ETVT_FACEBOOK){
			//for(int n = 0; n < X_SHORT_NAME; n++) p_receive->szGUID[n] = ~p_receive->szGUID[n];
			g_PlayMgr.proof_facebook(get_client_id(), p_receive->szGUID, p_receive->szCode, p_receive->eType);
		//}
	}

	return 0;
}




DWORD user::handle_mibao(tag_net_message* p_cmd)
{
	NET_C2L_mibao* p_receive = (NET_C2L_mibao*)p_cmd;

	
	INT n_ret = E_Success;

	if( !is_proof_end() )
	{
		n_ret = E_ProofResult_Mibao_Error;
	}
	else if( !check_mibao(p_receive->dwMBCrc) )
	{
		n_ret = E_ProofResult_Mibao_Error;
	}


	if( E_Success == n_ret )
	{
		//! 加入到游戏世界登入列表
		g_PlayMgr.remove_player_from_logining(get_client_id());

		//! 加入帐号
		g_PlayMgr.add_account(get_account_id(), get_client_id());

		//! 将玩家加入到GameServer的玩家列表中
		g_fpWorldMgr.add_to_world(this, get_login_world_name_crc());

	}
	
	else
	{
		
		NET_L2C_proof_result send;
		send.dw_error_code = n_ret;
		send_message(&send, send.dw_size);
	}

	return 0;
}

DWORD user::handle_regist_account(tag_net_message* p_cmd)
{
	NET_C2L_regist_account* p_receive = (NET_C2L_regist_account*)p_cmd;
		
	//odbc::sql_language_disposal* pStream = m_pDBGame->get_io();

	return 0;
}
//-------------------------------------------------------------------------------
//! 替换回车和换行,同时检查是否为空
//-------------------------------------------------------------------------------
BOOL user::check_name(string& str)
{
	//! 去掉回车
	INT n_pos = 0;
	while( (n_pos = str.find('\r', n_pos)) != str.npos )
		str.replace(n_pos, 1, "");

	//! 去掉换行
	n_pos = 0;
	while( (n_pos = str.find('\n', n_pos)) != str.npos )
		str.replace(n_pos, 1, "");

	//! 去掉tab
	n_pos = 0;
	while( (n_pos = str.find('\t', n_pos)) != str.npos )
		str.replace(n_pos, 1, "");

	//! 去掉空格
	if((n_pos = str.find(' ')) != str.npos)
	{
		str = str.substr(0,n_pos);
	}

	//! 检查用户名是否为空
	return str.empty();
}

//--------------------------------------------------------------------------------
//! 生成验证用密保
//--------------------------------------------------------------------------------
BOOL user::generate_mibao(CHAR sz_mibao[MAX_MIBAO])
{
	//! 从数据库里面查询该玩家的密报值
	CHAR sz_mibao_in_database[MIBAO_LENGTH] = {'\0'};
	if( !g_datebase.get_mibao(get_account_id(), sz_mibao_in_database) )
	{
		return FALSE;
	}

	CHAR sz_value[MAX_MIBAO] = {'\0'};
	// 密保的格式：以逗号分隔的字符序列，8*10的矩阵
	// 例如：97,55,87,21,90,33,19,36,55,90,26,96,24,61,32,27,70,64,86,69,97,54,36,21,
	// 18,58,55,96,37,32,75,64,08,87,08,74,33,13,34,90,70,14,09,98,93,37,19,75,21,68,51,
	// 46,59,41,86,69,13,93,00,15,48,36,57,50,16,98,24,57,38,63,91,28,53,06,35,40,61,59,94,15,
	for(INT n = 0; n < 3; ++n)
	{
		INT n_rand = get_tool()->tool_rand() % 80;		// 随机值

		INT nX = n_rand / 10;	// x轴，从'A'到'H'
		INT nY = n_rand % 10;	// y轴，从'0'到'9'

		//! 向密保行列中写入行列号
		sz_mibao[n*2]	=	'A' + nX;
		sz_mibao[n*2+1]	=	'0' + nY;

		//1 向值中写入密保卡值
		sz_value[n*2]	=	sz_mibao_in_database[n_rand*3];
		sz_value[n*2+1]	=	sz_mibao_in_database[n_rand*3+1];
	}

	//! 将szValue算出CRC保存在起来
	palyer_data_.dw_mibao_crc = get_tool()->crc32((LPBYTE)sz_value, MAX_MIBAO);


	return TRUE;
}


//--------------------------------------------------------------------------------
BOOL user::check_mibao(DWORD dw_result_crc)
{
	return palyer_data_.dw_mibao_crc == dw_result_crc;
}

//--------------------------------------------------------------------------------
LPCTSTR	user::get_accout_name_t()
{
	return sz_account_name_t;
}

VOID user::SetAccountName(const CHAR* p)
{
	if(!VALID_POINT(p))
		return;

	string strUserName = p;

	{
		strUserName.copy(palyer_data_.sz_account_name, X_SHORT_NAME);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP,NULL,palyer_data_.sz_account_name,-1,sz_account_name_t,X_SHORT_NAME);
#else
		strcpy(sz_account_name_t,palyer_data_.sz_account_name);
#endif
	}
}