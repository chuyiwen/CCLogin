/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _LOGIN_SERVER_DEFINE_H_
#define _LOGIN_SERVER_DEFINE_H_


#include "../common/ServerDefine/login_server_define.h"

#define MIBAO_LENGTH		240			// �ܱ����̶�

const INT CON_LOST			= -1;	// ���ӶϿ���־

//------------------------------------------------------------------------------
struct s_world_data 
{
	BOOL			b_valid;					//! ��ǰ�Ƿ�����
	TCHAR			sz_name[X_SHORT_NAME];	    //! World����
	DWORD			dw_name_crc;				//! ����CRCֵ
	E_world_status	e_status;					//! ��ǰ״̬
	INT				n_port;						//! �˿�
	DWORD			dw_ip;						//! IP
	INT				n_max_online_num;			//! �����������
	INT				n_cur_online_num;			//! ��ǰ��������
	BOOL			b_auto_seal;				//! �Ƿ��Զ�
	DWORD			dw_world_id;				//! worldid

	s_world_data()
	{
		ZeroMemory(this, sizeof(*this));
	}
};


//----------------------------------------------------------------
struct s_player_data 
{
	DWORD				dw_client_id;						
	DWORD				dw_cd_index;						//! ����ID
	DWORD				dw_account_id;						
	CHAR				sz_account_name[X_SHORT_NAME];		
	DWORD				dw_mibao_crc;						//! �ܱ�crc
//	BOOL				bDisabled;							// �Ƿ��ͣ
	DWORD				dw_frobid_mask;						//! ��ͣ���� ÿ1λ��Ӧһ�ַ�ͣ����
	BOOL				b_guard;							//! �������û�
	INT					n_acc_online_sec;					//! �ۼƵ�¼ʱ��
	DWORD				dw_ip;								//! �ͻ���IP
	DWORD				dw_world_name_crc;					//! ��Ϸ��������CRCֵ
	BYTE				by_privilege;						//! Ȩ��
	BYTE				by_SpecialAccount;
	BYTE				by_reserved[2];						//! �����־

	s_player_data()		{ ZeroMemory(this, sizeof(*this)); }
	VOID Reset()		{ ZeroMemory(this, sizeof(*this)); }
};


//----------------------------------------------------------------
struct s_account_data
{
	CHAR				sz_account_name[X_SHORT_NAME];
	DWORD				dw_ip;
	BOOL				b_guard;
};

#endif