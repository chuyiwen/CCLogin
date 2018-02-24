/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _ANTI_ADDICTION_MGR_H_
#define _ANTI_ADDICTION_MGR_H_

#include "../common/ServerDefine/anti_addiction_server_define.h"

//! �����Թ���
class anti_addiction_mgr
{
public:
	BOOL	init();

	//! �����������ߵķ��������
	BOOL	load_all_accounts();

	//! ֪ͨ�����Է�����
	VOID	notify_anti_addiction_server();

	//! ���ָ����Ϸ��������
	VOID	reset_world_accounts(DWORD dw_world_name_crc_reset, DWORD* p_ol_account_ids, INT n_olaccount_id_num);

	//! ����,�ǳ�֪ͨ
	VOID	login_notify(DWORD dw_account, DWORD dw_worldNameCrc, BOOL b_guard);
	VOID	logout_notify(DWORD dw_account);

	//! ֪ͨ����ۼ�����ʱ��
	VOID	notify_player(const s_anti_addiction_notify* p_notify);

	//! ����Ϸ������������Ϣ
	VOID	send_world_notifys();

	VOID	update();

private:
	package_map<DWORD, DWORD>						map_account_to_name_crc;
	mutex									set_lock;

	package_map<DWORD, package_list<s_anti_addiction_notify*>* > map_notify_lists;
	mutex									map_lock;
	
	INT										n_min_counter;
	INT										n_circle_unit;
};

extern anti_addiction_mgr g_anti_mgr;

#endif //_ANTI_ADDICTION_MGR_H_