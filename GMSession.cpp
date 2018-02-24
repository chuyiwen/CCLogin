/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

/**
 *	@file		GMSession
 *	@author		mwh
 *	@date		2011/04/07	initial
 *	@version	0.0.1.0
 *	@brief		连接GM服务器
*/

#include "StdAfx.h"
#include "../common/ServerDefine/GMServerMessage.h"
#include "GMSession.h"
#include "si_login_server.h"
#include "si_world_mgr.h"
#include "si_world.h"
#include "database.h"
#include "verify_policy.h"

#define GMSESSIONCONNECTTHREADNAME _T("GMSessionConnectGMServer")
#define UniCode2Unicode8(Str) get_tool()->unicode_to_unicode8(Str)

GMSession g_rtSession;

GMSession::GMSession()
:mPort(0),mWordID(0),mSectionID(0),mSendStatusTick(STATUSREPORTINTERVAL)
,mZoneNumber(0),mTerminateConnect(FALSE),mConnected(FALSE),mNetSession(0)
{
}

GMSession::~GMSession()
{

}

BOOL GMSession::Init()
{
	// 初始化成员属性
	mIP	= login_server::p_var_config->get_string(_T("ip gm_server"));
	mPort =	login_server::p_var_config->get_dword(_T("port gm_server"));
	mSectionID = 0;//login_server::p_var_config->get_dword(_T("section_id world"));
	mZoneNumber = login_server::p_var_config->get_int(_T("num zone_server"));

	for(INT n = 0; n < mZoneNumber; ++n)
	{
		TCHAR xZone[MAX_PATH] = {0};
		_stprintf(xZone, _T("zone%d"), n);

		mWordID.push_back(login_server::p_var_config->get_dword(_T("id"), xZone));
		mWorldName.push_back(login_server::p_var_config->get_string(_T("name"), xZone));
		mWorldNameCrc.push_back(get_tool()->crc32(mWorldName.back().c_str()));
	}

	mNetSession = new few_connect_client;
	ASSERT(VALID_POINT(mNetSession));
	if(!VALID_POINT(mNetSession))
	{
		ERROR_CLUE_ON(_T("\n\nfatal error on create net session!!\n\n"));
		return FALSE;
	}

	this->RegisterCmd();

	if(!mNetSession->init()) return FALSE;
	if(!CreateConnectThread()) return FALSE;

	return TRUE;
}

VOID GMSession::Destroy()
{
	
	Interlocked_Exchange((LONG*)&mTerminateConnect, TRUE);
	login_server::p_thread->waitfor_thread_destroy(GMSESSIONCONNECTTHREADNAME, INFINITE);

	mNetSession->destory();
	SAFE_DELETE(mNetSession);

	// 注销消息管理
	UnregisterCmd();

	mWordID.clear();
	mWorldName.clear();
	mWorldNameCrc.clear();
}

VOID GMSession::Update()
{
 	this->UpdateSession();
 	this->SendServerInfo();
}

VOID GMSession::UpdateSession()
{
	if(!VALID_POINT(mNetSession))
		return;

	if(!mNetSession->is_connect() &&
	   !login_server::p_thread->is_thread_active(GMSESSIONCONNECTTHREADNAME))
	{
		ReConnect(); return;
	}
 
 	if(mNetSession->is_connect())
 		HandleMessage();
}

