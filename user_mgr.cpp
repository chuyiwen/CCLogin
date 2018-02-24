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

#include "user_mgr.h"
#include "user.h"
#include "si_world_mgr.h"
#include "si_world.h"
#include "database.h"
#include "verify_policy_mgr.h"
#include "verify_policy.h"
#include "si_login_server.h"

user_mgr g_PlayMgr;

//-------------------------------------------------------------------------------
user_mgr::user_mgr() : n_player_num(0), n_player_loging(0), n_proof_result_num(0),
						dw_client_id_gen(0), dw_curr_ver(0), n_port(0), e_policy_(epp_null),p_policy_(NULL),b_use_gm_access_(TRUE)
{
}


//-------------------------------------------------------------------------------
user_mgr::~user_mgr()
{
	//Destroy();
}


//-------------------------------------------------------------------------------
BOOL user_mgr::init()
{
	
	if(!init_config())
	{
		return FALSE;
	}

	g_datebase.load_cache_account_data(map_account_data, map_account_name_crc_to_account_id);

	
	e_policy_ = (enum E_proof_policy)login_server::p_var_config->get_int(_T("policy"), _T("proof"));
	if( e_policy_ < epp_test || e_policy_ >= epp_end )
	{
		return FALSE;
	}

	
	p_policy_ = g_verify_policy_mgr.create_proof_policy(e_policy_);
	if( !VALID_POINT(p_policy_) )
	{
		return FALSE;
	}
	if( !p_policy_->init(fastdelegate::MakeDelegate(this, &user_mgr::proof_call_back)) )
	{
		return FALSE;
	}

	facebook_policy_ = g_verify_policy_mgr.create_proof_policy(epp_facebook);
	if( !VALID_POINT(facebook_policy_) )
	{
		return FALSE;
	}
	if( !facebook_policy_->init(fastdelegate::MakeDelegate(this, &user_mgr::facebook_proof_call_back)) )
	{
		return FALSE;
	}

	
	LPCTSTR sz_ver_num = login_server::p_var_config->get_string(_T("version version"));
	dw_curr_ver = get_tool()->crc32(sz_ver_num);

	
	n_port = login_server::p_var_config->get_int(_T("port player_session"));

	

	p_net_session = new IOCP;
	tstring log_name = create_network_log();
	p_net_session->get_log()->create_log(log_name.c_str());

	tag_server_config init_param;
	init_param.fn_login				=	fastdelegate::MakeDelegate(this, &user_mgr::login_call_back);
	init_param.fn_logout			=	fastdelegate::MakeDelegate(this, &user_mgr::logout_call_back);
	init_param.b_repeat_port		=	true;
	init_param.n_port				=	n_port;
	init_param.n_max_client_num	=	20000;
	init_param.n_accept_num		=	128;

	p_net_session->init(init_param);
	n_port = p_net_session->get_config()->n_port;

	
	user::register_player_msg();

	return TRUE;
}

//-------------------------------------------------------------------------
BOOL user_mgr::init_config()
{
	if( !VALID_POINT(login_server::p_var_gm) ) return FALSE;

	std::set <tstring> set_account_name;
	
	//! 是否使用gm地址限制
	b_use_gm_access_ = login_server::p_var_gm->get_int(_T("on switch"));

	
	INT n_account_num = login_server::p_var_gm->get_int(_T("num account"));

	TCHAR sz_temp[X_SHORT_NAME] = {_T('\0')};
	for(int i = 1; i<=n_account_num; ++i)
	{
		_stprintf(sz_temp,_T("name%d"),i);
		set_account_name.insert(login_server::p_var_gm->get_string(sz_temp,_T("account")));
	}
	CHAR sz_ip[X_IP_LEN]= {_T('\0')};
	CHAR sz_name[X_SHORT_NAME]= {_T('\0')};
	std::set <tstring>::iterator it = set_account_name.begin();
	for(;it != set_account_name.end();++it)
	{
		INT iIPNum = login_server::p_var_gm->get_int(_T("num"), (*it).c_str());
		for(int iLoop = 1; iLoop<=iIPNum; ++iLoop)
		{
			_stprintf(sz_temp,_T("ip%d"),iLoop);

			
			tstring strIP = login_server::p_var_gm->get_string(sz_temp,(*it).c_str());
			WideCharToMultiByte(CP_OEMCP,NULL,strIP.c_str(),-1,sz_ip,X_IP_LEN,NULL,FALSE);

			
			DWORD dw_ip = inet_addr(sz_ip);
			map_gm_vs_ip_.insert( make_pair((*it).c_str(),dw_ip) );

		}		
	}
	return TRUE;
}

