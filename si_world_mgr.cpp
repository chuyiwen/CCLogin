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

#include "../common/ServerDefine/login_world.h"
#include "../common/ServerDefine/login_server_define.h"
#include "si_world_mgr.h"
#include "si_world.h"
#include "si_login_server.h"
#include "database.h"
#include "anti_addiction_mgr.h"

world_mgr g_fpWorldMgr;
//------------------------------------------------------------------------------
world_mgr::world_mgr()
{

}

//------------------------------------------------------------------------------
world_mgr::~world_mgr()
{
    //destroy();
}


//-------------------------------------------------------------------------------
BOOL world_mgr::init()
{
	INT n_world_num = login_server::p_var_config->get_dword(_T("num zone_server"));

    for(INT n = 0; n < n_world_num; ++n)
    {
		
        si_world* p_world = new si_world;

		if( FALSE == p_world->init(n) )
		{
			SAFE_DELETE(p_world);
			return FALSE;
		}

        map_world.add(p_world->get_id(), p_world);
		
		
		BOOL b_ret = g_datebase.insert_world_state(p_world->get_world_id());
		if(!b_ret)
		{
			g_datebase.update_world_state(p_world->get_world_id(),0,-1);
		}

	
		g_datebase.insert_world_state_log(p_world->get_world_id(),0,-1);
    }

    if( map_world.empty() )
        return FALSE;

   
    dw_world_golden_code = login_server::p_var_config->get_dword(_T("zone_server golden_code"));
	dw_login_server_golden_code = login_server::p_var_config->get_dword(_T("login_server golden_code"));

	p_session = new few_connect_server;
	if( !VALID_POINT(p_session) ) return FALSE;

	register_world_msg();

	INT n_port = login_server::p_var_config->get_int(_T("port"), _T("zone_session"));
	p_session->init(fastdelegate::MakeDelegate(this, &world_mgr::login_call_back),
		fastdelegate::MakeDelegate(this, &world_mgr::logout_call_back), n_port);

	return TRUE;
}

//-------------------------------------------------------------------------------
VOID world_mgr::destroy()
{
  
    si_world* p_world = NULL;


	
	g_datebase.clear_world_state_table();

    map_world.reset_iterator();
    while( map_world.find_next(p_world) )
    {
        SAFE_DELETE(p_world);
    }
	map_world.clear();

	
	p_session->destory();
	SAFE_DELETE(p_session);
	
	
	unregister_world_msg();
	
}


//-------------------------------------------------------------------------------
VOID world_mgr::update()
{
	
	si_world* p_world = NULL;

	map_world.reset_iterator();
	while( map_world.find_next(p_world) )
	{
		p_world->update();
		
		update_world_state(p_world);
		
	}
}

//-------------------------------------------------------------------------------
VOID world_mgr::update_world_state(si_world* p_world)
{
	
	DWORD dw_cur_time = timeGetTime();
	if( (dw_cur_time - p_world->get_db_update_time()) > g_datebase.get_world_state_update_time() )
	{
		p_world->set_db_update_time(dw_cur_time);

		if(p_world->get_status() == ews_well && p_world->is_valid())
		{
			SHORT  s_state = ((DOUBLE)p_world->get_curr_online_num()/p_world->get_max_online_num())*100; //百分比

			//! 刷新世界
			g_datebase.update_world_state(p_world->get_world_id(),p_world->get_curr_online_num(),s_state);
		}
		else
		{
			
			g_datebase.update_world_state(p_world->get_world_id(),0,-1);
		}

	}
	if( (dw_cur_time - p_world->get_db_insert_time()) > g_datebase.get_world_state_insert_time() )
	{
		p_world->set_db_insert_time(dw_cur_time);

		if(p_world->get_status() == ews_well && p_world->is_valid()) 
		{
			SHORT  s_state = ((DOUBLE)p_world->get_curr_online_num()/p_world->get_max_online_num())*100; //百分比

			
			g_datebase.insert_world_state_log(p_world->get_world_id(),p_world->get_curr_online_num(),s_state);
		}
		else
		{
		
			g_datebase.insert_world_state_log(p_world->get_world_id(),0,-1);
		}

	}
}

