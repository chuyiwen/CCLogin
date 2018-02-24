/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/

#include "stdafx.h"

#include "../ServerDefine/msg_rt_errorcode.h"
#include "db.h"
#include "player.h"
#include "login_server.h"
#include "../ServerDefine/fatigue_define.h"
#include "../WorldDefine/time.h"
//-------------------------------------------------------------------------------
// ��ʼ������
//-------------------------------------------------------------------------------
BOOL DB::Init()
{
	// �������ļ���ʼ�����ݿ�����
	TObjRef<VarContainer> pVar = "LoginServerConfig";

	// ��ʼ�����ݿ�
	BOOL bRet = m_DB.Init(pVar->GetString(_T("ip database")), 
							pVar->GetString(_T("user database")),
							pVar->GetString(_T("psd database")), 
							pVar->GetString(_T("name database")),
							(INT)pVar->GetDword(_T("port database")));

	//����״̬db����ʱ�䣨���룩
	m_dwWorldStateUpdateTime = pVar->GetDword(_T("update_time world_state"));

	//����״̬log������ʱ�䣨���룩
	m_dwWorldStateInsertTime = pVar->GetDword(_T("insert_time world_state"));

	// ���ñ����ص�����
	m_DB.SetWarningCallBack((WARNINGCALLBACK)DBCallBack);

	return bRet;
}

//-------------------------------------------------------------------------------
// ���ٺ���
//-------------------------------------------------------------------------------
VOID DB::Destroy()
{
}

//--------------------------------------------------------------------------------
// ��ҵ���
//--------------------------------------------------------------------------------
VOID DB::PlayerLogin(DWORD dwAccountID, DWORD dwWorldNameCrc)
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return;

	pStream->SetUpdate("account");
	pStream->FillString("login_status=") << EPLS_Loging;
	pStream->FillString(",worldname_crc=") << dwWorldNameCrc;
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	m_DB.Execute(pStream);

	m_DB.ReturnStream(pStream);
}

//---------------------------------------------------------------------------------
// ��ҵ���
//---------------------------------------------------------------------------------
VOID DB::PlayerLogout(DWORD dwAccountID)
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return;

	pStream->SetUpdate("account");
	pStream->FillString("login_status=") << EPLS_OffLine;
	pStream->FillString(",worldname_crc=") << (DWORD)GT_INVALID;
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;
	pStream->FillString(" and login_status=") << EPLS_Loging;

	m_DB.Execute(pStream);

	m_DB.ReturnStream(pStream);
}

//---------------------------------------------------------------------------------
// ��ҵ�����Ϸ����
//---------------------------------------------------------------------------------
VOID DB::PlayerEnterWorld(DWORD dwAccountID, DWORD dwWorldNameCrc)
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return;

	pStream->SetUpdate("account");
	pStream->FillString("login_status=") << EPLS_Online;
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;
	pStream->FillString(" and worldname_crc=") << dwWorldNameCrc;

	m_DB.Execute(pStream);

	m_DB.ReturnStream(pStream);
}

//---------------------------------------------------------------------------------
// ��ҵǳ���Ϸ����
//---------------------------------------------------------------------------------
VOID DB::PlayerOutWorld(DWORD dwAccountID, DWORD dwWorldNameCrc)
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return;

	pStream->SetUpdate("account");
	pStream->FillString("login_status=") << EPLS_OffLine;
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;
	pStream->FillString(" and worldname_crc=") << dwWorldNameCrc;

	m_DB.Execute(pStream);

	m_DB.ReturnStream(pStream);
}

//---------------------------------------------------------------------------------
// ������ҵĵ����־
//---------------------------------------------------------------------------------
VOID DB::FixPlayerLoginStatus( DWORD dwAccountID, EPlayerLoginStatus eDest )
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return;

	pStream->SetUpdate("account");
	pStream->FillString("login_status=") << eDest;
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	m_DB.Execute(pStream);

	m_DB.ReturnStream(pStream);
}

