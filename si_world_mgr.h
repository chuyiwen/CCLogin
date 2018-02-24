/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _WORLD_MGR_H_
#define _WORLD_MGR_H_

class user;
class si_world;

class world_mgr/*: public Singleton<world_mgr>*/
{
public:

    world_mgr();
    ~world_mgr();

    BOOL		init();
    VOID		update();
    VOID		destroy();
	VOID		update_world_state(si_world* p_world);

	LPBYTE		receive_msg(DWORD dw_id, DWORD& dw_msg_size, INT& n_unrecved)	{ return p_session->recv_msg(dw_id, dw_msg_size, n_unrecved); }
	VOID		return_msg(DWORD dw_id, LPBYTE p_msg)						{ p_session->free_recv_msg(dw_id, p_msg); }
	VOID		send_msg(DWORD dw_id, LPVOID p_msg, DWORD dw_size)			{ p_session->send_msg(dw_id, p_msg, dw_size); }
	VOID		handle_cmd(LPVOID p_msg, DWORD dw_size, DWORD dw_param)		{ serverframe::net_command_manager::get_singleton().handle_message((tag_net_message*)p_msg, dw_size, dw_param); }


    si_world*	get_world(DWORD dw_name_crc)	{ return map_world.find(dw_name_crc); }
	DWORD		get_world_num()				{ return map_world.size();		}

	VOID		add_to_world(user* p_user, DWORD dw_world_name_crc);

private:

	VOID		register_world_msg();
	VOID		unregister_world_msg();

	//-----------------------------------------------------------------------
	DWORD		login_call_back(LPBYTE p_byte, DWORD dw_size);
	DWORD		logout_call_back(DWORD dw_param);


	//-----------------------------------------------------------------------
	DWORD		handle_certification(tag_net_message* p_msg, si_world* p_world);
	DWORD		handle_zone_server_status(tag_net_message* p_msg, si_world* p_world);
	DWORD		handle_player_login(tag_net_message* p_msg, si_world* p_world);
	DWORD		handle_player_logout(tag_net_message* p_msg, si_world* p_world);
	DWORD		handle_player_will_login(tag_net_message* p_msg, si_world* p_world);
	DWORD		handle_kick_log(tag_net_message* p_msg, si_world* p_world);
	DWORD		handle_world_closed(tag_net_message* p_msg, si_world* p_world);


private:
   

	few_connect_server*			p_session;				

    package_map<DWORD, si_world*>		map_world;			

	DWORD						dw_world_golden_code;	
	DWORD						dw_login_server_golden_code;	
};

extern world_mgr g_fpWorldMgr;


#endif
