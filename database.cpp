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

#include "../../common/WorldDefine/GMDefine.h"
#include "database.h"
#include "user.h"
#include "si_login_server.h"
#include "../common/ServerDefine/anti_addiction_server_define.h"
#include "../../common/WorldDefine/time.h"

database_operator g_datebase;

BOOL database_operator::init()
{
	BOOL b_ret = database_.init_db(login_server::p_var_config->get_string(_T("ip database")), 
							login_server::p_var_config->get_string(_T("user database")),
							login_server::p_var_config->get_string(_T("psd database")), 
							login_server::p_var_config->get_string(_T("name database")),
							(INT)login_server::p_var_config->get_dword(_T("port database")));

	dw_world_state_update_time_ = login_server::p_var_config->get_dword(_T("update_time world_state"));

	dw_world_state_insert_time_ = login_server::p_var_config->get_dword(_T("insert_time world_state"));

	database_.set_alarm_call_back((alarm_call_back)call_back);

	return b_ret;
}


VOID database_operator::destroy()
{
}

VOID database_operator::login(DWORD dw_account_id, DWORD dw_world_name_crc)
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return;

	p_stream->update_item("account");
	p_stream->write_string("login_status=") << epls_loging;
	p_stream->write_string(",world_name_crc=") << dw_world_name_crc;
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	database_.sql_execute(p_stream);

	database_.return_io(p_stream);
}


VOID database_operator::logout(DWORD dw_account_id)
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return;

	p_stream->update_item("account");
	p_stream->write_string("login_status=") << epls_off_line;
	p_stream->write_string(",world_name_crc=") << (DWORD)INVALID_VALUE;
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;
	p_stream->write_string(" and login_status=") << epls_loging;

	database_.sql_execute(p_stream);

	database_.return_io(p_stream);
}


VOID database_operator::into_world(DWORD dw_account_id, DWORD dw_world_name_crc)
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return;

	p_stream->update_item("account");
	p_stream->write_string("login_status=") << epls_online;
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;
	p_stream->write_string(" and world_name_crc=") << dw_world_name_crc;

	database_.sql_execute(p_stream);

	database_.return_io(p_stream);
}


VOID database_operator::exit_world(DWORD dw_account_id, DWORD dw_world_name_crc)
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return;

	p_stream->update_item("account");
	p_stream->write_string("login_status=") << epls_off_line;
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;
	p_stream->write_string(" and world_name_crc=") << dw_world_name_crc;

	database_.sql_execute(p_stream);

	database_.return_io(p_stream);
}

VOID database_operator::fixed_login_status( DWORD dw_account_id, E_player_login_status e_dest )
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return;

	p_stream->update_item("account");
	p_stream->write_string("login_status=") << e_dest;
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	database_.sql_execute(p_stream);

	database_.return_io(p_stream);
}