//-------------------------------------------------------------------------------
VOID user_mgr::destroy()
{
	
	user::unregister_player_msg();

	
	p_net_session->destroy();

    user* p_user = NULL;
    map_logining_user.reset_iterator();
    while( map_logining_user.find_next(p_user) )
    {
        SAFE_DELETE(p_user);
    }

	
	p_policy_->destroy();

	g_verify_policy_mgr.destroy_proof_policy(e_policy_, p_policy_);

	clean_cached_account_datas();

}


//-------------------------------------------------------------------------------
VOID user_mgr::update()
{
	
    update_session();

	update_proof_result();

	Interlocked_Exchange((LPLONG)&n_player_num,		map_all_user.size());
	Interlocked_Exchange((LPLONG)&n_player_loging,	map_logining_user.size());
	Interlocked_Exchange((LPLONG)&n_proof_result_num,	list_proof_result.size());
}


//-------------------------------------------------------------------------------
VOID user_mgr::update_session()
{
	user* p_user = NULL;
	map_logining_user.reset_iterator();
	while( map_logining_user.find_next(p_user) )
	{
		INT n_ret = p_user->handle_message();

		if( CON_LOST == n_ret )	
		{
			map_logining_user.erase(p_user->get_client_id());
			player_logout(p_user);
		}
	}
}


