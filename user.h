/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _USER_H_
#define _USER_H_

#include "../../common/WorldDefine/login_protocol.h"

#include "user_net_cmd_mgr.h"

class user_net_cmd_mgr;


class user  
{
public:

	//-------------------------------------------------------------------------
	user(DWORD dw_client_id, DWORD dw_cd_index, DWORD dw_ip, DWORD dw_world_id);
	~user();


	//-------------------------------------------------------------------------
	DWORD			get_account_id()				{ return palyer_data_.dw_account_id; }
	LPCSTR			get_accout_name()				{ return palyer_data_.sz_account_name; }
	LPCTSTR			get_accout_name_t();			
	DWORD			get_client_id()					{ return palyer_data_.dw_client_id; }
	DWORD			get_cd_index()					{ return palyer_data_.dw_cd_index; }
	BYTE			get_privilege()					{ return palyer_data_.by_privilege; }
	BYTE			get_specialacc()				{ return palyer_data_.by_SpecialAccount; }
	DWORD			get_login_world_name_crc()		{ return palyer_data_.dw_world_name_crc; }
	INT				get_verify_code()				{ return n_verify_code_; }
	s_player_data&	get_player_data()				{ return palyer_data_; }

	DWORD			get_pre_login_time()			{ return dw_pre_login_time;}
	DWORD			get_pre_login_ip()				{ return dw_pre_login_ip;}


	BOOL			get_forbid_mask()				{ return palyer_data_.dw_frobid_mask; }
	BOOL			is_guard()					{ return palyer_data_.b_guard; }

	//-------------------------------------------------------------------------
	VOID			proof_return(s_proof_result* p_result);


	//-------------------------------------------------------------------------
	VOID			set_conn_lost()				{ Interlocked_Exchange((LPLONG)(&b_conn_lost_), TRUE); }
	BOOL			is_conn_lost()				{ return b_conn_lost_; }

	//-------------------------------------------------------------------------
	static VOID		register_player_msg();
	static VOID		unregister_player_msg();

	INT				handle_message();
	VOID			send_message(LPVOID p_msg, DWORD dw_size);
	

	//-------------------------------------------------------------------------
	VOID			set_pre_login_time(tagDWORDTime dw_time)	{ dw_pre_login_time = dw_time; }
	VOID			set_pre_login_ip(DWORD dw_ip)		{ dw_pre_login_ip = dw_ip; }


	VOID			set_kick_time(tagDWORDTime dw_time)	{ dw_kick_time_ = dw_time; }
	tagDWORDTime	get_kick_time()						{ return dw_kick_time_; }

	BOOL			is_need_mibao()						{ return b_need_mibao_; }

	BOOL			is_proofing()						{ return b_proofing_; }
	BOOL			is_proof_end()						{ return b_proof_end_; }
	VOID			set_proofing()						{ b_proofing_ = TRUE; b_proof_end_ = FALSE; }
	VOID			set_proof_end()						{ b_proof_end_ = TRUE; b_proofing_ = FALSE; }

	BOOL			is_select_world_ok()				{ return b_select_world_ok_; }
	VOID			set_select_world_ok()					{ b_select_world_ok_ = TRUE; }

	BOOL			generate_mibao(CHAR sz_mibao[MAX_MIBAO]);
	BOOL			check_mibao(DWORD dw_result_crc);

	VOID			SetAccountName(const CHAR* p);

private:
	//-----------------------------------------------------------------------
	DWORD			gandle_proof(tag_net_message* p_cmd);
	DWORD			gandle_proof_third(tag_net_message* p_cmd);
	DWORD			handle_mibao(tag_net_message* p_cmd);
	DWORD			handle_regist_account(tag_net_message* p_cmd);

	//-----------------------------------------------------------------------
	LPBYTE			recv_msg(DWORD& dw_size, INT& n_unrecved);
	VOID			return_msg(LPBYTE p_msg);
	VOID            send_msg(LPBYTE p_msg, DWORD dw_size);


	//-----------------------------------------------------------------------
	BOOL			check_name(string& str);

private:
	static user_net_cmd_mgr		player_net_mgr;			

private:
	s_player_data				palyer_data_;			//! 玩家数据

	volatile BOOL				b_conn_lost_;			

	INT							n_verify_code_;			//! 验证码
	BOOL						b_need_mibao_;			//! 是否需要密保验证

	BOOL						b_proofing_;			//! 是否正在进行验证
	BOOL						b_proof_end_;			//! 是否已经验证完毕
	BOOL						b_select_world_ok_;		//! 是否已经选择游戏世界成功

	tagDWORDTime				dw_kick_time_;			//! 将要被踢掉的倒计时

	tagDWORDTime				dw_pre_login_time;		//!上次登录时间
	DWORD						dw_pre_login_ip;			//!上次登录ip

	TCHAR						sz_account_name_t[X_SHORT_NAME]; //! 自适应类型版的用户名
};


#endif