VOID database_operator::reset_all_login_status(E_player_login_status e_src, E_player_login_status e_dest)
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return;

	//! 将登陆中状态全部改变成已登入状态，修正因为登入服务器当机引发的异常
	p_stream->update_item("account");
	p_stream->write_string("login_status=") << e_dest;
	p_stream->where_item();
	p_stream->write_string("login_status=") << e_src;

	database_.sql_execute(p_stream);

	database_.return_io(p_stream);
}
//---------------------------------------------------------------------------------
//! 改变指定游戏世界玩家的登录状态
//---------------------------------------------------------------------------------
VOID database_operator::fixed_world_login_status( DWORD dw_world_name_crc, E_player_login_status e_src, E_player_login_status e_dest )
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return;

	p_stream->update_item("account");
	p_stream->write_string("login_status=") << e_dest;
	p_stream->where_item();
	p_stream->write_string("world_name_crc=")<<dw_world_name_crc;
	if (e_src != epls_null)
	{
		p_stream->write_string(" and login_status=") << e_src;
	}

	database_.sql_execute(p_stream);

	database_.return_io(p_stream);
}
//---------------------------------------------------------------------------------
//! 得到某个玩家的密保
//---------------------------------------------------------------------------------
BOOL database_operator::get_mibao(DWORD dw_account_id, CHAR szMibao[MIBAO_LENGTH])
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	
	p_stream->select_item("account", "mibao");
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	execute_result* p_result = database_.sql_query(p_stream);

	database_.return_io(p_stream);

	
	if( !VALID_POINT(p_result) || p_result->get_row_count() <= 0) return FALSE;

	BOOL b_ret = (*p_result)[0].get_blob(szMibao, MIBAO_LENGTH);
	database_.free_result_query(p_result);
	return b_ret;
}
//-------------------------------------------------------------------------
//! 插入世界状态记录
//-------------------------------------------------------------------------
BOOL database_operator::insert_world_state(DWORD dw_world_id)
{

	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	//! 查询用户名,密码, 登录状态
	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	char sz_date[X_DATATIME_LEN + 1];
	ZeroMemory(sz_date, sizeof(sz_date));
	DwordTime2DataTime(sz_date, X_DATATIME_LEN + 1, GetCurrentDWORDTime());


	p_stream->insert_item("world_state");
	p_stream->write_string("world_id=")<<dw_world_id;
	p_stream->write_string(",role_num=")<<0;
	p_stream->write_string(",world_state=")<<-1;
	p_stream->write_string(",time='").write_string(sz_date, p_con).write_string("'");

	database_.return_use_connect(p_con);

	BOOL b_ret = database_.sql_execute(p_stream);


	database_.return_io(p_stream);

	return b_ret;
}
//----------------------------------------------------------------------------
//! 插入世界状态log
//----------------------------------------------------------------------------
BOOL database_operator::insert_world_state_log(DWORD dw_world_id,INT i_role_num, SHORT s_state)
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;


	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	char sz_date[X_DATATIME_LEN + 1];
	ZeroMemory(sz_date, sizeof(sz_date));
	DwordTime2DataTime(sz_date, X_DATATIME_LEN + 1, GetCurrentDWORDTime());

	p_stream->insert_item("world_state_log");
	p_stream->write_string("world_id=")<<dw_world_id;
	p_stream->write_string(",role_num=")<<i_role_num;
	p_stream->write_string(",world_state=")<<s_state;
	p_stream->write_string(",time='").write_string(sz_date, p_con).write_string("'");


	database_.return_use_connect(p_con);


	BOOL b_ret = database_.sql_execute(p_stream);


	database_.return_io(p_stream);

	return b_ret;
}
//----------------------------------------------------------------------------
//! 清空状态表
//----------------------------------------------------------------------------
BOOL database_operator::clear_world_state_table()
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	p_stream->clear();
	p_stream->write_string("TRUNCATE TABLE world_state");


	BOOL b_ret = database_.sql_execute(p_stream);

	database_.return_io(p_stream);

	return b_ret;

}
//!----------------------------------------------------------------------------
//! 更新状态
//!----------------------------------------------------------------------------
BOOL database_operator::update_world_state(DWORD dw_world_id,INT i_role_num, SHORT s_state)
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	//查询用户名,密码, 登录状态
	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	char sz_date[X_DATATIME_LEN + 1];
	ZeroMemory(sz_date, sizeof(sz_date));
	DwordTime2DataTime(sz_date, X_DATATIME_LEN + 1, GetCurrentDWORDTime());

	p_stream->update_item("world_state");
	p_stream->write_string("role_num=")<<i_role_num;
	p_stream->write_string(",world_state=")<<s_state;
	p_stream->write_string(",time='").write_string(sz_date, p_con).write_string("'");
	p_stream->where_item();
	p_stream->write_string("world_id=")<<dw_world_id;


	database_.return_use_connect(p_con);


	BOOL b_ret = database_.sql_execute(p_stream);

	database_.return_io(p_stream);

	return b_ret;
}