//-------------------------------------------------------------------------------
VOID user_mgr::update_proof_result()
{
	//! 每个Tick最多处理20个人
	INT n_num = 20;

	//! 取出验证结果
	s_proof_result_full* p_result = list_proof_result.pop_front();

	while( VALID_POINT(p_result) )
	{
		
		user* p_user = map_logining_user.find(p_result->dw_client_id);
		if( !VALID_POINT(p_user) )
		{
			SAFE_DELETE(p_result);
			p_result = list_proof_result.pop_front();
			continue;
		}

		if (p_result->by_Type == 1)
		{
			p_user->SetAccountName(p_result->szAccount);
		}

		if( E_Success != p_result->n_ret )
		{
			//! 帐号已经登陆
			if( E_ProofResult_Account_In_Use == p_result->n_ret )
			{
				//! 如果数据库是在线状态，则发往world服务器踢人
				if( epls_online == p_result->e_login_status )
				{
					si_world* p_world = g_fpWorldMgr.get_world(p_result->dw_world_name_crc);
					if( VALID_POINT(p_world) )
					{
						NET_L2W_kick_player send;
						send.dw_account_id = p_result->dw_account_id;
						p_world->send_kick_player_msg(send);
					}
					else
					{
						g_datebase.fixed_login_status(p_result->dw_account_id, epls_off_line);
						p_result->n_ret = E_Success;	// 注意这个地方
					}
				}
				//! 如果这时候帐号处在登陆中
				else if( epls_loging == p_result->e_login_status )
				{
					//! 在本地查找是否该账号在login里面，如果有，不管了
					if( is_account_exist(p_result->dw_account_id) )
					{

					}
					//! 如果没有，则可能是因为有些情况导致不同步，那么就直接清空数据库，并设置成功
					else
					{
						g_datebase.fixed_login_status(p_result->dw_account_id, epls_off_line);
						p_result->n_ret = E_Success;	// 注意这个地方
					}
				}
			}
			//! 如果是GM工具封停 往world_forbid表里搜索一下
			else if( p_result->n_ret == E_ProofResult_Forbid_GMTool )
			{
				BOOL bForbid = g_datebase.if_world_forbid(p_result->dw_account_id,p_result->dw_world_name_crc);

				//! 这表示该账号可能是别的游戏世界被封停了
				if( bForbid == FALSE )
				{
					p_result->n_ret = E_Success;	// 注意这个地方
				}
			}
			// 其它情况
			else
			{

			}
		}
		//! 验证成功
		else
		{
			//! 如果是权限为gm到map_gm_vs_ip_表里找
			if( b_use_gm_access_ && (p_result->by_privilege != 0) )
			{
				BOOL b_find = FALSE;
				typedef std::multimap<tstring, DWORD>::iterator itFinder;

				itFinder beg = map_gm_vs_ip_.lower_bound(p_user->get_accout_name_t()),
					end = map_gm_vs_ip_.upper_bound(p_user->get_accout_name_t());
				while( beg != end )
				{
					if( beg->second == p_user->get_player_data().dw_ip )
					{
						b_find = TRUE;
						break;
					}
					++beg;
				}
				if( b_find == FALSE )
				{
					//! 目前对应成这个
					p_result->n_ret = E_ProofResult_Account_No_Match;	// 注意这个地方

					few_connect_client p_trans;
					char sz_ip[X_IP_LEN];
					strcpy(sz_ip,p_trans.ip_to_string(p_user->get_player_data().dw_ip));

					TCHAR tszIP[X_IP_LEN];
#ifdef UNICODE
					MultiByteToWideChar(CP_ACP,NULL,sz_ip,-1,tszIP,X_SHORT_NAME);
#else
					strcpy(tszIP,sz_ip);
#endif
					SI_LOG->write_log(_T("GM:%s非法登录服务器!!	IP:%s\r\n"),p_user->get_accout_name_t(),tszIP);
				}	
			}
		}

		//! 如果对应客户端的ip被封了
		if( TRUE == g_datebase.if_ip_forbid(p_user->get_player_data().dw_ip) )
		{
			
			p_result->n_ret = E_ProofResult_Account_No_Match;	// 注意这个地方
		}

		//! 如果验证成功，此时再验证一下游戏世界
		if( E_Success == p_result->n_ret )
		{
			//! 找到玩家所要请求进入的游戏世界
			si_world* pWorld = g_fpWorldMgr.get_world(p_user->get_login_world_name_crc());
			if( !VALID_POINT(pWorld) || ews_well != pWorld->get_status() )
			{
				p_result->n_ret = E_SelectWorld_Server_Maintenance;
			}
		}


		
		if( E_Success == p_result->n_ret )
		{
			//! 设置玩家验证完毕
			p_user->proof_return(p_result);

			//! 设置玩家上次登录ip和时间
			p_user->set_pre_login_ip(p_result->dw_pre_login_ip);
			p_user->set_pre_login_time(p_result->dw_pre_login_time);

			//! 如果该玩家还需要密保
			if( p_user->is_need_mibao() )
			{
				NET_L2C_mibao send;
				p_user->generate_mibao(send.szMiBao);	// 生成密保
				p_user->send_message(&send, send.dw_size);
			}
			//! 不需要密保，则加入到游戏世界列表中等待
			else
			{
				remove_player_from_logining(p_user->get_client_id());

				//! 加入帐号
				add_account(p_user->get_account_id(), p_user->get_client_id());

				//! 将玩家加入到世界的玩家列表中
				g_fpWorldMgr.add_to_world(p_user, p_user->get_login_world_name_crc());
			}
		}
		//! 如果最终没有验证成功
		else
		{
			NET_L2C_proof_result send;
			send.dw_error_code = p_result->n_ret;

			p_user->send_message(&send, send.dw_size);
		}

		SAFE_DELETE(p_result);

		//! 减一个人，如果减到0就退出
		if( --n_num <= 0 ) break;

		p_result = list_proof_result.pop_front();
	}
}


//-------------------------------------------------------------------------------
UINT user_mgr::login_call_back(tag_unit* pUnit, tag_login_param* pParam)
{
    
	const static DWORD c2lProofcrc = get_tool()->crc32("NET_C2L_proof");
	const static DWORD c2lProofthirdcrc = get_tool()->crc32("NET_C2L_proof_third");


	tag_net_message* pCmd = (tag_net_message*)pUnit->p_buffer;

	DWORD dw_world_crc;
	//! 检查消息的Crc是否相同

	if(pCmd->dw_message_id == c2lProofcrc){
		dw_world_crc = static_cast<NET_C2L_proof*>(pCmd)->dw_world_name_crc;
	}else if(pCmd->dw_message_id == c2lProofthirdcrc){
		dw_world_crc = static_cast<NET_C2L_proof_third*>(pCmd)->dw_world_name_crc;
	}else{
		return INVALID_VALUE; 
	}



	g_login.lock_update();

	//! 生成一个新的客户端ID
	DWORD dw_client_id = generate_client_id();
    
	//! 生成玩家
	user* p_user = new user(dw_client_id, pParam->dw_handle, pParam->dw_address, dw_world_crc);
	if( !VALID_POINT(p_user) )
	{
		g_login.unlock_update();
		return INVALID_VALUE;
	}

	
	add_player_to_all(p_user);
	
	add_player_to_logining(p_user);

	g_login.unlock_update();

    return dw_client_id;
}


