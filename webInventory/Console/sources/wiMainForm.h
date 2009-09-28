/*
    webInventory is the web-application audit programm
    Copyright (C) 2009 Andrew Abramov stinger911@gmail.com

    This file is part of webInventory

    webInventory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webInventory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webInventory.  If not, see <http://www.gnu.org/licenses/>.
*/
/***************************************************************
 * @file      wiMainForm.h
 * @brief     Declaration of the MainForm class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      30.06.2009
 **************************************************************/
#ifndef __wiMainForm__
#define __wiMainForm__

#include "wiGuiData.h"
#include "wiStatBar.h"
#include "Config.h"
#include "wiApp.h"
#include "wiTasks.h"
#include "wiReports.h"
#include "wiSettings.h"
#include "wiTcpClient.h"
#ifdef SERVICE_EDITION
  #include "wiService.h"
#endif

#define wxPluginsData   10000

/** Implementing MainForm */
class wiMainForm : public MainForm
{
protected:
    wiTasks* m_pTasks;
    wxPanel* m_pReports;
#ifdef SERVICE_EDITION
    wxPanel* m_pService;
#endif
    wiSettings* m_pSettings;
    wiStatBar* m_statusBar;

    // Handlers for MainForm events.
	void OnClose( wxCloseEvent& event );
    void OnTimer( wxTimerEvent& event );

    // functions

	// members
	CConfigEngine m_cfgEngine;
	wiTcpClient*  m_client;
	PluginList* m_plugList;
	wxString m_connectionName;
    wxTimer m_timer;
    bool m_connectionStatus;

public:
	/** Constructor */
	wiMainForm( wxWindow* parent );

	CConfigEngine* Config() { return &m_cfgEngine; };

	void DoConnect(int account);
	void DoDisconnect();
	void LoadPluginList();
	void ShowConnectionError();
	PluginList* GetPluginList();

    ObjectList* GetObjectList(const wxString& criteria = wxT(""));
    ProfileList* GetProfileList(const wxString& criteria = wxT(""));
    TaskList* GetTaskList(const wxString& criteria = wxT(""));
    wxString UpdateObject(ObjectInf& objInfo);

	wxString DoClientCommand(const wxString& cmd, const wxString& params);
	char** StringListToXpm(vector<string>& data);
	wxString FromStdString(const std::string& str);
	bool IsConnected() { return m_client != NULL; };

    void Connected(bool forced = true);
    void Disconnected(bool forced = true);
};

#define FRAME_WINDOW ((wiMainForm*)(wxGetApp().GetTopWindow()))

#endif // __wiMainForm__