//-------------------------------------------------------------------------------
VOID database_operator::update_account_login_info( DWORD dw_account_id, DWORD &dw_ip)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return;

	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	few_connect_client p_trans;

	char sz_date[X_DATATIME_LEN + 1];
	ZeroMemory(sz_date, sizeof(sz_date));
	DwordTime2DataTime(sz_date, X_DATATIME_LEN + 1, GetCurrentDWORDTime());

	p_stream->update_item("account");
	p_stream->write_string("ip='").write_string(p_trans.ip_to_string(dw_ip), p_con).write_string("',");
	p_stream->write_string("time='").write_string(sz_date, p_con).write_string("'");
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;


	BOOL b_ret = database_.sql_execute( p_stream );

	database_.return_use_connect(p_con);	 
 	database_.return_io(p_stream);

}

//-------------------------------------------------------------------------------
//! 保存Log进数据库
//-----------------------------------------------------------------------------------
VOID database_operator::log_action( DWORD dw_account_id, LPCSTR sz_account_name, DWORD dw_ip, LPCSTR sz_action )
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return;

	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	few_connect_client p_trans;

	char sz_date[X_DATATIME_LEN + 1];
	ZeroMemory(sz_date, sizeof(sz_date));
	DwordTime2DataTime(sz_date, X_DATATIME_LEN + 1, GetCurrentDWORDTime());

	p_stream->insert_item("login_log");
	p_stream->write_string("account_id=") << dw_account_id;
	p_stream->write_string(",account_name='").write_string(sz_account_name, p_con).write_string("'");
	p_stream->write_string(",ip='").write_string(p_trans.ip_to_string(dw_ip), p_con).write_string("'");
	p_stream->write_string(",action='").write_string(sz_action, p_con).write_string("'");
	p_stream->write_string(",time='").write_string(sz_date, p_con).write_string("'");

	database_.return_use_connect(p_con);
	database_.add_asynchronism_query(p_stream);
}


//-------------------------------------------------------------------------------
//! 保存KickLog进数据库
//-------------------------------------------------------------------------------
VOID database_operator::insert_kick_log( const CHAR* p_role_name, const CHAR* p_world_name, const CHAR* p_account_name, DWORD dw_account_id, DWORD dw_time, UINT16 u_16err_code, BOOL b_seal, DWORD dwRoleLevel)
{
	sql_language_disposal* p_stream = database_.get_io();
	
	if (!VALID_POINT(p_stream))
		return;

	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	if (!VALID_POINT(p_con))
	{
		return;
	}

	char sz_date[X_DATATIME_LEN + 1];
	ZeroMemory(sz_date, sizeof(sz_date));
	DwordTime2DataTime(sz_date, X_DATATIME_LEN + 1, dw_time);

	p_stream->insert_item("game_guarder_log");
	p_stream->write_string(" account_id=") << dw_account_id;
	p_stream->write_string(", name='").write_string(p_account_name, p_con).write_string("'");
	p_stream->write_string(" ,kick_time='").write_string(sz_date, p_con).write_string("'");
	p_stream->write_string(" ,seal=") << b_seal;
	p_stream->write_string(" ,error_code=") << u_16err_code;
	p_stream->write_string(", role_name='").write_string(p_role_name, p_con).write_string("'");
	p_stream->write_string(", world_name='").write_string(p_world_name, p_con).write_string("'");
	p_stream->write_string(" ,role_level=") << dwRoleLevel;

	database_.return_use_connect(p_con);
	database_.add_asynchronism_query(p_stream);
}
//-------------------------------------------------------------------------------
//! 查看指定游戏世界是否被封停
//-------------------------------------------------------------------------------
BOOL database_operator::if_world_forbid(DWORD dw_account_id,DWORD dw_world_name_crc)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return FALSE;	
	p_stream->select_item("world_forbid","*");
	p_stream->where_item();
	p_stream->write_string("account_id=") << dw_account_id;
	p_stream->write_string(" AND world_name_crc=") << dw_world_name_crc;

	execute_result* p_result = database_.sql_query( p_stream );
	if(!VALID_POINT(p_result) || p_result->get_row_count() <= 0)
	{
		database_.return_io(p_stream);
		return FALSE;
	}

	database_.return_io(p_stream);
	return TRUE;
}
//-------------------------------------------------------------------------------
//! 查看指定账号是否存在被封停游戏世界中
//-------------------------------------------------------------------------------
BOOL database_operator::if_have_world_forbid(DWORD dw_account_id)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return FALSE;	

	//! 检查指定游戏世界是否被封了
	p_stream->select_item("world_forbid","*");
	p_stream->where_item();
	p_stream->write_string("account_id=") << dw_account_id;

	execute_result* p_result = database_.sql_query( p_stream );

	if(!VALID_POINT(p_result) || p_result->get_row_count() <= 0)
	{
		database_.free_result_query( p_result );
		database_.return_io(p_stream);
		return FALSE;
	}
	database_.free_result_query( p_result );
	database_.return_io(p_stream);
	return TRUE;
}
//-------------------------------------------------------------------------------
//! 查看指定ip是不是被封了
//-------------------------------------------------------------------------------
BOOL database_operator::if_ip_forbid(DWORD dw_ip)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return FALSE;

	few_connect_client p_trans;

	char sz_ip[X_IP_LEN];
	strcpy(sz_ip,p_trans.ip_to_string(dw_ip));


	p_stream->select_item("black_list","*");
	p_stream->where_item();
	p_stream->write_string("ip = '").write_string(sz_ip).write_string("'");

	execute_result* p_result = database_.sql_query(p_stream);

	if(!VALID_POINT(p_result) || p_result->get_row_count() <= 0)
	{
		database_.free_result_query( p_result );
		database_.return_io(p_stream);
		return FALSE;
	}

	database_.free_result_query( p_result );
	database_.return_io(p_stream);
	return TRUE;
}