//-------------------------------------------------------------------------------
VOID world_mgr::register_world_msg()
{
	REGISTER_NET_MSG("NET_W2L_certification",	world_mgr,	handle_certification,		_T("NET_W2L_certification") );
	REGISTER_NET_MSG("NET_W2L_world_status",		world_mgr,handle_zone_server_status,	_T("NET_W2L_world_status") );
	REGISTER_NET_MSG("NET_W2L_player_will_login",	world_mgr,handle_player_will_login,	_T("NET_W2L_player_will_login") );
	REGISTER_NET_MSG("NET_W2L_player_login",		world_mgr,handle_player_login,		_T("NET_W2L_player_login") );
	REGISTER_NET_MSG("NET_W2L_player_logout",	world_mgr,	handle_player_logout,		_T("NET_W2L_player_logout") );
	REGISTER_NET_MSG("NET_W2L_kick_log",		world_mgr,	handle_kick_log,			_T("NET_W2L_kick_log") );
	REGISTER_NET_MSG("NET_2L_world_colsed",	world_mgr,	handle_world_closed,		_T("NET_2L_world_colsed") );
}


//------------------------------------------------------------------------
VOID world_mgr::unregister_world_msg()
{
	UNREGISTER_NET_MSG("NET_W2L_certification",	world_mgr,	handle_certification);
	UNREGISTER_NET_MSG("NET_W2L_world_status",		world_mgr,handle_zone_server_status);
	UNREGISTER_NET_MSG("NET_W2L_player_will_login",	world_mgr,handle_player_will_login);
	UNREGISTER_NET_MSG("NET_W2L_player_login",		world_mgr,handle_player_login);
	UNREGISTER_NET_MSG("NET_W2L_player_logout",	world_mgr,	handle_player_logout);
	UNREGISTER_NET_MSG("NET_W2L_kick_log",		world_mgr,	handle_kick_log);
	UNREGISTER_NET_MSG("NET_2L_world_colsed",	world_mgr,	handle_world_closed);
}


//------------------------------------------------------------------------
DWORD world_mgr::login_call_back(LPBYTE p_byte, DWORD dw_size)
{
	NET_W2L_certification* p_receive = (NET_W2L_certification*)p_byte;

	
	if( p_receive->dw_golden_code != dw_world_golden_code )
		return INVALID_VALUE;

	
	g_login.lock_update();

	
	DWORD dw_name_crc = get_tool()->crc32(p_receive->sz_world_name);

	
	si_world* p_world = get_world(dw_name_crc);
	if( !VALID_POINT(p_world) )
	{
		print_message(_T("one invalid siGameServer login, name=%s\r\n"), p_receive->sz_world_name);

		
		g_login.unlock_update();

		return INVALID_VALUE;
	}

	
	if( !p_world->world_login(p_receive->dw_ip, p_receive->dw_port, p_receive->dw_online_account_id, p_receive->n_online_account_num) )
	{
		print_message(_T("one siGameServer login which is already online, name=%s\r\n"), p_receive->sz_world_name);

		
		g_login.unlock_update();

		return INVALID_VALUE;
	}

	
	print_message(_T("Hello siGameServer——%s\r\n"),p_receive->sz_world_name);

	
	g_login.unlock_update();

	
	return dw_name_crc;
}


//----------------------------------------------------------------------------
DWORD world_mgr::logout_call_back(DWORD dw_param)
{
	DWORD dw_world_id = dw_param;

	
	g_login.lock_update();

	si_world* p_world = get_world(dw_world_id);
	if( !VALID_POINT(p_world) )
	{
		
		g_login.unlock_update();

		return 0;
	}

	
	p_world->world_logout();
	print_message(_T("Bye siGameServer——%s\r\n"), p_world->get_name());

	
	g_login.unlock_update();

	return 0;
}


//------------------------------------------------------------------------------
VOID world_mgr::add_to_world(user* p_user, DWORD dw_world_name_crc)
{
	if( !VALID_POINT(p_user) ) return;

	si_world* p_world = get_world(dw_world_name_crc);
	if( !VALID_POINT(p_world) ) return;

	p_world->add_player(p_user);

	
	g_datebase.login(p_user->get_account_id(), dw_world_name_crc);
}


//------------------------------------------------------------------------------
DWORD world_mgr::handle_certification(tag_net_message* p_msg, si_world* p_world)
{
	NET_W2L_certification* p_receive = (NET_W2L_certification*)p_msg;

	
	NET_L2W_certification send;
	send.dw_golden_code = dw_login_server_golden_code;

	send_msg(p_world->get_id(), &send, send.dw_size);

	return 0;
}