//-------------------------------------------------------------------------------
UINT user_mgr::logout_call_back(DWORD dw_client_id)
{
	
	g_login.lock_update();

    //! 查找玩家
    user* p_user = map_all_user.find(dw_client_id);
    if( VALID_POINT(p_user) )
    {
        //! 设置此玩家断开连接
		p_user->set_conn_lost();
    }

	
	g_login.unlock_update();

    return 0;
}


//---------------------------------------------------------------------------------
VOID user_mgr::player_logout(user* p_user)
{
	//! 是否设置数据库中该玩家为离线
	BOOL bLogoutFromDB = TRUE;		


	//! 还没有验证成功，数据库根本就没有操作过
	if( !p_user->is_proof_end() || INVALID_VALUE == p_user->get_account_id() )
	{
		bLogoutFromDB = FALSE;
	}
	else if( p_user->is_select_world_ok() )	
	{
		bLogoutFromDB = FALSE;
	}


	if( bLogoutFromDB )
	{
		g_datebase.logout(p_user->get_account_id());
	}

	
	if( INVALID_VALUE != p_user->get_account_id() )
	{
		remove_account(p_user->get_account_id());
	}

	
	remove_player_rrom_all(p_user->get_client_id());

	
	SAFE_DELETE(p_user);
}


//------------------------------------------------------------------------------------
VOID user_mgr::proof(DWORD dw_client_id, LPCSTR sz_accout_name, LPCSTR sz_password, LPCSTR sz_guid)
{
	if( !VALID_POINT(sz_accout_name) || !VALID_POINT(sz_password) || !VALID_POINT(sz_guid) )
		return;

	//! 发送给相应的验证策略进行验证
	p_policy_->proof(dw_client_id, sz_accout_name, sz_password, sz_guid, 0);
}

VOID user_mgr::proof_facebook(DWORD dw_client_id, LPCSTR szGUID, LPCSTR szCode, INT nType)
{
	if( !VALID_POINT(szGUID) || !VALID_POINT(szCode))
		return;

	//! 发送给相应的验证策略进行验证
	facebook_policy_->proof(dw_client_id, szGUID, szCode, "", nType);
}


//--------------------------------------------------------------------------------------
UINT user_mgr::proof_call_back(INT n_ret, VOID* p)
{
	s_proof_result* p_result = (s_proof_result*)p;
	s_proof_result_full* pResultFull = new s_proof_result_full;
	memcpy(pResultFull, p_result, sizeof(s_proof_result));
	pResultFull->n_ret = n_ret;
	pResultFull->by_Type = 0;
	list_proof_result.push_back(pResultFull);

	return 0;
}

UINT user_mgr::facebook_proof_call_back(INT n_ret, VOID* p)
{
	s_proof_result_third* p_result = (s_proof_result_third*)p;
	s_proof_result_full* pResultFull = new s_proof_result_full;
	memcpy(pResultFull, &p_result->result, sizeof(s_proof_result));
	pResultFull->n_ret = n_ret;
	pResultFull->by_Type = 1;

	strncpy(pResultFull->szAccount,p_result->szAccount, X_SHORT_NAME);

	//if(n_ret == E_Success){
	//	user* p_user = map_logining_user.find(p_result->result.dw_client_id);
	//	if( !VALID_POINT(p_user) )
	//	{
	//		p_user->SetAccountName(p_result->szAccount);
	//	}
	//}

	list_proof_result.push_back(pResultFull);

	return 0;
}

VOID user_mgr::cache_account_name( DWORD dw_account_id, const CHAR* sz_account_name)
{
	s_account_data* p_account_data = map_account_data.find(dw_account_id);		
	if (!VALID_POINT(p_account_data))
	{
		p_account_data = new s_account_data;
		map_account_data.add(dw_account_id, p_account_data);
	}
	memcpy(p_account_data->sz_account_name, sz_account_name, X_SHORT_NAME);
}

VOID user_mgr::cache_ip_addres( DWORD dw_account_id, DWORD dw_ip)
{
	s_account_data* p_account_data = map_account_data.find(dw_account_id);		
	if (!VALID_POINT(p_account_data))
	{
		p_account_data = new s_account_data;
		map_account_data.add(dw_account_id, p_account_data);
	}
	p_account_data->dw_ip = dw_ip;
}