//-------------------------------------------------------------------------------
//! 解除封停
//-------------------------------------------------------------------------------
DWORD database_operator::remove_account_forbid( LPCTSTR sz_account_name, DWORD dw_forbidM,DWORD dw_world_name_crc)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return EGMDatabase_Busy;

	
	p_stream->select_item("account","id,forbid_mask");
	p_stream->where_item();
	p_stream->write_string("name='").write_string(sz_account_name).write_string("'");

	execute_result* p_result = database_.sql_query( p_stream );
	
	if(!VALID_POINT(p_result) || p_result->get_row_count() <= 0)
	{
		database_.return_io(p_stream);
		return EGMForbidAccount_AccountNotExist;
	}

	DWORD dw_account_id = (*p_result)[0].get_dword();
	//! 要保证原来的标志位为1
	if(  ( (*p_result)[1].get_short() & dw_forbidM ) != dw_forbidM )
	{
		
		if(dw_forbidM & eplm_gm_tool)
		{
			
			if(!if_world_forbid(dw_account_id,dw_world_name_crc))
			{
				database_.free_result_query( p_result );
				database_.return_io(p_stream);
				return EGMForbidAccount_Forbidded;
			}
		}
		else
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return EGMForbidAccount_Forbidded;
		}
	}
	//! 如果是GM工具封号 则更新world_forbid表
	if(dw_forbidM & eplm_gm_tool)
	{
		p_stream->delete_item("world_forbid");
		p_stream->where_item();
		p_stream->write_string("account_id=")<<dw_account_id;
		p_stream->write_string(" AND");
		p_stream->write_string(" world_name_crc=")<<dw_world_name_crc;

		BOOL b_ret = database_.sql_execute(p_stream);
		if(b_ret == false)
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return EGMDatabase_Busy;
		}
		//! 如果是gm工具封停指定游戏世界 只要当该账号所有游戏世界都被解封 才异或标志位

		//! 还存在被封停的游戏世界
		if(if_have_world_forbid(dw_account_id))
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return E_Success;
		}
	}

	p_stream->update_item("account");
	p_stream->write_string("forbid_mask = ") << dw_forbidM;
	p_stream->write_string("^forbid_mask");
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	BOOL b_ret = database_.sql_execute(p_stream);
	if(b_ret == FALSE)
	{
		database_.free_result_query( p_result );
		database_.return_io(p_stream);
		return EGMDatabase_Busy;
	}
	database_.free_result_query( p_result );
	database_.return_io(p_stream);
	return E_Success;
}


