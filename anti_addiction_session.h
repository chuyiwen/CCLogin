/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#ifndef _ANTI_ADDICTION_SESSION_H_
#define _ANTI_ADDICTION_SESSION_H_


//! �����ԻỰ
class anti_addiction_session
{
public:
	anti_addiction_session() : n_port(0), b_terminate_update(FALSE), b_terminate_connect(FALSE), b_connected(FALSE) {}
	~anti_addiction_session() { /*destroy();*/ }

public:

	BOOL	init();
	
	VOID	destroy();

	VOID	send_query_status(DWORD dw_account);
	VOID	send_online_guard_accounts( DWORD *p_ol_accounts, DWORD dw_num );
	DWORD	handle_guard_account_notify( PVOID p_msg );


	VOID	update();

	
	//! �Ƿ�����
	BOOL	is_connected()	{	return p_transport->is_connect();	}

private:

	//! �����߳�
	UINT	thread_connect_server();
	static UINT WINAPI static_thread_connect_server(LPVOID p_data);

	//! ��ʼ������
	BOOL	init_config();


private:

	few_connect_client*			p_transport;

	//-------------------------------------------------------------------------
	volatile BOOL				b_terminate_update;
	volatile BOOL				b_terminate_connect;

	//-------------------------------------------------------------------------
	CHAR						_sz_ip[X_IP_LEN];		//! IP
	INT							n_port;					//! �˿�
	volatile BOOL				b_connected;			

};

extern anti_addiction_session g_anti_addiction_session;


#endif 