//---------------------------------------------------------------------------------
// ����ʱ����������ҵĵ����־
//---------------------------------------------------------------------------------
VOID DB::ResetAllPlayerLoginStatus(EPlayerLoginStatus eSrc, EPlayerLoginStatus eDest)
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return;

	// ����½��״̬ȫ���ı���ѵ���״̬�������Ϳ���������Ϊlogin�����������쳣
	pStream->SetUpdate("account");
	pStream->FillString("login_status=") << eDest;
	pStream->SetWhere();
	pStream->FillString("login_status=") << eSrc;

	m_DB.Execute(pStream);

	m_DB.ReturnStream(pStream);
}
//---------------------------------------------------------------------------------
// �ı�ָ����Ϸ������ҵĵ�¼״̬
//---------------------------------------------------------------------------------
VOID DB::FixOneWorldPlayerLoginStatus( DWORD dwWorldNameCrc, EPlayerLoginStatus eSrc, EPlayerLoginStatus eDest )
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return;

	pStream->SetUpdate("account");
	pStream->FillString("login_status=") << eDest;
	pStream->SetWhere();
	pStream->FillString("worldname_crc=")<<dwWorldNameCrc;
	if (eSrc != EPLS_Null)
	{
		pStream->FillString(" and login_status=") << eSrc;
	}

	m_DB.Execute(pStream);

	m_DB.ReturnStream(pStream);
}
//---------------------------------------------------------------------------------
// �õ�ĳ����ҵ��ܱ�
//---------------------------------------------------------------------------------
BOOL DB::GetPlayerMibao(DWORD dwAccountID, CHAR szMibao[MIBAO_LEN])
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return FALSE;

	// ����sql
	pStream->SetSelect("account", "mibao");
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	execute_result* pResult = m_DB.Query(pStream);

	m_DB.ReturnStream(pStream);

	// ��ѯ���Ϊ��
	if( !P_VALID(pResult) || pResult->get_row_count() <= 0) return FALSE;

	BOOL bRet = (*pResult)[0].get_blob(szMibao, MIBAO_LEN);
	m_DB.FreeQueryResult(pResult);
	return bRet;
}
//-------------------------------------------------------------------------
// ��������״̬��¼
//-------------------------------------------------------------------------
BOOL DB::InsertWorldState(DWORD dwWorldID)
{

	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return FALSE;

	//��ѯ�û���,����, ��¼״̬
	Connection* pCon = m_DB.GetFreeConnection();

	char szDate[X_DATATIME_LEN + 1];
	ZeroMemory(szDate, sizeof(szDate));
	DwordTime2DataTime(szDate, X_DATATIME_LEN + 1, GetCurrentDWORDTime());


	pStream->SetInsert("world_state");
	pStream->FillString("worldid=")<<dwWorldID;
	pStream->FillString(",rolenum=")<<0;
	pStream->FillString(",worldstate=")<<-1;
	pStream->FillString(",time='").FillString(szDate, pCon).FillString("'");

	m_DB.ReturnConnection(pCon);
	// ��ѯ
	BOOL bRet = m_DB.Execute(pStream);

	// �ͷ���
	m_DB.ReturnStream(pStream);

	return bRet;
}
//----------------------------------------------------------------------------
// ��������״̬log
//----------------------------------------------------------------------------
BOOL DB::InsertWorldStateLog(DWORD dwWorldID,INT iRoleNum, SHORT sState)
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return FALSE;

	//��ѯ�û���,����, ��¼״̬
	Connection* pCon = m_DB.GetFreeConnection();

	char szDate[X_DATATIME_LEN + 1];
	ZeroMemory(szDate, sizeof(szDate));
	DwordTime2DataTime(szDate, X_DATATIME_LEN + 1, GetCurrentDWORDTime());

	pStream->SetInsert("world_state_log");
	pStream->FillString("worldid=")<<dwWorldID;
	pStream->FillString(",rolenum=")<<iRoleNum;
	pStream->FillString(",worldstate=")<<sState;
	pStream->FillString(",time='").FillString(szDate, pCon).FillString("'");


	m_DB.ReturnConnection(pCon);

	// ��ѯ
	BOOL bRet = m_DB.Execute(pStream);

	// �ͷ���
	m_DB.ReturnStream(pStream);

	return bRet;
}
//----------------------------------------------------------------------------
// �������״̬��
//----------------------------------------------------------------------------
BOOL DB::ClearWorldStateTable()
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return FALSE;

	pStream->Clear();
	pStream->FillString("TRUNCATE TABLE world_state");

	// ��ѯ
	BOOL bRet = m_DB.Execute(pStream);

	// �ͷ���
	m_DB.ReturnStream(pStream);

	return bRet;

}
//----------------------------------------------------------------------------
// ��������״̬
//----------------------------------------------------------------------------
BOOL DB::UpdateWorldState(DWORD dwWorldID,INT iRoleNum, SHORT sState)
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return FALSE;

	//��ѯ�û���,����, ��¼״̬
	Connection* pCon = m_DB.GetFreeConnection();

	char szDate[X_DATATIME_LEN + 1];
	ZeroMemory(szDate, sizeof(szDate));
	DwordTime2DataTime(szDate, X_DATATIME_LEN + 1, GetCurrentDWORDTime());

	pStream->SetUpdate("world_state");
	pStream->FillString("rolenum=")<<iRoleNum;
	pStream->FillString(",worldstate=")<<sState;
	pStream->FillString(",time='").FillString(szDate, pCon).FillString("'");
	pStream->SetWhere();
	pStream->FillString("worldid=")<<dwWorldID;


	m_DB.ReturnConnection(pCon);

	// ��ѯ
	BOOL bRet = m_DB.Execute(pStream);

	// �ͷ���
	m_DB.ReturnStream(pStream);

	return bRet;
}
////-------------------------------------------------------------------------------
//// �����ݿ��в�ѯ�������
////-------------------------------------------------------------------------------
//BOOL DB::QueryPlayerData(Player* pPlayer)
//{
//	if (!P_VALID(pPlayer))
//		return FALSE;
//	tagPlayerData* pData = pPlayer->GetPlayerData();
//	if(!P_VALID(pData))
//		return FALSE;
//
//	// ��ȡ��
//	sql_language_disposal* pStream = m_DB->GetStream();
//	if (!P_VALID(pStream))
//		return FALSE;
//
//	//��ѯ�û���,����, ��¼״̬
//	Connection* pCon = m_DB->GetFreeConnection();
//
//	pStream->SetSelect( "account", "id, password_crc, disabled, privilege,login_status, logout, online_time, guard, guard_accolsec, guard_acculsec, seal_time, seal_code" );
//	pStream->SetWhere();
//	pStream->FillString("name='").FillString(pData->szAccountName, pCon).FillString("'");
//
//	m_DB->ReturnConnection(pCon);
//
//	execute_result* pResult = m_DB->Query(pStream);
//
//	// �ͷ���
//	m_DB->ReturnStream(pStream);
//
//	//��ѯ���Ϊ��
//	if(!P_VALID(pResult))
//		return FALSE;
//	
//	//�����ݿ��ѯ���ݳ�ʼ�� player
//	pData->dwAccountID	= (*pResult)[0].GetDword();
//	pData->dwPasswordCrc = (*pResult)[1].GetDword();
//	pData->bDisabled	= (*pResult)[2].GetBool();
//	pData->byPrivilege	= (*pResult)[3].GetByte();
//	pData->eLoginStatus = (EPlayerLoginStatus)(*pResult)[4].GetInt();
//	pData->dwLastLogoutTime = (*pResult)[5].GetDword();
//	pData->dwOnlineTime = (*pResult)[6].GetDword();
//	pData->bGuard		= (*pResult)[7].GetBool();
//	pData->nAccOLSec	= (*pResult)[8].GetDword();
//	pData->dwAccULSec	= (*pResult)[9].GetDword();
//	pData->dwtSealTime	= (*pResult)[10].GetDword();
//	pData->u16SealCode	= (*pResult)[11].GetShort();
//
//	//�ͷ� Result
//	m_DB->FreeQueryResult( pResult );
//	return TRUE;
//}
//
////-------------------------------------------------------------------------------
//// �������ݿ����������
////-------------------------------------------------------------------------------
//BOOL DB::UpdatePlayerData(Player* pPlayer)
//{
//	if (!P_VALID(pPlayer))
//		return FALSE;
//	tagPlayerData* pData = pPlayer->GetPlayerData();
//	if(!P_VALID(pData))
//		return FALSE;
//
//	sql_language_disposal* pStream = m_DB->GetStream();
//
//	if (!P_VALID(pStream))
//		return FALSE;
//
//	// ��ʽ��SQL���
//	Connection* pCon = m_DB->GetFreeConnection();
//		
//	pStream->SetUpdate("account");
//	pStream->FillString("login_status=") << pData->eLoginStatus;
//	pStream->FillString(",worldname_crc=") << pData->dwWorldNameCrc;
//	pStream->FillString(",ip='").FillString( pData->szIP ).FillString("'");
//	pStream->FillString(",ip='").FillString(pData->szIP, pCon).FillString("',");
//	pStream->FillString("guard_accolsec=") << pData->dwTotalGuardTime;
//	pStream->SetWhere();
//	pStream->FillString("name='").FillString(pData->szAccountName, pCon).FillString("'");
//
//	m_DB->ReturnConnection(pCon);
//
//	// ִ��
//	BOOL bRet = m_DB->Execute(pStream);
//	m_DB->ReturnStream(pStream);
//	return bRet;
//}
//

