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

#include "../../common/WorldDefine/svn_revision.h"	

#include "si_login_server.h"
#include "si_world_mgr.h"
#include "user_mgr.h"
#include "database.h"
#include "verify_policy_mgr.h"
//#include "montioring_session.h"
#include "GMSession.h"
#include "anti_addiction_mgr.h"
#include "anti_addiction_session.h"
#include "anti_addiction_server.h"


#define CONFIG_INI  "server_config\\login\\login"

file_container*			login_server::p_var_config = NULL;
file_container*			login_server::p_var_gm = NULL;
thread_manager*					login_server::p_thread = NULL;


login_server::login_server() : b_terminate_update(FALSE), n_cpu_num(0), dw_total_phys(0), dw_avail_phys(0), dw_total_tun_minute(0),
							dw_idle_time(0), dw_tick(0), b_auto_regist_account(false)
{
	login_server::p_var_config = new file_container;

	login_server::p_var_gm = new file_container;
	
	p_log = new log_file;
	p_vfs = new file_system;
	login_server::p_thread = new thread_manager;
}

login_server::~login_server()
{
	SAFE_DELETE(p_log);
	SAFE_DELETE(p_vfs);
	SAFE_DELETE(login_server::p_thread);
}


BOOL login_server::init(HINSTANCE hInst)
{
	///* 定时关服务器
	tagDWORDTime dwShutDownTime;
	ZeroMemory(&dwShutDownTime, sizeof(dwShutDownTime));
	dwShutDownTime.year = SHUT_DOWN_YEAY;
	dwShutDownTime.month = SHUT_DOWN_MONTH;

	DWORD dwCurTime = GetCurrentDWORDTime();

	if (dwCurTime > dwShutDownTime)
		return false;
	//*/


	p_log->create_log();
	
	if( !VALID_POINT(login_server::p_var_config) )
	{
		return FALSE;
	}
	
	TCHAR t_sz_path[MAX_PATH];
	ZeroMemory(t_sz_path, sizeof(t_sz_path));
	if (!get_file_io_mgr()->get_ini_path(t_sz_path, _T(CONFIG_INI))||
		!login_server::p_var_config->load(g_login.get_file_system(), t_sz_path))
	{
		ERROR_CLUE_ON(_T("配置文件未找到"));
		return FALSE;
	}

	if( !VALID_POINT(login_server::p_var_gm) ) return FALSE;
	if( !get_file_io_mgr()->get_ini_path(t_sz_path, _T("server_config/login/gm_access"))||
		!login_server::p_var_gm->load(g_login.get_file_system(), t_sz_path))
	{
		ERROR_CLUE_ON(_T("配置文件未找到"));
		return FALSE;
	}

	
	dw_id			=	login_server::p_var_config->get_dword(_T("id section"));

	f_login_limit_num = login_server::p_var_config->get_float(_T("limit login_limit"));

	LONG l_width		=	login_server::p_var_config->get_dword(_T("width display"));
	LONG l_height	=	login_server::p_var_config->get_dword(_T("height display"));


	b_auto_regist_account = login_server::p_var_config->get_dword(_T("auto_regist login_limit"));

	get_window()->init(l_width, l_height, TRUE);
	get_window()->create_window(_T("LoginServer"), hInst);

	n_cpu_num = get_tool()->get_cup_num();
	dw_total_tun_minute = 0;

	//! 是否模拟防沉迷服务器
	b_sim_fatigue_server = (BOOL)p_var_config->get_dword(_T("switch sim_fatigueserver"));

	// 生成各个单件
	/*new ProofPolicyMgr;
	new PlayerMgr;
	new FpWorldMgr;
	new database_operator;
	new anti_addiction_mgr;
	new FatigueSession;*/

	if( FALSE == g_datebase.init() )
	{
		SI_LOG->write_log(_T("db_interface  init Failure\r\n"));
		return FALSE;
	}

	if( FALSE == g_verify_policy_mgr.init() )
	{
		SI_LOG->write_log(_T("ProofPolicyMgr init Failure\r\n"));
		return FALSE;
	}

	if( FALSE == g_fpWorldMgr.init() )
	{
		SI_LOG->write_log(_T("FpWorldMgr init Failure\r\n"));
		return FALSE;
	}

	if( FALSE == g_PlayMgr.init() )
	{
		SI_LOG->write_log(_T("PlayerMgr init Failure !\r\n"));
		return FALSE;
	}

	if (g_rtSession.Init() == FALSE)
	{
		MessageBox(NULL, _T("监控Session初始化失败"), _T("login_server"), MB_OK);
		SI_LOG->write_log(_T("RT_Session init Failure\r\n"));
		return FALSE;
	}

	if (!g_anti_addiction_session.init())
	{
		MessageBox(NULL, _T("FagitueSession初始化失败"), _T("login_server"), MB_OK);
		SI_LOG->write_log(_T("Fatigue_Session init Failure\r\n"));
		return FALSE;
	}

	if (!g_anti_mgr.init())
	{
		MessageBox(NULL, _T("FagitueMgr初始化失败"), _T("login_server"), MB_OK);
		SI_LOG->write_log(_T("FagitueMgr init Failure\r\n"));
		return FALSE;
	}

	if(is_sim_fatigue_server())
	{
		if (!g_anti_server.init())
		{
			MessageBox(NULL, _T("FagitueServer初始化失败"), _T("login_server"), MB_OK);
			SI_LOG->write_log(_T("FagitueServer init Failure\r\n"));
			return FALSE;
		}
	}

	g_datebase.reset_all_login_status(epls_loging, epls_unknown);
	g_datebase.reset_all_login_status(epls_online, epls_unknown);

	if( FALSE == p_thread->create_thread( _T("thread_update"), &login_server::static_thread_update, this) )
		return FALSE;
	
	return TRUE;
}