DWORD database_operator::remove_account_forbid( DWORD dw_account_id, DWORD dw_forbidM,DWORD dw_world_name_crc)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return EGMDatabase_Busy;

	
	p_stream->select_item("account","forbid_mask");
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	execute_result* p_result = database_.sql_query( p_stream );
	
	if(!VALID_POINT(p_result) || p_result->get_row_count() <= 0)
	{
		database_.return_io(p_stream);
		return EGMForbidAccount_AccountNotExist;
	}
	//! 要保证原来的标志位为1
	if(  ( (*p_result)[0].get_short() & dw_forbidM ) != dw_forbidM )
	{
		//! 如果是gm
		if(dw_forbidM & eplm_gm_tool)
		{
			//! 如果对应游戏世界没有被封停
			if(!if_world_forbid(dw_account_id,dw_world_name_crc))
			{
				database_.free_result_query( p_result );
				database_.return_io(p_stream);
				return EGMForbidAccount_Forbidded;
			}
		}
		else
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return EGMForbidAccount_Forbidded;
		}
	}
	//! 如果是GM工具封号 需要更新world_forbid表
	if(dw_forbidM & eplm_gm_tool)
	{
		p_stream->delete_item("world_forbid");
		p_stream->where_item();
		p_stream->write_string("account_id=")<<dw_account_id;
		p_stream->write_string(" AND");
		p_stream->write_string(" world_name_crc=")<<dw_world_name_crc;

		BOOL b_ret = database_.sql_execute(p_stream);
		if(b_ret == false)
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return EGMDatabase_Busy;
		}
		//! 还存在被封停的游戏世界
		if(if_have_world_forbid(dw_account_id))
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return E_Success;
		}
	}

	p_stream->update_item("account");
	p_stream->write_string("forbid_mask = ") << dw_forbidM;
	p_stream->write_string("^forbid_mask");
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	BOOL b_ret = database_.sql_execute(p_stream);
	if(b_ret == FALSE)
	{
		database_.free_result_query( p_result );
		database_.return_io(p_stream);
		return EGMDatabase_Busy;
	}



	database_.free_result_query( p_result );
	database_.return_io(p_stream);
	return E_Success;
}



DWORD database_operator::set_account_chenmi( DWORD dw_account_id, DWORD dw_chen_mi,DWORD dw_world_name_crc)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return EGMDatabase_Busy;

	p_stream->update_item("account");
	p_stream->write_string("guard=") << (BYTE)dw_chen_mi;
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	BOOL b_rtv = database_.sql_execute(p_stream);
	database_.return_io(p_stream);

	return b_rtv ? E_Success : EGMDatabase_Busy;
}