//-------------------------------------------------------------------------------
// ����account���ε�¼��ʱ���ip
//-------------------------------------------------------------------------------
VOID DB::UpdateAccountLoginInfo( DWORD dwAccountID, DWORD &dwIP)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return;

	//��ʽ��SQL���
	Connection* pCon = m_DB.GetFreeConnection();

	TObjRef<StreamTransport> pTrans;
	pTrans = "StreamTransport";

	char szDate[X_DATATIME_LEN + 1];
	ZeroMemory(szDate, sizeof(szDate));
	DwordTime2DataTime(szDate, X_DATATIME_LEN + 1, GetCurrentDWORDTime());

	//
	pStream->SetUpdate("account");
	pStream->FillString("ip='").FillString(pTrans->IP2String(dwIP), pCon).FillString("',");
	pStream->FillString("time='").FillString(szDate, pCon).FillString("'");
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;


	BOOL bRet = m_DB.Execute( pStream );

	m_DB.ReturnConnection(pCon);	 
	 // �ͷ���
 	m_DB.ReturnStream(pStream);

}

//-------------------------------------------------------------------------------
// ��Log�������ݿ�
//-------------------------------------------------------------------------------
VOID DB::LogPlayerAction( DWORD dwAccountID, LPCSTR szAccountName, DWORD dwIP, LPCSTR szAction )
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return;

	//��ʽ��SQL���
	Connection* pCon = m_DB.GetFreeConnection();

	TObjRef<StreamTransport> pTrans;
	pTrans = "StreamTransport";

	char szDate[X_DATATIME_LEN + 1];
	ZeroMemory(szDate, sizeof(szDate));
	DwordTime2DataTime(szDate, X_DATATIME_LEN + 1, GetCurrentDWORDTime());

	pStream->SetInsert("login_log");
	pStream->FillString("accountID=") << dwAccountID;
	pStream->FillString(",accountName='").FillString(szAccountName, pCon).FillString("'");
	pStream->FillString(",ip='").FillString(pTrans->IP2String(dwIP), pCon).FillString("'");
	pStream->FillString(",action='").FillString(szAction, pCon).FillString("'");
	pStream->FillString(",time='").FillString(szDate, pCon).FillString("'");

	m_DB.ReturnConnection(pCon);
	m_DB.AddQuery(pStream);
}