//---------------------------------------------------------------------------------
VOID login_server::destroy()
{
	
	Interlocked_Exchange((LPLONG)&b_terminate_update, TRUE );
	p_thread->waitfor_thread_destroy(_T("thread_update"), INFINITE);
	
	// 删除单件指针
	/*delete g_PlayMgr.getSingletonPtr();
	delete g_fpWorldMgr.getSingletonPtr();
	delete g_datebase.getSingletonPtr();
	delete g_verify_policy_mgr.getSingletonPtr();
	delete g_anti_addiction_session.getSingletonPtr();*/
	//delete sRTSession.getSingletonPtr();
	g_PlayMgr.destroy();
	g_fpWorldMgr.destroy();
	g_datebase.destroy();
	g_verify_policy_mgr.destroy();
	g_anti_addiction_session.destroy();
	g_rtSession.Destroy();

	
	SAFE_DELETE(login_server::p_var_config);
	
	SAFE_DELETE(login_server::p_var_gm)

	get_window()->destroy();
	p_log->close_file();

	

}

//-----------------------------------------------------------------------------------
VOID login_server::main_loop()
{
	DWORD dw_msg, dw_param1, dw_param2;
	static DWORD dw_time_keepr = timeGetTime();

	while ( FALSE == get_window()->message_loop() && FALSE == b_terminate_update )
	{
		if( FALSE == get_window()->is_window_active() )
		{
			Sleep(30);
			continue;
		}

		while( get_window()->peek_window_message( dw_msg, dw_param1, dw_param2 ) )
		{
			if( dw_msg == WM_QUIT )
			{
				return;
			}
			if(dw_msg == WM_MYSELF_USER)
			{
				if(dw_param1 == 1)
				{
					get_window()->print_list();
				}
			}
		}

		//! 更新内存信息
		update_memory_usage();

		dw_total_tun_minute = timeGetTime() - dw_time_keepr;
		INT n_hour = dw_total_tun_minute / 3600000;
		INT n_min = (dw_total_tun_minute % 3600000) / 60000;
		INT n_sec = ((dw_total_tun_minute % 3600000) % 60000) / 1000;

		
		get_window()->watch_info(_T("cpu_num:"),				n_cpu_num);
		get_window()->watch_info(_T("mem_total:"),				dw_total_phys);
		get_window()->watch_info(_T("mem_avail:"),				dw_avail_phys);

		
		get_window()->watch_info(_T("sec: "),					n_sec);
		get_window()->watch_info(_T("min: "),					n_min);
		get_window()->watch_info(_T("hour: "),					n_hour);

		get_window()->watch_info(_T("tick: "),					dw_tick);
		get_window()->watch_info(_T("idle: "),					dw_idle_time);
		
		get_window()->watch_info(_T("all:"),					g_PlayMgr.get_player_num());
		get_window()->watch_info(_T("loging:"),				g_PlayMgr.get_player_loging());
		get_window()->watch_info(_T("proof_wait:"),			g_PlayMgr.get_proof_result_num());

		get_window()->watch_info(_T("---------------------"), 0);

		get_window()->watch_info(_T("available_stream_num:"),	g_datebase.get_available_stream_num());
		get_window()->watch_info(_T("all_stream_num:"),		g_datebase.get_all_stream_num());
		get_window()->watch_info(_T("asy_stream_num:"),		g_datebase.get_unhandled_asyn_stream_num());

		Sleep(50);
	}
}


UINT login_server::thread_update()
{
	DWORD dw_time = 0;

	//modify mmz at 2010.9.17 release也记dump
//#ifdef DEBUG
	_set_se_translator(serverdump::si_translation); 

	try
	{
//#endif
		while( !b_terminate_update )
		{
			
			dw_time = timeGetTime();

			
			lock_update();

			
			g_fpWorldMgr.update();

			
			g_PlayMgr.update();

			
			g_rtSession.Update();

			
			g_PlayMgr.start_send_all_msg();

			
			g_anti_addiction_session.update();

			//! 每分钟发给中心激活玩家在线情况
			g_anti_mgr.update();

			
			unlock_update();

			
			dw_time = timeGetTime() - dw_time;
			if( dw_time < TICK_TIME )
			{
				Interlocked_Exchange((LPLONG)&dw_idle_time, TICK_TIME - dw_time);
			}
			else
			{
				Interlocked_Exchange((LPLONG)&dw_idle_time, 0);
			}

			
			Interlocked_Exchange_Add((LPLONG)&dw_tick, 1);

			
			if( dw_time < TICK_TIME )
			{
				Sleep(TICK_TIME - dw_time);
			}
		}
//#ifdef DEBUG
	}
	catch(serverdump::throw_exception)
	{
		
		if( get_tool()->is_debug_present() )
		{
			throw;
		}
		else
		{
			exit(1);
		}
	}
//#endif
	_endthreadex(0);

	return 0;
}

UINT login_server::static_thread_update(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	login_server* p_this = (login_server*)p_data;
	return p_this->thread_update();
}


VOID login_server::update_memory_usage()
{
	MEMORYSTATUS memory_status;
	memory_status.dwLength = sizeof(memory_status);
	GlobalMemoryStatus(&memory_status);

	dw_total_phys = memory_status.dwTotalPhys;
	dw_avail_phys = memory_status.dwAvailPhys;

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
}

login_server g_login;