VOID GMSession::SendServerInfo()
{
	if(!mNetSession->is_connect() || !mConnected)
		return;

	--mSendStatusTick;
	if(mSendStatusTick > 0) return;

	mSendStatusTick = STATUSREPORTINTERVAL;

	NET_S2GMS_ServerInfo send;
	send.e_status = ews_well;
	send.n_online_num = send.n_max_online_num = 0;
	for(INT n = 0; n < mZoneNumber; ++n)
	{
		si_world* xWorld = g_fpWorldMgr.get_world(mWorldNameCrc[n]);
		if (!VALID_POINT(xWorld))
		{
			print_message(_T("GameServer Error id<%d> name<%s> error!\r\n"), mWordID[n],mWorldName[n].c_str());
			break;
		}

		if (xWorld->get_status() != ews_well)
		{
			send.e_status = ews_system_error;
		}
		else
		{
			send.n_online_num += xWorld->get_curr_online_num();
			send.n_max_online_num += xWorld->get_max_online_num();
		}	
	}
	verify_policy* p_policy = g_PlayMgr.get_proof_policy_ptr();
	if (VALID_POINT(p_policy) && p_policy->get_proof_server_status() != ews_well)
	{
		send.e_status = ews_proof_error;
	}
	mNetSession->send_msg(&send, send.dw_size);
}
UINT GMSession::ThreadConnect()
{
	ASSERT(VALID_POINT(mNetSession));

	THROW_EXCEPTION_START

	while(!mTerminateConnect && !mNetSession->is_connect())
	{
		if( !mNetSession->is_trying_create_connect() )
		{
			mNetSession->try_create_connect(UniCode2Unicode8(mIP.c_str()), mPort);
		}
		Sleep(100); 
		continue;
	}

	if(mNetSession->is_connect())
	{
		print_message(_T("connected to GMServer %s:%4d\r\n"), mIP.c_str(), mPort);

		NET_S2GMS_SeverLogin send;
		send.dw_crc = 1;
		send.e_type = EST_LOGINSERVER;
		mNetSession->send_msg(&send, send.dw_size);
	}
	
	THROW_EXCEPTION_END
	
	::_endthreadex(0);
	return 0;
}

UINT GMSession::ThreadCall(LPVOID lpVoid)
{
	ASSERT(VALID_POINT(lpVoid));
	return static_cast<GMSession*>(lpVoid)->ThreadConnect();
}

BOOL GMSession::ReConnect()
{
	Interlocked_Exchange((LONG*)&mTerminateConnect, TRUE);

	mNetSession->disconnect();
	login_server::p_thread->waitfor_thread_destroy(GMSESSIONCONNECTTHREADNAME, INFINITE);

	// 重新启动登陆服务器连接线程
	Interlocked_Exchange((LONG*)&mTerminateConnect, FALSE);
	this->CreateConnectThread();
	return TRUE;
}

BOOL GMSession::CreateConnectThread()
{
	if(!login_server::p_thread->create_thread(GMSESSIONCONNECTTHREADNAME, &GMSession::ThreadCall, this))
		return FALSE;

	while(!login_server::p_thread->is_thread_active(GMSESSIONCONNECTTHREADNAME))
		continue;

	return TRUE;
}

VOID GMSession::HandleMessage()
{
	DWORD dwSize = 0;
	LPBYTE lpMsg = mNetSession->recv_msg(dwSize);
	net_command_manager& netMan = serverframe::net_command_manager::get_singleton();
 	while(VALID_POINT(lpMsg))
 	{
  		// 处理消息
  		netMan.handle_message((tag_net_message*)lpMsg,dwSize, INVALID_VALUE);
  
  		// 回收资源
  		mNetSession->free_recv_msg(lpMsg);
 
 		lpMsg = mNetSession->recv_msg(dwSize);
 	}
}