//-------------------------------------------------------------------------------
// ��KickLog�������ݿ�
//-------------------------------------------------------------------------------
VOID DB::InsertKickLog( const CHAR* pAccountName, DWORD dwAccountID, DWORD dwTime, UINT16 u16ErrCode, BOOL bSeal )
{
	sql_language_disposal* pStream = m_DB.GetStream();
	
	if (!P_VALID(pStream))
		return;

	//��ʽ��SQL���
	Connection* pCon = m_DB.GetFreeConnection();

	if (!P_VALID(pCon))
	{
		return;
	}

	char szDate[X_DATATIME_LEN + 1];
	ZeroMemory(szDate, sizeof(szDate));
	DwordTime2DataTime(szDate, X_DATATIME_LEN + 1, dwTime);

	pStream->SetInsert("game_guarder_log");
	pStream->FillString(" account_id=") << dwAccountID;
	pStream->FillString(", name='").FillString(pAccountName, pCon).FillString("'");
	pStream->FillString(" ,kick_time='").FillString(szDate, pCon).FillString("'");
	pStream->FillString(" ,seal=") << bSeal;
	pStream->FillString(" ,error_code=") << u16ErrCode;

	m_DB.ReturnConnection(pCon);
	m_DB.AddQuery(pStream);
}
//-------------------------------------------------------------------------------
// ��ָ����Ϸ�����Ƿ񱻷�ͣ
//-------------------------------------------------------------------------------
BOOL DB::IfWorldForbid(DWORD dwAccountID,DWORD dwWorldNameCrc)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return FALSE;	

	//���ָ����Ϸ�����Ƿ񱻷���
	pStream->SetSelect("world_forbid","*");
	pStream->SetWhere();
	pStream->FillString("accountid=") << dwAccountID;
	pStream->FillString(" AND worldname_crc=") << dwWorldNameCrc;

	execute_result* pResult = m_DB.Query( pStream );
	//��ѯ���Ϊ��
	if(!P_VALID(pResult) || pResult->get_row_count() <= 0)
	{
		m_DB.ReturnStream(pStream);
		return FALSE;
	}

	m_DB.ReturnStream(pStream);
	return TRUE;
}
//-------------------------------------------------------------------------------
// ��ָ���˺��Ƿ���ڱ���ͣ��Ϸ����
//-------------------------------------------------------------------------------
BOOL DB::IfHaveWorldForbid(DWORD dwAccountID)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return FALSE;	

	//���ָ����Ϸ�����Ƿ񱻷���
	pStream->SetSelect("world_forbid","*");
	pStream->SetWhere();
	pStream->FillString("accountid=") << dwAccountID;

	execute_result* pResult = m_DB.Query( pStream );
	//��ѯ���Ϊ��
	if(!P_VALID(pResult) || pResult->get_row_count() <= 0)
	{
		m_DB.FreeQueryResult( pResult );
		m_DB.ReturnStream(pStream);
		return FALSE;
	}
	m_DB.FreeQueryResult( pResult );
	m_DB.ReturnStream(pStream);
	return TRUE;
}
//-------------------------------------------------------------------------------
// ��ָ��ip�ǲ��Ǳ�����
//-------------------------------------------------------------------------------
BOOL DB::IfIPForbid(DWORD dwIP)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return FALSE;

	TObjRef<StreamTransport> pTrans;
	pTrans = "StreamTransport";

	char szIP[X_IP_LEN];
	strcpy(szIP,pTrans->IP2String(dwIP));

	//��ѯ
	pStream->SetSelect("black_list","*");
	pStream->SetWhere();
	pStream->FillString("ip = '").FillString(szIP).FillString("'");

	execute_result* pResult = m_DB.Query(pStream);
	//��ѯ���Ϊ��
	if(!P_VALID(pResult) || pResult->get_row_count() <= 0)
	{
		m_DB.FreeQueryResult( pResult );
		m_DB.ReturnStream(pStream);
		return FALSE;
	}

	m_DB.FreeQueryResult( pResult );
	m_DB.ReturnStream(pStream);
	return TRUE;
}

//-------------------------------------------------------------------------------
// ���
//-------------------------------------------------------------------------------
DWORD DB::RemoveAccountForbid( LPCTSTR szAccountName, DWORD dwForbidM,DWORD dwWorldNameCrc)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return E_RT_Error;

	//����߼�
	pStream->SetSelect("account","id,forbid_mask");
	pStream->SetWhere();
	pStream->FillString("name='").FillString(szAccountName).FillString("'");

	execute_result* pResult = m_DB.Query( pStream );
	//��ѯ���Ϊ��
	if(!P_VALID(pResult) || pResult->get_row_count() <= 0)
	{
		m_DB.ReturnStream(pStream);
		return E_Login_GMServer_NoAccount;
	}
	//�˺�id
	DWORD dwAccountID = (*pResult)[0].get_dword();
	//Ҫ��֤ԭ���ı�־λΪ1
	if(  ( (*pResult)[1].get_short() & dwForbidM ) != dwForbidM )
	{
		//�����gm
		if(dwForbidM & EPLM_GMTool)
		{
			//�����Ӧ��Ϸ����û�б���ͣ
			if(!IfWorldForbid(dwAccountID,dwWorldNameCrc))
			{
				m_DB.FreeQueryResult( pResult );
				m_DB.ReturnStream(pStream);
				return E_Login_GMServer_Freed;
			}
		}
		else
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_Login_GMServer_Freed;
		}
	}
	//�����GM���߷�� ��Ҫ����world_forbid��
	if(dwForbidM & EPLM_GMTool)
	{
		pStream->SetDelete("world_forbid");
		pStream->SetWhere();
		pStream->FillString("accountid=")<<dwAccountID;
		pStream->FillString(" AND");
		pStream->FillString(" worldname_crc=")<<dwWorldNameCrc;

		BOOL bRet = m_DB.Execute(pStream);
		if(bRet == false)
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_RT_Error;
		}
		//�����gm���߷�ָͣ����Ϸ���� ֻҪ�����˺�������Ϸ���綼����� ������־λ

		//�����ڱ���ͣ����Ϸ����
		if(IfHaveWorldForbid(dwAccountID))
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_RT_Success;
		}
	}
	//���һ��
	pStream->SetUpdate("account");
	pStream->FillString("forbid_mask = ") << dwForbidM;
	pStream->FillString("^forbid_mask");
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	BOOL bRet = m_DB.Execute(pStream);
	if(bRet == FALSE)
	{
		m_DB.FreeQueryResult( pResult );
		m_DB.ReturnStream(pStream);
		return E_RT_Error;
	}
	m_DB.FreeQueryResult( pResult );
	m_DB.ReturnStream(pStream);
	return E_RT_Success;
}
DWORD DB::RemoveAccountForbid( DWORD dwAccountID, DWORD dwForbidM,DWORD dwWorldNameCrc)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return E_RT_Error;

	//����߼�
	pStream->SetSelect("account","forbid_mask");
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	execute_result* pResult = m_DB.Query( pStream );
	//��ѯ���Ϊ��
	if(!P_VALID(pResult) || pResult->get_row_count() <= 0)
	{
		m_DB.ReturnStream(pStream);
		return E_Login_GMServer_NoAccount;
	}
	//Ҫ��֤ԭ���ı�־λΪ1
	if(  ( (*pResult)[0].get_short() & dwForbidM ) != dwForbidM )
	{
		//�����gm
		if(dwForbidM & EPLM_GMTool)
		{
			//�����Ӧ��Ϸ����û�б���ͣ
			if(!IfWorldForbid(dwAccountID,dwWorldNameCrc))
			{
				m_DB.FreeQueryResult( pResult );
				m_DB.ReturnStream(pStream);
				return E_Login_GMServer_Freed;
			}
		}
		else
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_Login_GMServer_Freed;
		}
	}
	//�����GM���߷�� ��Ҫ����world_forbid��
	if(dwForbidM & EPLM_GMTool)
	{
		pStream->SetDelete("world_forbid");
		pStream->SetWhere();
		pStream->FillString("accountid=")<<dwAccountID;
		pStream->FillString(" AND");
		pStream->FillString(" worldname_crc=")<<dwWorldNameCrc;

		BOOL bRet = m_DB.Execute(pStream);
		if(bRet == false)
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_RT_Error;
		}
		//�����ڱ���ͣ����Ϸ����
		if(IfHaveWorldForbid(dwAccountID))
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_RT_Success;
		}
	}
	//���һ��
	pStream->SetUpdate("account");
	pStream->FillString("forbid_mask = ") << dwForbidM;
	pStream->FillString("^forbid_mask");
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	BOOL bRet = m_DB.Execute(pStream);
	if(bRet == FALSE)
	{
		m_DB.FreeQueryResult( pResult );
		m_DB.ReturnStream(pStream);
		return E_RT_Error;
	}



	m_DB.FreeQueryResult( pResult );
	m_DB.ReturnStream(pStream);
	return E_RT_Success;
}

