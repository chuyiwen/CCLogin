/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _ANTI_ADDICTION_SERCER_H_
#define _ANTI_ADDICTION_SERCER_H_

struct s_anti_addiction_info;
struct tagNC_ANTIENTHRALL;
class anti_addiction_server;

//-------------------------------------------------------------------------
//! ·À³ÁÃÔ¼ÆÊ±Æ÷
//-------------------------------------------------------------------------
class anti_addiction_timer
{
	typedef package_map<DWORD, s_anti_addiction_info*>	anti_addiction_info_map;

public:
	anti_addiction_timer(anti_addiction_server* an_a_server)
		:p_fatigue_server(an_a_server){}
	~anti_addiction_timer();



	VOID	updata();
	VOID	reset();

	VOID	record_ol_guard_accounts(tagNC_ANTIENTHRALL* p_msg);
	VOID	notify_ol_guard_users(s_anti_addiction_info* p_an_a_info);
	VOID	notify_login_user(LPCSTR sz_account_name);


	anti_addiction_info_map &get_fatigue_info_map(){return map_all_fatigue;}

private:
	
	package_safe_map<DWORD, LPSTR>		map_crc_name;		

	anti_addiction_info_map		map_all_fatigue;	
	anti_addiction_server*		p_fatigue_server;
};


//-------------------------------------------------------------------------
//! ·À³ÁÃÔ·þÎñÆ÷
//-------------------------------------------------------------------------
class anti_addiction_server
{
public:
	anti_addiction_server();

	~anti_addiction_server(){ /*destroy();*/ }

public:
	BOOL	init();
	VOID	destroy();

	VOID	send_message(PVOID p_msg, DWORD dw_size);

private:
	UINT	thread_update();
	static UINT WINAPI static_thread_update(LPVOID p_data);

	//-------------------------------------------------------------------------
	DWORD	login_call_back(LPBYTE p_byte, DWORD dw_size);
	DWORD	logout_call_back(DWORD dw_section_id);

	//-------------------------------------------------------------------------
	BOOL	init_config();

	//-------------------------------------------------------------------------
	VOID	update_session();

	//-------------------------------------------------------------------------
	DWORD	handle_online_accounts(tag_net_message* p_cmd);
	DWORD	handle_query_state( tag_net_message* p_cmd);

private:

	few_connect_server*			p_session;
	//-------------------------------------------------------------------------
	volatile BOOL				b_terminate_update;
	volatile BOOL				b_terminate_connect;

	//-------------------------------------------------------------------------
	INT							n_port;				// ¶Ë¿Ú

	anti_addiction_timer		anti_addiction_timer_;
	DWORD						dw_section_id;

};

extern anti_addiction_server g_anti_server;

#endif

