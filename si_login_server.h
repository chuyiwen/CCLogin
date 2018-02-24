/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _LOGIN_SERVER_H_
#define _LOGIN_SERVER_H_


class login_server
{
public:

	login_server();
	~login_server();


	BOOL			init(HINSTANCE hInst);
	VOID			main_loop();
	VOID			destroy();
	
	UINT			thread_update();
	static UINT WINAPI static_thread_update(LPVOID p_data);

	VOID			lock_update()		{ update_mutex.lock(); }
	VOID			unlock_update()		{ update_mutex.un_lock(); }


	VOID			shutdown()			{ Interlocked_Exchange((long*)(&b_terminate_update), TRUE); }


	DWORD			get_section_id()		{ return dw_id; }


	log_file*			get_log()	{ return p_log; }
	BOOL			is_sim_fatigue_server()	{	return b_sim_fatigue_server;	}

	file_system*	get_file_system() { return p_vfs; }

	FLOAT get_login_limit_num() { return f_login_limit_num; }

	BOOL isAutoRegist() { return b_auto_regist_account; }
public:
	static file_container*			p_var_config;
	static file_container*			p_var_gm;
	static thread_manager*					p_thread;
	
private:
	VOID			update_memory_usage();

private:
	
	log_file*					p_log;

	file_system*			p_vfs;


	DWORD					dw_id;

	FLOAT					f_login_limit_num;


	volatile BOOL			b_terminate_update;		
	mutex					update_mutex;			
	

	INT						n_cpu_num;		
	//! 总物理内存
	DWORD					dw_total_phys;		
	//! 可用物理内存
	DWORD					dw_avail_phys;			
	DWORD					dw_total_tun_minute;		
	BOOL					b_sim_fatigue_server;	//! 是否启用简单防沉迷服务器

	BOOL					b_auto_regist_account;	// 是否自动注册账号

	volatile DWORD	 		dw_idle_time;			//! 空闲时间
	volatile DWORD			dw_tick;				//! 服务器心跳
};

extern login_server g_login;

#define SI_LOG	(g_login.get_log())

#endif