DWORD DB::SetAccountChenMi( DWORD dwAccountID, DWORD dwChenMi,DWORD dwWorldNameCrc)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return E_RT_Error;

	pStream->SetUpdate("account");
	pStream->FillString("guard=") << (BYTE)dwChenMi;
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	BOOL bRtv = m_DB.Execute(pStream);
	m_DB.ReturnStream(pStream);

	return bRtv ? E_RT_Success : E_RT_Error;
}

//-------------------------------------------------------------------------------
// ���
//-------------------------------------------------------------------------------
DWORD DB::ForbidAccount( LPCTSTR szAccountName, DWORD dwForbidM,DWORD dwWorldNameCrc)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return E_RT_Error;

	//����߼�
	pStream->SetSelect("account","id,forbid_mask");
	pStream->SetWhere();
	pStream->FillString("name='").FillString(szAccountName).FillString("'");

	execute_result* pResult = m_DB.Query( pStream );
	//��ѯ���Ϊ��
	if(!P_VALID(pResult) || pResult->get_row_count() <= 0)
	{
		m_DB.ReturnStream(pStream);
		return E_Login_GMServer_NoAccount;
	}
	//�˺�id
	DWORD dwAccountID = (*pResult)[0].get_dword();
	SHORT sForbidMask = (*pResult)[1].get_short();

	//Ҫ��֤ԭ���Ķ�Ӧ��־λΪ0
	if(  ( sForbidMask & dwForbidM ) != 0 )
	{
		//�����gm
		if(dwForbidM & EPLM_GMTool)
		{
			//�����Ӧ��Ϸ�����Ѿ�����ͣ
			if(IfWorldForbid(dwAccountID,dwWorldNameCrc))
			{
				m_DB.FreeQueryResult( pResult );
				m_DB.ReturnStream(pStream);
				return E_Login_GMServer_Forbidded;
			}
		}
		else
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_Login_GMServer_Forbidded;
		}
	}


	//�����GM���߷�� ��Ҫ����world_forbid��
	if(dwForbidM & EPLM_GMTool)
	{
		pStream->SetInsert("world_forbid");
		pStream->FillString("accountid=")<<dwAccountID;
		pStream->FillString(",worldname_crc=")<<dwWorldNameCrc;

		BOOL bRet = m_DB.Execute(pStream);
		if(bRet == FALSE)
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_RT_Error;
		}
		//���֮ǰ��־λ�Ѿ���������
		if(sForbidMask & EPLM_GMTool)
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_RT_Success;
		}
	}
	//���һ��
	pStream->SetUpdate("account");
	pStream->FillString("forbid_mask = ") << dwForbidM;
	pStream->FillString("^forbid_mask");
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	BOOL bRet = m_DB.Execute(pStream);
	if(bRet == FALSE)
	{
		m_DB.FreeQueryResult( pResult );
		m_DB.ReturnStream(pStream);
		return E_RT_Error;
	}


	m_DB.FreeQueryResult( pResult );
	m_DB.ReturnStream(pStream);
	return E_RT_Success;
}