//-------------------------------------------------------------------------------
//! 封号
//-------------------------------------------------------------------------------
DWORD database_operator::forbid_account( LPCTSTR sz_account_name, DWORD dw_forbidM,DWORD dw_world_name_crc,  DWORD start, DWORD end)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return EGMDatabase_Busy;

	
	p_stream->select_item("account","id,forbid_mask");
	p_stream->where_item();
	p_stream->write_string("name='").write_string(sz_account_name).write_string("'");

	execute_result* p_result = database_.sql_query( p_stream );

	if(!VALID_POINT(p_result) || p_result->get_row_count() <= 0)
	{
		database_.return_io(p_stream);
		return EGMForbidAccount_AccountNotExist;
	}

	DWORD dw_account_id = (*p_result)[0].get_dword();
	SHORT s_forbid_mask = (*p_result)[1].get_short();

	if(  ( s_forbid_mask & dw_forbidM ) != 0 )
	{
	
		if(dw_forbidM & eplm_gm_tool)
		{
		
			if(if_world_forbid(dw_account_id,dw_world_name_crc))
			{
				database_.free_result_query( p_result );
				database_.return_io(p_stream);
				return EGMForbidAccount_Forbidded;
			}
		}
		else
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return EGMForbidAccount_Forbidded;
		}
	}



	if(dw_forbidM & eplm_gm_tool)
	{
		p_stream->insert_item("world_forbid");
		p_stream->write_string("account_id=")<<dw_account_id;
		p_stream->write_string(",world_name_crc=")<<dw_world_name_crc;
		p_stream->write_string(",forbid_start=") << start;
		p_stream->write_string(",forbid_end=") << end;

		BOOL b_ret = database_.sql_execute(p_stream);
		if(b_ret == FALSE)
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return EGMDatabase_Busy;
		}
		
		if(s_forbid_mask & eplm_gm_tool)
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return E_Success;
		}
	}

	p_stream->update_item("account");
	p_stream->write_string("forbid_mask = ") << dw_forbidM;
	p_stream->write_string("^forbid_mask");
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	BOOL b_ret = database_.sql_execute(p_stream);
	if(b_ret == FALSE)
	{
		database_.free_result_query( p_result );
		database_.return_io(p_stream);
		return EGMDatabase_Busy;
	}


	database_.free_result_query( p_result );
	database_.return_io(p_stream);
	return E_Success;
}

DWORD database_operator::forbid_account( DWORD dw_account_id, DWORD dw_forbidM,DWORD dw_world_name_crc)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return EGMDatabase_Busy;

	
	p_stream->select_item("account","forbid_mask");
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	execute_result* p_result = database_.sql_query( p_stream );
	
	if(!VALID_POINT(p_result) || p_result->get_row_count() <= 0)
	{
		database_.return_io(p_stream);
		return EGMForbidAccount_AccountNotExist;
	}
	SHORT s_forbid_mask = (*p_result)[0].get_short();
	
	if(  ( s_forbid_mask & dw_forbidM ) != 0 )
	{
	
		if(dw_forbidM & eplm_gm_tool)
		{
	
			if(if_world_forbid(dw_account_id,dw_world_name_crc))
			{
				database_.free_result_query( p_result );
				database_.return_io(p_stream);
				return EGMForbidAccount_Forbidded;
			}
		}
		else
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return EGMForbidAccount_Forbidded;
		}
	}

	if(dw_forbidM & eplm_gm_tool)
	{
		p_stream->insert_item("world_forbid");
		p_stream->write_string("account_id=")<<dw_account_id;
		p_stream->write_string(",world_name_crc=")<<dw_world_name_crc;

		BOOL b_ret = database_.sql_execute(p_stream);
		if(b_ret == FALSE)
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return EGMDatabase_Busy;
		}

		if(s_forbid_mask & eplm_gm_tool)
		{
			database_.free_result_query( p_result );
			database_.return_io(p_stream);
			return E_Success;
		}
	}

	p_stream->update_item("account");
	p_stream->write_string("forbid_mask = ") << dw_forbidM;
	p_stream->write_string("^forbid_mask");
	p_stream->where_item();
	p_stream->write_string("id=") << dw_account_id;

	BOOL b_ret = database_.sql_execute(p_stream);
	if(b_ret == FALSE)
	{
		database_.free_result_query( p_result );
		database_.return_io(p_stream);
		return EGMDatabase_Busy;
	}
	database_.free_result_query( p_result );
	database_.return_io(p_stream);
	return E_Success;
}