//-------------------------------------------------------------------------------
DWORD world_mgr::handle_zone_server_status(tag_net_message* p_msg, si_world* p_world)
{
	NET_W2L_world_status* p_receive = (NET_W2L_world_status*)p_msg;

	p_world->update_status(p_receive->e_status, p_receive->n_cur_player_num, p_receive->n_player_num_limit);

	return 0;
}


//--------------------------------------------------------------------------------
DWORD world_mgr::handle_player_login(tag_net_message* p_msg, si_world* p_world)
{
	NET_W2L_player_login* p_receive = (NET_W2L_player_login*)p_msg;

	
	if( E_Success != p_receive->dw_error_code )
	{
		SI_LOG->write_log(_T("Player Login Failure! AccountID:%u ErrorCode=%d\r\n"), p_receive->dw_account_id, p_receive->dw_error_code);

		
		g_datebase.logout(p_receive->dw_account_id);
		g_anti_mgr.logout_notify(p_receive->dw_account_id);
	}
	
	else
	{
		
		g_datebase.into_world(p_receive->dw_account_id, p_world->get_id());

		const s_account_data* p_account_data = g_PlayMgr.get_cached_account_data(p_receive->dw_account_id);
		if (!VALID_POINT(p_account_data))
		{
			ASSERT(0);
			return INVALID_VALUE;
		}
		g_anti_mgr.login_notify(p_receive->dw_account_id, p_world->get_id(), p_account_data->b_guard);
		
		g_datebase.update_account_login_info(p_receive->dw_account_id, p_receive->dw_ip);

		
		g_datebase.log_action(p_receive->dw_account_id, p_account_data->sz_account_name, p_receive->dw_ip, "login");
		g_PlayMgr.cache_ip_addres(p_receive->dw_account_id, p_receive->dw_ip);
	}

	return 0;
}


//-----------------------------------------------------------------------------------
DWORD world_mgr::handle_player_logout(tag_net_message* p_msg, si_world* p_world)
{
	NET_W2L_player_logout* p_receive = (NET_W2L_player_logout*)p_msg;

	
	g_datebase.exit_world(p_receive->dw_account_id, p_world->get_id());

	const s_account_data* p_account_data = g_PlayMgr.get_cached_account_data(p_receive->dw_account_id);
	if (!VALID_POINT(p_account_data))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	g_datebase.log_action(p_receive->dw_account_id, p_account_data->sz_account_name, p_account_data->dw_ip, "logout");
	g_anti_mgr.logout_notify(p_receive->dw_account_id);

	return 0;
}


//-------------------------------------------------------------------------------------
DWORD world_mgr::handle_player_will_login(tag_net_message* p_msg, si_world* p_world)
{
	NET_W2L_player_will_login* p_receive = (NET_W2L_player_will_login*)p_msg;

	p_world->player_will_login_ret(p_receive->dw_account_id, p_receive->dw_error_code);

	return 0;
}

//-----------------------------------------------------------------------
DWORD world_mgr::handle_world_closed(tag_net_message* p_msg, si_world* p_world)
{
	//! 改变对应玩家的登录状态
	g_datebase.fixed_world_login_status(p_world->get_id(), epls_null, epls_off_line);

	NET_L2W_world_colsed send;
	send_msg(p_world->get_id(), &send, send.dw_size);

	return 0;
}

//-----------------------------------------------------------------------
DWORD world_mgr::handle_kick_log(tag_net_message* p_msg, si_world* p_world)
{
	NET_W2L_kick_log* p_receive = (NET_W2L_kick_log*)p_msg;

	const s_account_data* p_account_data = g_PlayMgr.get_cached_account_data(p_receive->dw_account_id);
	if (!VALID_POINT(p_account_data))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	g_datebase.insert_kick_log(p_receive->szRoleName, p_receive->szWorldName, p_account_data->sz_account_name, p_receive->dw_account_id, p_receive->dw_kick_time, p_receive->u_16err_code, p_receive->by_seal, p_receive->dw_role_level);
	//if (p_receive->by_seal && p_world->is_auto_seal())
	//{
	//	g_datebase.forbid_account(p_receive->dw_account_id, eplm_waigua);
	//}

	return 0;
}