DWORD DB::ForbidAccount( DWORD dwAccountID, DWORD dwForbidM,DWORD dwWorldNameCrc)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return E_RT_Error;

	//����߼�
	pStream->SetSelect("account","forbid_mask");
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	execute_result* pResult = m_DB.Query( pStream );
	//��ѯ���Ϊ��
	if(!P_VALID(pResult) || pResult->get_row_count() <= 0)
	{
		m_DB.ReturnStream(pStream);
		return E_Login_GMServer_NoAccount;
	}
	SHORT sForbidMask = (*pResult)[0].get_short();
	//Ҫ��֤ԭ���Ķ�Ӧ��־λΪ0
	if(  ( sForbidMask & dwForbidM ) != 0 )
	{
		//�����gm
		if(dwForbidM & EPLM_GMTool)
		{
			//�����Ӧ��Ϸ�����Ѿ�����ͣ
			if(IfWorldForbid(dwAccountID,dwWorldNameCrc))
			{
				m_DB.FreeQueryResult( pResult );
				m_DB.ReturnStream(pStream);
				return E_Login_GMServer_Forbidded;
			}
		}
		else
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_Login_GMServer_Forbidded;
		}
	}
	//�����GM���߷�� ��Ҫ����world_forbid��
	if(dwForbidM & EPLM_GMTool)
	{
		pStream->SetInsert("world_forbid");
		pStream->FillString("accountid=")<<dwAccountID;
		pStream->FillString(",worldname_crc=")<<dwWorldNameCrc;

		BOOL bRet = m_DB.Execute(pStream);
		if(bRet == FALSE)
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_RT_Error;
		}
		//���֮ǰ��־λ�Ѿ���������
		if(sForbidMask & EPLM_GMTool)
		{
			m_DB.FreeQueryResult( pResult );
			m_DB.ReturnStream(pStream);
			return E_RT_Success;
		}
	}
	//���һ��
	pStream->SetUpdate("account");
	pStream->FillString("forbid_mask = ") << dwForbidM;
	pStream->FillString("^forbid_mask");
	pStream->SetWhere();
	pStream->FillString("id=") << dwAccountID;

	BOOL bRet = m_DB.Execute(pStream);
	if(bRet == FALSE)
	{
		m_DB.FreeQueryResult( pResult );
		m_DB.ReturnStream(pStream);
		return E_RT_Error;
	}
	m_DB.FreeQueryResult( pResult );
	m_DB.ReturnStream(pStream);
	return E_RT_Success;
}
//-------------------------------------------------------------------------------
// ��IP
//-------------------------------------------------------------------------------
DWORD DB::ForbidIP(DWORD dwIP)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return E_RT_Error;

	TObjRef<StreamTransport> pTrans;
	pTrans = "StreamTransport";

	char szIP[X_IP_LEN];
	strcpy(szIP,pTrans->IP2String(dwIP));

	//�����
	pStream->SetInsert("black_list");
	pStream->FillString("ip = '").FillString(szIP).FillString("'");

	BOOL bRet = m_DB.Execute(pStream);
	if(bRet == FALSE)
	{
		m_DB.ReturnStream(pStream);
		return E_RT_Error;
	}

	return E_RT_Success;
}
//-------------------------------------------------------------------------------
// ���IP
//-------------------------------------------------------------------------------
DWORD DB::RemoveIPForbid(DWORD dwIP)
{
	sql_language_disposal* pStream = m_DB.GetStream();

	if (!P_VALID(pStream))
		return E_RT_Error;

	TObjRef<StreamTransport> pTrans;
	pTrans = "StreamTransport";

	char szIP[X_IP_LEN];
	strcpy(szIP,pTrans->IP2String(dwIP));

	//��ɾ��
	pStream->SetDelete("black_list");
	pStream->SetWhere();
	pStream->FillString("ip = '").FillString(szIP).FillString("'");

	BOOL bRet = m_DB.Execute(pStream);
	if(bRet == FALSE)
	{
		m_DB.ReturnStream(pStream);
		return E_RT_Error;
	}
	return E_RT_Success;
}

VOID DB::ResetPlayersLoginStatus( DWORD* pdwAccountIDs, INT nNum, EPlayerLoginStatus eDest )
{
	for (INT i=0; i<nNum; ++i)
	{
		FixPlayerLoginStatus(pdwAccountIDs[i], eDest);
	}
}

BOOL DB::LoadAllFatigueInfo( TMap<DWORD, tagFatigueInfo*> &mapFatigueInfo )
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return FALSE;

	// ����sql
	pStream->SetSelect("fatigue_time", 
						"accountname_crc,"			//0
						"acc_online_time,"		//1
						"acc_offline_time"	//2
						);

	execute_result* pResult = m_DB.Query(pStream);
	m_DB.ReturnStream(pStream);

	// ��ѯ���Ϊ��
	if( !P_VALID(pResult) || pResult->get_row_count() <= 0) return FALSE;

	tagFatigueInfo* tmp = NULL;
	INT nCount = pResult->get_row_count();
	for (INT i=0; i<nCount; ++i)
	{
		DWORD dwAccountNameCrc = (*pResult)[0].get_dword();
		tmp = mapFatigueInfo.Peek(dwAccountNameCrc);
		if (!P_VALID(tmp))
		{
			tmp = new tagFatigueInfo;
			mapFatigueInfo.Add(dwAccountNameCrc, tmp);
		}

		tmp->dwAccountNameCrc = dwAccountNameCrc;
		tmp->nAccOnLineTimeMin = (*pResult)[1].get_int();
		tmp->nAccOffLineTimeMin = (*pResult)[2].get_int();

		pResult->next_row();
	}

	m_DB.FreeQueryResult(pResult);
	return TRUE;
}

// VOID DB::SaveFatigueInfo( tagFatigueInfo* pToSave )
// {
// 	sql_language_disposal* pStream = m_DB.GetStream();
// 	if( !P_VALID(pStream) ) return ;
// 
// 	pStream->SetUpdate("fatigue_time");
// 	pStream->FillString("acc_online_time=")<<pToSave->nAccOnLineTimeMin;
// 	pStream->FillString(",acc_offline_time=")<<pToSave->nAccOffLineTimeMin;
// 	pStream->SetWhere();
// 	pStream->FillString("account_id=")<<pToSave->dwAccountID;
// 
// 
// 	m_DB.Execute(pStream);
// 
// 	m_DB.ReturnStream(pStream);
// }

// BOOL DB::LoadLoginStatus( TMap<DWORD, tagFatigueInfo*> &mapFatigueInfo )
// {
// 	sql_language_disposal* pStream = m_DB.GetStream();
// 	if( !P_VALID(pStream) ) return FALSE;
// 
// 	// ����sql
// 	pStream->SetSelect("account", 
// 		"id, "			//0
// 		"login_status, "		//1
// 		"worldname_crc, "		//2
// 		"guard");				//3
// 
// 	execute_result* pResult = m_DB.Query(pStream);
// 	m_DB.ReturnStream(pStream);
// 
// 	// ��ѯ���Ϊ��
// 	if( !P_VALID(pResult) || pResult->GetRowCount() <= 0) return FALSE;
// 
// 	tagFatigueInfo* tmp = NULL;
// 	INT nCount = pResult->GetRowCount();
// 	for (INT i=0; i<nCount; ++i)
// 	{
// 		DWORD dwAccountID = (*pResult)[0].GetDword();
// 		tmp = mapFatigueInfo.Peek(dwAccountID);
// 		if (!P_VALID(tmp))
// 		{
// 			tmp = new tagFatigueInfo;
// 			mapFatigueInfo.Add(dwAccountID, tmp);
// 			tmp->dwAccountID = dwAccountID;
// 		}
// 		tmp->ePls = EPlayerLoginStatus((*pResult)[1].GetInt());
// 		tmp->dwWorldNameCrc = (*pResult)[2].GetDword();
// 		tmp->bGuard = (*pResult)[3].GetBool();
// 		pResult->NextRow();
// 	}
// 
// 	m_DB.FreeQueryResult(pResult);
// 	return TRUE;
// }