//-------------------------------------------------------------------------------
//! 封IP
//-------------------------------------------------------------------------------
DWORD database_operator::forbid_ip(DWORD dw_ip)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return EGMDatabase_Busy;

	few_connect_client p_trans;

	char sz_ip[X_IP_LEN];
	strcpy(sz_ip,p_trans.ip_to_string(dw_ip));

	
	p_stream->insert_item("black_list");
	p_stream->write_string("ip = '").write_string(sz_ip).write_string("'");

	BOOL b_ret = database_.sql_execute(p_stream);
	if(b_ret == FALSE)
	{
		database_.return_io(p_stream);
		return EGMDatabase_Busy;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 解封IP
//-------------------------------------------------------------------------------
DWORD database_operator::remove_ip_forbid(DWORD dw_ip)
{
	sql_language_disposal* p_stream = database_.get_io();

	if (!VALID_POINT(p_stream))
		return EGMDatabase_Busy;

	few_connect_client p_trans;

	char sz_ip[X_IP_LEN];
	strcpy(sz_ip,p_trans.ip_to_string(dw_ip));

	
	p_stream->delete_item("black_list");
	p_stream->where_item();
	p_stream->write_string("ip = '").write_string(sz_ip).write_string("'");

	BOOL b_ret = database_.sql_execute(p_stream);
	if(b_ret == FALSE)
	{
		database_.return_io(p_stream);
		return EGMDatabase_Busy;
	}
	return E_Success;
}

VOID database_operator::reset_login_status( DWORD* p_account_ids, INT n_num, E_player_login_status e_dest )
{
	for (INT i=0; i<n_num; ++i)
	{
		fixed_login_status(p_account_ids[i], e_dest);
	}
}

BOOL database_operator::load_all_fatigue_info( package_map<DWORD, s_anti_addiction_info*> &map_fatigue_info )
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	
	p_stream->select_item("fatigue_time", 
						"account_name_crc,"			
						"acc_online_time,"		
						"acc_offline_time"	
						);

	execute_result* p_result = database_.sql_query(p_stream);
	database_.return_io(p_stream);

	//! 查询结果为空
	if( !VALID_POINT(p_result) || p_result->get_row_count() <= 0) return FALSE;

	s_anti_addiction_info* tmp = NULL;
	INT n_count = p_result->get_row_count();
	for (INT i=0; i<n_count; ++i)
	{
		DWORD dw_account_name_crc = (*p_result)[0].get_dword();
		tmp = map_fatigue_info.find(dw_account_name_crc);
		if (!VALID_POINT(tmp))
		{
			tmp = new s_anti_addiction_info;
			map_fatigue_info.add(dw_account_name_crc, tmp);
		}

		tmp->dw_account_name_crc = dw_account_name_crc;
		tmp->n_account_online_time_min = (*p_result)[1].get_int();
		tmp->n_account_offline_time_min = (*p_result)[2].get_int();

		p_result->next_row();
	}

	database_.free_result_query(p_result);
	return TRUE;
}

BOOL database_operator::load_online_guard_account_IDs( package_map<DWORD, DWORD> &map_accounts )
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;


	p_stream->select_item("account", "id, world_name_crc");
	p_stream->where_item();
	p_stream->write_string("login_status=1 and guard=1");

	execute_result* p_result = database_.sql_query(p_stream);
	database_.return_io(p_stream);

	
	if( !VALID_POINT(p_result) || p_result->get_row_count() <= 0) return FALSE;

	INT nCount = p_result->get_row_count();

	map_accounts.clear();
	for (INT i=0; i<nCount; ++i)
	{
		DWORD dw_account_id = (*p_result)[0].get_dword();
		DWORD dw_world_name_crc = (*p_result)[1].get_dword();
		
		if (!VALID_VALUE(map_accounts.find(dw_account_id)))
		{
			map_accounts.add(dw_account_id, dw_world_name_crc);
		}

		p_result->next_row();
	}

	database_.free_result_query(p_result);
	return TRUE;
}

