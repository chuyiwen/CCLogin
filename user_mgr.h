/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _USER_MGR_H_
#define _USER_MGR_H_

#include "user.h"

class user;
class verify_policy;


//-----------------------------------------------------------------------------
class user_mgr/* : public Singleton<user_mgr>  */
{
public:

    user_mgr();
	
    ~user_mgr();


    BOOL			init();
    VOID			update();
    VOID			destroy();


	DWORD			get_curr_ver()		{ return dw_curr_ver; }
	INT				get_player_num()		{ return n_player_num; }
	INT				get_player_loging()	{ return n_player_loging; }
	INT				get_proof_result_num()	{ return n_proof_result_num; }
	E_proof_policy	get_proof_policy()	{ return e_policy_; }
	verify_policy*	get_proof_policy_ptr()	{ return p_policy_; }


    LPBYTE			recv_msg(DWORD& dw_size, INT& n_msg_num, DWORD dw_cd_index);
	VOID			return_msg(LPBYTE p_msg);
	VOID			send_msg(DWORD dw_cd_index, LPBYTE p_msg, DWORD dw_size);

	VOID			start_send_all_msg()	{ p_net_session->active_send(); }

	//--------------------------------------------------------------------------
	//! ����
	//--------------------------------------------------------------------------
    VOID			kick(DWORD dw_cd_index)		{ p_net_session->kick_client(dw_cd_index); }

	//--------------------------------------------------------------------------
	//! ID����
	//--------------------------------------------------------------------------
	DWORD			generate_client_id()				{ ++dw_client_id_gen; return dw_client_id_gen; }
    

	VOID			add_player_to_all(user* p_user)				{ map_all_user.add(p_user->get_client_id(), p_user); }
	VOID			add_player_to_logining(user* p_user)		{ map_logining_user.add(p_user->get_client_id(), p_user); }
	VOID			remove_player_rrom_all(DWORD dw_client_id)		{ map_all_user.erase(dw_client_id); }
	VOID			remove_player_from_logining(DWORD dw_client_id)	{ map_logining_user.erase(dw_client_id); }
	VOID			player_logout(user* p_user);

	VOID			add_account(DWORD dw_account_id, DWORD dw_client_id);
	VOID			remove_account(DWORD dw_account_id);
	BOOL			is_account_exist(DWORD dw_account_id)			{ return map_account_id_client_id.is_exist(dw_account_id); }

	const s_account_data*		get_cached_account_data(DWORD dw_account_id)	{	return map_account_data.find(dw_account_id);		}
	VOID			cache_account_name(DWORD dw_account_id, const CHAR* sz_account_name);
	VOID			cache_ip_addres( DWORD dw_account_id, DWORD dw_ip);
	VOID			cache_guard( DWORD dw_account_id, BOOL b_guard);
	VOID			erase_cached_account_data(DWORD dw_account_id);
	VOID			clean_cached_account_datas();
	VOID			map_account_name_to_account_id(LPCSTR sz_account_name, DWORD dw_account_id);
	DWORD			get_account_id_by_account_name(LPCSTR sz_account_name);

	//--------------------------------------------------------------------------
	//! ��֤
	//--------------------------------------------------------------------------
	VOID			proof(DWORD dw_client_id, LPCSTR sz_accout_name, LPCSTR sz_password, LPCSTR sz_guid);
	VOID			proof_facebook(DWORD dw_client_id, LPCSTR szGUID, LPCSTR szCode, INT nType);
	
	tstring			create_network_log();

private:

	//--------------------------------------------------------------------------
	BOOL			init_config();


	//--------------------------------------------------------------------------
	VOID			update_session();
	VOID			update_proof_result();

	//--------------------------------------------------------------------------
    UINT			login_call_back(tag_unit*, tag_login_param*);
    UINT			logout_call_back(DWORD);

	//--------------------------------------------------------------------------
	UINT			proof_call_back(INT n_ret, VOID* p_result);
	UINT			facebook_proof_call_back(INT n_ret, VOID* p_result);
private:
    
	//--------------------------------------------------------------------------
	IOCP*					p_net_session;
    INT							n_port;

	//--------------------------------------------------------------------------
    DWORD						dw_client_id_gen;

	//! �汾�ź�����
    DWORD						dw_curr_ver;

	//! ͳ����Ϣ
	volatile INT				n_player_num;
	volatile INT				n_player_loging;
	volatile INT				n_proof_result_num;

	//! ������
	//! ���е�user,��ClientID Ϊ key
	package_safe_map<DWORD, user*>				map_all_user;			

	//! ���ڵ�¼�����
	package_map<DWORD, user*>					map_logining_user;		

	//! ���ߵ�AccountID��ClientID��Ӧ��
	package_map<DWORD, DWORD>					map_account_id_client_id;

	//! ���سɹ������
	package_safe_list<s_proof_result_full*>		list_proof_result;		
	
	//! ���е�AccountID�Ļ������ݣ�����AccountName��guard��ip ��ʼ�����أ���̬����
	package_safe_map<DWORD, s_account_data*>	map_account_data;		

	//! accountid -> namecrc	��ʼ��ʱ���أ����Ҷ�̬���
	package_safe_map<DWORD, DWORD>				map_account_name_crc_to_account_id;	


	//! ��֤����
	E_proof_policy						e_policy_;				//! ��֤��ʽ
	verify_policy*						p_policy_;				//! ��֤����
	verify_policy*						facebook_policy_;		//! ��֤����

	std::multimap<tstring, DWORD>		map_gm_vs_ip_;			//! gmָ��ip
	BOOL								b_use_gm_access_;		//! �Ƿ�ʹ��gm��ַ����

};



extern user_mgr g_PlayMgr;

#endif