// VOID DB::UpdateFatigueTimeTable()
// {
// 	sql_language_disposal* pStream = m_DB.GetStream();
// 	if( !P_VALID(pStream) ) return ;
// 
// 	pStream->FillString("INSERT INTO fatigue_time(account_id)  (SELECT id FROM account ) on DUPLICATE key update account_id=account_id");
// 
// 	m_DB.Execute(pStream);
// 
// 	m_DB.ReturnStream(pStream);
// }

BOOL DB::LoadOnLineGuardAccountIDs( TMap<DWORD, DWORD> &mapAccounts )
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return FALSE;

	// ����sql
	pStream->SetSelect("account", "id, worldname_crc");
	pStream->SetWhere();
	pStream->FillString("login_status=1 and guard=1");

	execute_result* pResult = m_DB.Query(pStream);
	m_DB.ReturnStream(pStream);

	// ��ѯ���Ϊ��
	if( !P_VALID(pResult) || pResult->get_row_count() <= 0) return FALSE;

	INT nCount = pResult->get_row_count();

	mapAccounts.Clear();
	for (INT i=0; i<nCount; ++i)
	{
		DWORD dwAccountID = (*pResult)[0].get_dword();
		DWORD dwWorldNameCrc = (*pResult)[1].get_dword();
		
		if (!GT_VALID(mapAccounts.Peek(dwAccountID)))
		{
			mapAccounts.Add(dwAccountID, dwWorldNameCrc);
		}

		pResult->next_row();
	}

	m_DB.FreeQueryResult(pResult);
	return TRUE;
}

BOOL DB::LoadCacheAccountData( TSafeMap<DWORD, tagAccountData*> &mapAccountData, TSafeMap<DWORD, DWORD> &mapNameCrc2AccountID)
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return FALSE;

	TObjRef<Util> pUtil;
	pUtil = "Util";
	TObjRef<StreamTransport> pTrans;
	pTrans = "StreamTransport";

	// ��������
	Connection* pCon = m_DB.GetFreeConnection();

	pStream->SetSelect("account", "id, name,guard,ip");

	// �ͷ�����
	m_DB.ReturnConnection(pCon);

	execute_result* pResult = m_DB.Query(pStream);

	// �ͷ���
	m_DB.ReturnStream(pStream);

	// ��ѯ���Ϊ��
	if( !P_VALID(pResult) || pResult->get_row_count() <= 0) return FALSE;

	DWORD dwAccountID = GT_INVALID;
	DWORD dwAccountNameCrc = GT_INVALID;
	tagAccountData* pAccountData = NULL;

	mapAccountData.Clear();
	mapNameCrc2AccountID.Clear();

	BOOL bRtv = TRUE;

	for (INT i=0; i<pResult->get_row_count(); ++i)
	{
		// ���ý��
		pAccountData = new tagAccountData;
		if (!P_VALID(pAccountData))
		{
			bRtv = FALSE;
			break;
		}

		dwAccountID = (*pResult)[0].get_dword();

		memcpy(pAccountData->szAccountName, (*pResult)[1].get_string(), sizeof(CHAR)*16);
		pAccountData->bGuard			=	(*pResult)[2].get_bool();

		//�ϴε�¼��ip
		char szIP[X_IP_LEN] = "";
		memcpy(szIP,(*pResult)[3].get_string(),(*pResult)[3].get_length());
		pAccountData->dwIp = pTrans->StringIP2IP(szIP);

		dwAccountNameCrc = pUtil->Crc32(pAccountData->szAccountName);

		if (!GT_VALID(mapAccountData.Peek(dwAccountID)))
		{
			mapAccountData.Add(dwAccountID, pAccountData);
		}
		if (!GT_VALID(mapNameCrc2AccountID.Peek(dwAccountNameCrc)))
		{
			mapNameCrc2AccountID.Add(dwAccountNameCrc, dwAccountID);
		}

		pResult->next_row();
	}

	// �黹�����
	m_DB.FreeQueryResult(pResult);

	return bRtv;

}

VOID DB::SaveFatigueInfo( tagFatigueInfo* pFatigueInfo )
{
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return ;

	pStream->SetReplace("fatigue_time");
	pStream->FillString("acc_online_time=")<<pFatigueInfo->nAccOnLineTimeMin;
	pStream->FillString(",acc_offline_time=")<<pFatigueInfo->nAccOffLineTimeMin;
	pStream->FillString(",accountname_crc=")<<pFatigueInfo->dwAccountNameCrc;

	m_DB.Execute(pStream);

	m_DB.ReturnStream(pStream);
}