//=================================================
//	消息处理注册
//=================================================
VOID GMSession::RegisterCmd()
{
	REGISTER_NET_MSG("NET_GMS2S_SeverLogin",	GMSession,	HandleServerLogin,		_T("Login GMServer"));
	REGISTER_NET_MSG("NET_GMS2S_forbid_account",	GMSession,	HandleForbidAccount,	_T("Forbid One Account"));
	REGISTER_NET_MSG("NET_GMS2S_forbid_ip",		GMSession,	HandleForbidIP,			_T("Forbid One IP"));
	REGISTER_NET_MSG("NET_GMS2S_SetMaxPlayerNumber",		GMSession,	HandleSetMaxPlayerNumber, _T("set world's max player"));
}
VOID GMSession::UnregisterCmd()
{
	UNREGISTER_NET_MSG("NET_GMS2S_SeverLogin",	GMSession,	HandleServerLogin);
	UNREGISTER_NET_MSG("NET_GMS2S_forbid_account",GMSession,	HandleForbidAccount);
	UNREGISTER_NET_MSG("NET_GMS2S_forbid_ip",		GMSession,	HandleForbidIP);
	UNREGISTER_NET_MSG("NET_GMS2S_SetMaxPlayerNumber",		GMSession,	HandleSetMaxPlayerNumber);
}
//=================================================
//	辅助函数
//=================================================
BOOL GMSession::SkipBlank(tstring& _Str)
{
	INT xPos = _Str.find_first_not_of(_T(' '));
	if(xPos != _Str.npos) _Str = _Str.substr(xPos);
	
	xPos = _Str.find(_T(' '));
	if(xPos != _Str.npos) _Str = _Str.substr(0,xPos);

	return !_Str.empty();
}
VOID GMSession::GetForbidAccount(tstring& _Account,vector<tstring>& _Out)
{
	// Split string with ,
	for( INT xPrePos = 0, xCurPos = 0; xCurPos != _Account.npos; xPrePos = xCurPos + 1)
	{
		tstring xAccountName;

		xCurPos = _Account.find(_T(','), xPrePos);
		if(xCurPos == _Account.npos)
			xAccountName = _Account.substr(xPrePos, _Account.length() - xPrePos);
		else
			xAccountName = _Account.substr(xPrePos, xCurPos - xPrePos);

		if(SkipBlank(xAccountName))_Out.push_back(xAccountName);
	}
}