BOOL database_operator::load_cache_account_data( package_safe_map<DWORD, s_account_data*> &map_account_data, package_safe_map<DWORD, DWORD> &map_name_crc_to_account_id)
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	tool unit;
	few_connect_client p_trans;

	
	tag_mysql_connect* p_con = database_.get_idlesse_connect();

	p_stream->select_item("account", "id, name,guard,ip");

	
	database_.return_use_connect(p_con);

	execute_result* p_result = database_.sql_query(p_stream);

	
	database_.return_io(p_stream);

	
	if( !VALID_POINT(p_result) || p_result->get_row_count() <= 0) return FALSE;

	DWORD dw_account_id = INVALID_VALUE;
	DWORD dw_account_name_crc = INVALID_VALUE;
	s_account_data* p_account_data = NULL;

	map_account_data.clear();
	map_name_crc_to_account_id.clear();

	BOOL b_rtv = TRUE;

	for (INT i=0; i<p_result->get_row_count(); ++i)
	{
		
		p_account_data = new s_account_data;
		if (!VALID_POINT(p_account_data))
		{
			b_rtv = FALSE;
			break;
		}

		dw_account_id = (*p_result)[0].get_dword();

		memcpy(p_account_data->sz_account_name, (*p_result)[1].get_string(), sizeof(CHAR)*16);
		p_account_data->b_guard			=	(*p_result)[2].get_bool();

		
		char sz_ip[X_IP_LEN] = "";
		memcpy(sz_ip,(*p_result)[3].get_string(),(*p_result)[3].get_length());
		p_account_data->dw_ip = p_trans.stringip_to_ip(sz_ip);

		dw_account_name_crc = unit.crc32(p_account_data->sz_account_name);

		if (!VALID_VALUE(map_account_data.find(dw_account_id)))
		{
			map_account_data.add(dw_account_id, p_account_data);
		}
		if (!VALID_VALUE(map_name_crc_to_account_id.find(dw_account_name_crc)))
		{
			map_name_crc_to_account_id.add(dw_account_name_crc, dw_account_id);
		}

		p_result->next_row();
	}

	
	database_.free_result_query(p_result);

	return b_rtv;

}

VOID database_operator::save_fatigue_info( s_anti_addiction_info* p_fatigue_info )
{
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return ;

	p_stream->replace_item("fatigue_time");
	p_stream->write_string("acc_online_time=")<<p_fatigue_info->n_account_online_time_min;
	p_stream->write_string(",acc_offline_time=")<<p_fatigue_info->n_account_offline_time_min;
	p_stream->write_string(",account_name_crc=")<<p_fatigue_info->dw_account_name_crc;

	database_.sql_execute(p_stream);

	database_.return_io(p_stream);
}

BOOL database_operator::load_fatigue_info(DWORD dw_account_name_crc, s_anti_addiction_info* p_fatigue_info )
{
	if (!VALID_POINT(p_fatigue_info)) return FALSE;
	sql_language_disposal* p_stream = database_.get_io();
	if( !VALID_POINT(p_stream) ) return FALSE;

	
	p_stream->select_item("fatigue_time", "acc_online_time, acc_offline_time");
	p_stream->where_item();
	p_stream->write_string("account_name_crc=") << dw_account_name_crc;

	execute_result* p_result = database_.sql_query(p_stream);
	database_.return_io(p_stream);

	
	if( !VALID_POINT(p_result) || p_result->get_row_count() != 1) return FALSE;

	p_fatigue_info->dw_account_name_crc = dw_account_name_crc;
	p_fatigue_info->n_account_online_time_min = (*p_result)[0].get_int();
	p_fatigue_info->n_account_offline_time_min = (*p_result)[1].get_int();

	database_.free_result_query(p_result);
	return TRUE;	
}


VOID call_back(db_interface* p_database, INT n_reason, INT n_param)
{
	if (n_reason == db_interface::EOE_System)
	{
		SI_LOG->write_log(_T("odbc Warning CallBack: Reason  %s , n_param = %u\r\n"), _T("EDBE_System"), n_param);
	}
	else if(n_reason == db_interface::EOE_QueueFull)
	{
		SI_LOG->write_log(_T("odbc Warning CallBack: Reason  %s , n_param = %u\r\n"), _T("EDBE_QueueFull"), n_param);
	}
	else if(n_reason == db_interface::EOE_PoolFull)
	{
		SI_LOG->write_log(_T("odbc Warning CallBack: Reason  %s , n_param = %u\r\n"), _T("EDBE_PoolFull"), n_param);
	}
	else
	{
		SI_LOG->write_log(_T("odbc Warning CallBack: Reason Unknow,n_reason = %u, n_param = %u\r\n"), n_reason, n_param);
	}
}