BOOL DB::LoadFatigueInfo(DWORD dwAccountNameCrc, tagFatigueInfo* pFatigueInfo )
{
	if (!P_VALID(pFatigueInfo)) return FALSE;
	sql_language_disposal* pStream = m_DB.GetStream();
	if( !P_VALID(pStream) ) return FALSE;

	// ����sql
	pStream->SetSelect("fatigue_time", "acc_online_time, acc_offline_time");
	pStream->SetWhere();
	pStream->FillString("accountname_crc=") << dwAccountNameCrc;

	execute_result* pResult = m_DB.Query(pStream);
	m_DB.ReturnStream(pStream);

	// ��ѯ���Ϊ��
	if( !P_VALID(pResult) || pResult->get_row_count() != 1) return FALSE;

	pFatigueInfo->dwAccountNameCrc = dwAccountNameCrc;
	pFatigueInfo->nAccOnLineTimeMin = (*pResult)[0].get_int();
	pFatigueInfo->nAccOffLineTimeMin = (*pResult)[1].get_int();

	m_DB.FreeQueryResult(pResult);
	return TRUE;	
}
//
////-------------------------------------------------------------------------------
//// ����ҳɹ���¼��Ϸ���磬�������ݿ��������
////-------------------------------------------------------------------------------
//BOOL DB::UpdatePlayerIntoWorld(EPlayerLoginStatus status, DWORD dwLoginTime, DWORD dwAccULSec, DWORD dwAccOLSec, DWORD dwAccountNameCrc)
//{
//	sql_language_disposal* pStream = m_DB->GetStream();
//
//	if (!P_VALID(pStream))
//		return FALSE;
//
//	//��ʽ��SQL���
//	pStream->SetUpdate("account");
//	pStream->FillString("login_status=") << status;
//	pStream->FillString(",login=") << dwLoginTime;
//	pStream->FillString(",guard_acculsec=") << dwAccULSec;
//	pStream->FillString(",guard_accolsec=") << dwAccOLSec;
//	pStream->SetWhere();
//	pStream->FillString("id='") <<dwAccountNameCrc ;
//	pStream->FillString( "'" );
//	// ִ��
//	BOOL bRet = m_DB->Execute(pStream);
//	m_DB->ReturnStream(pStream);
//	return bRet;
//}
////-------------------------------------------------------------------------------
//// ������뿪��Ϸ���磬�������ݿ��������
////-------------------------------------------------------------------------------
//BOOL DB::UpdatePlayerOutWorld(EPlayerLoginStatus status, DWORD dwLogoutTime, DWORD dwOnlineTime, DWORD dwAccOLSec, DWORD dwAccountNameCrc)
//{
//	sql_language_disposal* pStream = m_DB->GetStream();
//
//	if (!P_VALID(pStream))
//		return FALSE;
//
//	//��ʽ��SQL���
//	pStream->SetUpdate("account");
//	pStream->FillString("login_status=") << status;
//	pStream->FillString(",logout=") << dwLogoutTime;
//	pStream->FillString(",online_time=") << dwOnlineTime;
//	pStream->FillString(",guard_accolsec=") << dwAccOLSec;
//	pStream->SetWhere();
//	pStream->FillString("id='")<<dwAccountNameCrc ;
//	pStream->FillString("'");
//
//	// ִ��
//	BOOL bRet = m_DB->Execute(pStream);
//	m_DB->ReturnStream(pStream);
//	return bRet;
//}
//
////-------------------------------------------------------------------------------
//// ��fpworld����,��ո÷������ϵ�¼�����
////-------------------------------------------------------------------------------
//VOID DB::UpdatePlayer_FpWorld(EPlayerLoginStatus status, DWORD dwWorldNameCrc, DWORD dwLogoutTime)
//{
//	sql_language_disposal* pStream = m_DB->GetStream();
//
//	if (!P_VALID(pStream))
//		return;
//
//	//��ʽ��SQL���
//	pStream->SetUpdate("account");
//	pStream->FillString("login_status=") << status;
//	pStream->FillString(",logout=") << dwLogoutTime;
//	pStream->SetWhere();
//	pStream->FillString("worldname_crc='")<<dwWorldNameCrc ;
//	pStream->FillString( "'" );
//
//	// ִ��
//	m_DB->Execute(pStream);
//	m_DB->ReturnStream(pStream);
//}
//
//
//-------------------------------------------------------------------------------
// �����ݿ��в�ѯ����ʺ�����
//-------------------------------------------------------------------------------
// tstring DB::GetAccountName(DWORD dwAccountNameCrc)
// {
// 	tstring name;
// 	// ��ȡ��
// 	sql_language_disposal* pStream = m_DB.GetStream();
// 	if (!P_VALID(pStream))
// 		return name;
// 
// 	//��ѯ�û���,����, ��¼״̬
// 	pStream->SetSelect( "account", "name" );
// 	pStream->SetWhere().FillString("id =") << dwAccountNameCrc;
// 
// 	execute_result* pResult = m_DB.Query( pStream );
// 
// 	// �ͷ���
// 	m_DB.ReturnStream(pStream);
// 
// 	//��ѯ���Ϊ��
// 	if(!P_VALID(pResult) || pResult->GetRowCount() <= 0)
// 		return name;
// 
// 	//�����ݿ��ѯ���ݳ�ʼ�� player
// 	name = (*pResult)[0].GetUnicodeString();
// 
// 	//�ͷ� Result
// 	m_DB.FreeQueryResult( pResult );
// 	return name;
// }

//------------------------------------------------------------------------------------
// ���ݿ�Ԥ���ص�����
//------------------------------------------------------------------------------------
VOID DBCallBack(DataBase* pDB, INT nReason, INT nParam)
{
	if (nReason == DataBase::EDBE_System)
	{
		ILOG->Write(_T("Beton Warning CallBack: Reason  %s , nParam = %u\r\n"), _T("EDBE_System"), nParam);
	}
	else if(nReason == DataBase::EDBE_QueueFull)
	{
		ILOG->Write(_T("Beton Warning CallBack: Reason  %s , nParam = %u\r\n"), _T("EDBE_QueueFull"), nParam);
	}
	else if(nReason == DataBase::EDBE_PoolFull)
	{
		ILOG->Write(_T("Beton Warning CallBack: Reason  %s , nParam = %u\r\n"), _T("EDBE_PoolFull"), nParam);
	}
	else
	{
		ILOG->Write(_T("Beton Warning CallBack: Reason Unknow,nReason = %u, nParam = %u\r\n"), nReason, nParam);
	}
}