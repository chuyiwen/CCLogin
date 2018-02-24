/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "stdafx.h"
#include "si_login_server.h"


//-----------------------------------------------------------------------------
INT APIENTRY _tWinMain(HINSTANCE hInst, HINSTANCE, LPTSTR, INT)
{
	serverbase::init_network();
	serverbase::init_serverbase();

	//modify mmz at 2010.9.17 release“≤º«dump
//#ifdef DEBUG
	_set_se_translator( serverdump::si_translation ); 

	try
	{
//#endif
		if (g_login.init(hInst) == TRUE)
		{
			g_login.main_loop();
		}
		g_login.destroy();

//#ifdef DEBUG
	}
	catch( serverdump::throw_exception )
	{
		throw;
	}
//#endif

	serverbase::destroy_serverbase();
	serverbase::destroy_network();

	return 0;
}