DWORD GMSession::GetForbidWorldCrc(tag_net_message* lpMsg, vector<DWORD>& _Out)
{
	DWORD xWorldNameCRC = 0, xError = E_Success;
	NET_GMS2S_forbid_account* xPortocol = (NET_GMS2S_forbid_account*)lpMsg;
	
	//! 封停大区下所有的游戏世界
	if(_tcscmp(_T(""), xPortocol->sz_world) == 0)
	{
		for(INT n = 0; n < mZoneNumber; ++n)
			_Out.push_back(mWorldNameCrc[n]);
	}
	//! 封停是指定游戏世界
	else
	{
		for(INT n = 0; n < mZoneNumber; ++n)
		{
			if(_tcscmp(mWorldName[n].c_str(), xPortocol->sz_world) == 0)
			{
				_Out.push_back(mWorldNameCrc[n]);
				break;
			}
		}

		if(_Out.empty()) xError = EGMForbidAccount_ServerNotExist;
	}

	return xError;
}
//=================================================
//	以下是逻辑处理
//=================================================
DWORD GMSession::HandleServerLogin(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_SeverLogin* xPorotocol = (NET_GMS2S_SeverLogin*)lpMsg;
	Interlocked_Exchange((LPLONG)&mConnected, TRUE);
	return 0;
}
DWORD GMSession::HandleForbidIP(tag_net_message* lpMsg, DWORD)
{

	NET_GMS2S_forbid_ip* xProtocol = (NET_GMS2S_forbid_ip*)lpMsg;
	
	DWORD xReturn = 0;
	if(xProtocol->b_forbid == TRUE)
		xReturn = g_datebase.forbid_ip(xProtocol->dw_ip);
	else
		xReturn = g_datebase.remove_ip_forbid(xProtocol->dw_ip);

	NET_S2GMS_forbid_ip send;
	send.dw_client_id = xProtocol->dw_client_id;
	send.dw_error_code = xReturn;
	mNetSession->send_msg(&send, send.dw_size);

	return 0;
}
DWORD GMSession::HandleForbidAccount(tag_net_message* lpMsg, DWORD)
{

	NET_GMS2S_forbid_account* xProtocol = (NET_GMS2S_forbid_account*)lpMsg;
	DWORD xErrorCode = E_Success;
	std::vector<DWORD> _WorldCRC;


	tstring xAccounts = xProtocol->sz_account;
	if(xAccounts.length() == 0)
	{
		NET_S2GMS_forbid_account send;	
		send.b_forbid = xProtocol->b_forbid;
		send.dw_client_id =	xProtocol->dw_client_id;
		send.dw_error_code = EGMForbidAccount_AccountNotExist;
		_tcscpy(send.sz_account, xProtocol->sz_account);
		mNetSession->send_msg(&send, send.dw_size);
		return 0;
	}

	xErrorCode = GetForbidWorldCrc(xProtocol,_WorldCRC);
	if(xErrorCode != E_Success)
	{

		NET_S2GMS_forbid_account send;	
		send.b_forbid = xProtocol->b_forbid;
		send.dw_client_id =	xProtocol->dw_client_id;
		send.dw_error_code = xErrorCode;
		_tcscpy(send.sz_account, xProtocol->sz_account);
		mNetSession->send_msg(&send, send.dw_size);
		return 0;
	}

	//! 得到要封停账号 
	vector <tstring> xAccountList;
	GetForbidAccount(xAccounts,xAccountList);

	//! 操作都成功
	std::map <tstring,DWORD> xMapRetCode;
	BOOL AllSuccess = TRUE;

	for(size_t iWorldLoop = 0; iWorldLoop<_WorldCRC.size(); iWorldLoop++)
	{
		for(size_t iAccountLoop = 0; iAccountLoop<xAccountList.size(); iAccountLoop++)
		{
			//封停指定账户
			if(xProtocol->b_forbid == TRUE)
			{
				xErrorCode = g_datebase.forbid_account(xAccountList[iAccountLoop].c_str(),eplm_gm_tool,_WorldCRC[iWorldLoop], xProtocol->dwTimeStart, xProtocol->dwTimeEnd);		 	
			}
			else if(xProtocol->b_forbid == FALSE)
			{
				xErrorCode = g_datebase.remove_account_forbid(xAccountList[iAccountLoop].c_str(),eplm_gm_tool,_WorldCRC[iWorldLoop]);
			}

			if(xErrorCode != E_Success)
			{
				AllSuccess = FALSE;
				xMapRetCode.insert(make_pair(xAccountList[iAccountLoop],xErrorCode));
			}
		}
	}

	if(AllSuccess == TRUE)
	{
		NET_S2GMS_forbid_account send;	
		send.b_forbid = xProtocol->b_forbid;
		send.dw_client_id =	xProtocol->dw_client_id;
		send.dw_error_code = E_Success;
		send.dw_world_crc = _WorldCRC.size( ) ? _WorldCRC[0] : 0;
		_tcscpy(send.sz_account, xProtocol->sz_account);
		mNetSession->send_msg(&send, send.dw_size);
	}
	else
	{
		std::map <tstring,DWORD>::iterator iter = xMapRetCode.begin();
		for(; iter != xMapRetCode.end(); ++iter)
		{
			//往回gmserver返结果
			NET_S2GMS_forbid_account send;	
			send.b_forbid = xProtocol->b_forbid;
			send.dw_client_id =	xProtocol->dw_client_id;
			send.dw_error_code = iter->second;
			_tcscpy(send.sz_account,iter->first.c_str());
			mNetSession->send_msg(&send, send.dw_size);
		}
	}

	return 0;
}


DWORD GMSession::HandleSetMaxPlayerNumber(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_SetMaxPlayerNumber* xProtocol = (NET_GMS2S_SetMaxPlayerNumber*)lpMsg;
	si_world* pWorld = g_fpWorldMgr.get_world(xProtocol->dwWorldNameCRC);
	if(VALID_POINT(pWorld)) pWorld->set_max_online_num(xProtocol->nMaxPlayerNumber);
	return 0;
}
