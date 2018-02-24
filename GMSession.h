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
 *	@brief		����GM������
*/

#ifndef __GMSESSION_H__
#define __GMSESSION_H__

enum{ STATUSREPORTINTERVAL = TICK_PER_SECOND };

//=================================================
//	����GM������
//=================================================
class GMSession
{
	tstring mIP;
	DWORD mPort;
	DWORD mSectionID;
	INT mSendStatusTick;
	INT mZoneNumber;
	std::vector<DWORD> mWordID;
	std::vector<DWORD> mWorldNameCrc;
	std::vector<tstring> mWorldName;
	volatile BOOL mTerminateConnect;
	volatile BOOL mConnected;
	few_connect_client* mNetSession;
public:
	GMSession();
	~GMSession();
public:
	BOOL Init();
	VOID Destroy();
	VOID Update();
public:
	inline BOOL IsConnected() const;
private:
	VOID SendServerInfo();
	VOID RegisterCmd();
	VOID UnregisterCmd();
	VOID UpdateSession();
private:
	UINT ThreadConnect();
	static UINT WINAPI ThreadCall(LPVOID lpVoid);
private:
	BOOL ReConnect();
	BOOL CreateConnectThread();
	VOID HandleMessage();
private:
	BOOL SkipBlank(tstring& _Str);
	VOID GetForbidAccount(tstring& _Account,vector<tstring>& _Out);
	DWORD GetForbidWorldCrc(tag_net_message* lpMsg, vector<DWORD>& _Out);
//=================================================
//	�������߼�����
//=================================================
private:
	DWORD	HandleServerLogin(tag_net_message* lpMsg, DWORD);
	DWORD	HandleForbidIP(tag_net_message* lpMsg, DWORD);
	DWORD	HandleForbidAccount(tag_net_message* lpMsg, DWORD);
	DWORD	HandleSetMaxPlayerNumber(tag_net_message* lpMsg, DWORD);

//=================================================
//	�������߼�����
//=================================================
};

inline BOOL GMSession::IsConnected() const { return mConnected; }

extern GMSession g_rtSession;

#endif // __GMSESSION_H__