VOID user_mgr::cache_guard( DWORD dw_account_id, BOOL b_guard)
{
	s_account_data* p_account_data = map_account_data.find(dw_account_id);		
	if (!VALID_POINT(p_account_data))
	{
		p_account_data = new s_account_data;
		map_account_data.add(dw_account_id, p_account_data);
	}
	p_account_data->b_guard = b_guard;
}

VOID user_mgr::erase_cached_account_data( DWORD dw_account_id )
{
	s_account_data* p_account_data = map_account_data.find(dw_account_id);
	if (VALID_POINT(p_account_data))
	{
		SAFE_DELETE(p_account_data);
		map_account_data.erase(dw_account_id);
	}
}

VOID user_mgr::clean_cached_account_datas()
{
	std::list<DWORD> listIDs;
	map_account_data.copy_key_to_list(listIDs);
	std::list<DWORD>::iterator itr = listIDs.begin();
	while(itr != listIDs.end())
	{
		erase_cached_account_data(*itr);
		++itr;
	}
}

VOID user_mgr::map_account_name_to_account_id( LPCSTR sz_account_name, DWORD dw_account_id )
{
	DWORD dwNameCrc = get_tool()->crc32(sz_account_name);
	if (!VALID_VALUE(map_account_name_crc_to_account_id.find(dwNameCrc)))
	{
		map_account_name_crc_to_account_id.add(dwNameCrc, dw_account_id);
	}
}

DWORD user_mgr::get_account_id_by_account_name( LPCSTR sz_account_name )
{
	DWORD dwNameCrc = get_tool()->crc32(sz_account_name);
	return map_account_name_crc_to_account_id.find(dwNameCrc);
}


//----------------------------------------------------------------------------
LPBYTE user_mgr::recv_msg(DWORD& dw_size, INT& n_msg_num, DWORD dw_cd_index)
{
	return p_net_session->recv(dw_cd_index, dw_size, n_msg_num);
}


//----------------------------------------------------------------------------
VOID user_mgr::return_msg(LPBYTE p_msg)
{
	p_net_session->free_recv(p_msg);
}


//----------------------------------------------------------------------------
VOID user_mgr::send_msg(DWORD dw_cd_index, LPBYTE p_msg, DWORD dw_size)
{
	p_net_session->send(dw_cd_index, p_msg, dw_size);
}


//-----------------------------------------------------------------------------
VOID user_mgr::add_account(DWORD dw_account_id, DWORD dw_client_id)
{
	if( !VALID_POINT(dw_account_id) || !VALID_POINT(dw_client_id) )
		return;

	map_account_id_client_id.add(dw_account_id, dw_client_id);
}


//------------------------------------------------------------------------------
VOID user_mgr::remove_account(DWORD dw_account_id)
{
	if( !VALID_POINT(dw_account_id) ) return;

	map_account_id_client_id.erase(dw_account_id);
}

tstring	user_mgr::create_network_log()
{
	tstring str_file_name = _T("log\\");

	TCHAR sz_time[MAX_PATH], sz_handle_name[MAX_PATH];
	wsprintf(sz_handle_name, _T("LoginNetWork"));

	TCHAR *p_result = _tcsrchr(sz_handle_name, _T('\\'));
	p_result = p_result ?	p_result+1 :	p_result = (TCHAR *)sz_handle_name;

	TCHAR* p_result1 = _tcsrchr(p_result, _T('.'));
	if( p_result1 )
		*p_result1 = _T('\0');

	str_file_name += p_result;
	str_file_name += _T("_");

	FILETIME st_current_time;
	GetSystemTimeAsFileTime(&st_current_time);
	sz_time[0] = _T('\0');
	WORD w_data, w_time;
	if (FileTimeToLocalFileTime(&st_current_time, &st_current_time) &&
		FileTimeToDosDateTime(&st_current_time, &w_data, &w_time))
	{
		wsprintf(sz_time, _T("[%d-%d-%d %02d-%02d-%02d %05d___%d].log"),		
			(w_data / 32) & 15, w_data & 31, (w_data / 512) + 1980,
			(w_time >> 11), (w_time >> 5) & 0x3F, (w_time & 0x1F) * 2, 
			GetCurrentProcessId(),
			rand());
	}

	str_file_name += sz_time;

	return str_file_name;
}




