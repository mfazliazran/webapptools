/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "weHelper.h"
#include "weTask.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread.hpp>

void WeTaskProcessor(WeTask* tsk)
{
    WeOption opt;
    int i;
    WeTask::WeRequestMap::iterator tsk_it;
    WeTask::WeRequestMap::iterator tsk_go;
    WeResponseList::iterator rIt;
    iweResponse* resp;
    iweRequest* curr_url;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTaskProcessor started for task " << ((void*)&tsk));
    while (tsk->IsReady())
    {
        tsk->WaitForData();
        opt = tsk->Option(weoParallelReq);
        SAFE_GET_OPTION_VAL(opt, tsk->taskQueueSize, 1);

        // send request if slots available
        while (tsk->taskQueueSize > tsk->taskQueue.size()) {
            for (tsk_go = tsk->taskList.begin(); tsk_go != tsk->taskList.end(); tsk_go++)
            {
                if (tsk_go->second != NULL) {
                    break;
                }
            }
            if (tsk_go != tsk->taskList.end())
            {
                curr_url = tsk_go->second;
                // search for transport or recreate request to all transports
                if (curr_url->RequestUrl().IsValid()) {
                    // search for transport
                    for(i = 0; i < tsk->transports.size(); i++)
                    {
                        if (tsk->transports[i]->IsOwnProtocol(curr_url->RequestUrl().protocol)) {
                            resp = tsk->transports[i]->Request(curr_url);
                            resp->ID(curr_url->ID());
                            resp->processor = curr_url->processor;
                            resp->context = curr_url->context;
                            tsk->taskQueue.push_back(resp);
                            LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeTaskProcessor: send request to " << curr_url->RequestUrl().ToString());
                            break;
                        }
                    }
                }
                else {
                    // try to send request though appropriate transports
                    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeTaskProcessor: send request to " << curr_url->RequestUrl().ToString());
                }

                string u_req = curr_url->RequestUrl().ToString();
                tsk->taskList[u_req] = NULL;
                delete curr_url;
            }
            else {
                break; // no URLs in the waiting list
            }
        }

        // if any requests pending process transport operations
        if (tsk->taskQueue.size() > 0) {
            LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTaskProcessor: transport->ProcessRequests()");
            for(i = 0; i < tsk->transports.size(); i++)
            {
                tsk->transports[i]->ProcessRequests();
            }
            boost::this_thread::sleep(boost::posix_time::millisec(500));
            for(rIt = tsk->taskQueue.begin(); rIt != tsk->taskQueue.end();) {
                if ((*rIt)->Processed()) {
                    if ((*rIt)->processor) {
                        (*rIt)->processor((*rIt), (*rIt)->context);
                    }
                    tsk->taskQueue.erase(rIt);
                    rIt = tsk->taskQueue.begin();
                }
                else {
                    rIt++;
                }
            }
        }
    };
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTaskProcessor finished for task " << ((void*)&tsk));
}

WeTask::WeTask()
{
    FUNCTION;
    // set the default options
    processThread = true;
    mutex_ptr = (void*)(new boost::mutex);
    event_ptr = (void*)(new boost::condition_variable);
    taskList.clear();
    taskQueue.clear();
    processThread = true;
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask created");
    boost::thread process(WeTaskProcessor, this);
}

WeTask::WeTask( WeTask& cpy )
{
    options = cpy.options;
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask assigned");
}

WeTask::~WeTask()
{
    /// @todo Cleanup
}

iweResponse* WeTask::GetRequest( iweRequest* req )
{
    /// @todo Implement this!
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::GetRequest (WeURL)");
    LOG4CXX_ERROR(WeLogger::GetLogger(), " *** Not implemented yet! ***");
    return NULL;
}

void WeTask::GetRequestAsync( iweRequest* req )
{
    /// @todo Implement this!
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::GetRequestAsync");
    // wake-up task_processor
    if (event_ptr != NULL && mutex_ptr != NULL) {
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        boost::unique_lock<boost::mutex> lock(*mt);
        taskList[req->RequestUrl().ToString()] = req;
        LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::GetRequestAsync: new task size=" << taskList.size());
        cond->notify_all();
    }
    return;
}

bool WeTask::IsReady()
{
    return (transports.size() > 0 && processThread);
}

void WeTask::AddTransport(const  string& transp )
{
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::AddTransport (const string&)");
    //iweTransport* tr = WeCreateNamedTransport(transp, NULL);
    //AddTransport(tr);
}

void WeTask::AddTransport( iweTransport* transp )
{
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::AddTransport (iweTransport)");
    for (int i = 0; i < transports.size(); i++)
    {
        if (transports[i]->GetID() == transp->GetID())
        {
            // transport already in list
            LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeTask::AddTransport - transport already in list");
            return;
        }
    }
    transports.push_back(transp);
    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::AddTransport: added " << transp->GetDesc());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn std::string WeTask::ToXml( void )
///
/// @brief  Converts this object to an XML.
///
/// This function realizes alternate serialization mechanism. It generates more compact and
/// simplified XML representation. This representation is used for internal data exchange
/// (for example to store data through iweStorage interface).
///
/// @retval This object as a std::string.
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string WeTask::ToXml( void )
{
    string retval;
    string optList;
    int optCount;
    int optType;
    WeOption optVal;
    string strData;
    int    intData;
    unsigned int uintData;
    char   chData;
    unsigned char uchData;
    long    longData;
    unsigned long ulongData;
    bool    boolData;
    double  doubleData;
    WeOptions::iterator it;

    retval = "";

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::ToXml");
    optVal = Option(weoID);
    SAFE_GET_OPTION_VAL(optVal, strData, "");
    retval += "<task id='" + WeScreenXML(strData) + "'>\n";

    optVal = Option(weoName);
    SAFE_GET_OPTION_VAL(optVal, strData, "");
    retval += "  <name>" + WeScreenXML(strData) + "</name>\n";

    optVal = Option(weoTaskStatus);
    SAFE_GET_OPTION_VAL(optVal, intData, 0);
    retval += "  <status>" + boost::lexical_cast<string>(intData) + "</status>\n";

    optVal = Option(weoTaskCompletion);
    SAFE_GET_OPTION_VAL(optVal, intData, 0);
    retval += "  <completion>" + boost::lexical_cast<string>(intData) + "</completion>\n";

    optCount = 0;
    optList = "";
    for (it = options.begin(); it != options.end(); it++) {
        strData = it->first;
        // skip predefined options
        if (strData == weoID) {
            continue;
        }
        if (strData == weoName) {
            continue;
        }
        if (strData == weoTaskStatus) {
            continue;
        }
        if (strData == weoTaskCompletion) {
            continue;
        }
        optType = it->second->Which();
        try
        {
            switch(optType)
            {
            case 0:
                it->second->GetValue(chData);
                strData = boost::lexical_cast<string>(chData);
                break;
            case 1:
                it->second->GetValue(uchData);
                strData = boost::lexical_cast<string>(uchData);
                break;
            case 2:
                it->second->GetValue(intData);
                strData = boost::lexical_cast<string>(intData);
                break;
            case 3:
                it->second->GetValue(uintData);
                strData = boost::lexical_cast<string>(uintData);
                break;
            case 4:
                it->second->GetValue(longData);
                strData = boost::lexical_cast<string>(longData);
                break;
            case 5:
                it->second->GetValue(ulongData);
                strData = boost::lexical_cast<string>(ulongData);
                break;
            case 6:
                it->second->GetValue(boolData);
                strData = boost::lexical_cast<string>(boolData);
                break;
            case 7:
                it->second->GetValue(doubleData);
                strData = boost::lexical_cast<string>(doubleData);
                break;
            case 8:
                it->second->GetValue(strData);
                break;
            default:
                //optVal = *(it->second);
                strData = "";
            }
        }
        catch (...)
        {
        	strData = "";
        }
        optCount++;
        strData = WeScreenXML(strData);
        optList += "    <option name='" + it->first + "' type='" + boost::lexical_cast<string>(optType) + "'>" + strData + "</option>\n";
    }
    if (optCount > 0)
    {
        retval += "  <options count='" + boost::lexical_cast<string>(optCount) + "'>\n";
        retval += optList;
        retval += "  </options>\n";
    }

    retval += "</task>\n";
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void WeTask::FromXml( string input )
///
/// @brief  Initializes this object from the given from XML.
///
/// This function reconstructs object back from the XML generated by the @b ToXml function
///
/// @param  input - The input XML.
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeTask::FromXml( string input )
{
    WeStrStream st(input.c_str());
    WeTagScanner sc(st);

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::FromXml - string");
    FromXml(sc);
}

void WeTask::FromXml( WeTagScanner& sc, int token /*= -1*/ )
{
    int pos;
    int parseLevel = 0;
    int intData;
    bool inParsing = true;
    string name, val, dat;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::FromXml - WeTagScanner");
    while (inParsing)
    {
        pos = sc.GetPos();
        if (token == -1) {
            token = sc.GetToken();
        }
        switch(token)
        {
        case wstError:
            LOG4CXX_WARN(WeLogger::GetLogger(), "WeTask::FromXml parsing error");
            inParsing = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(WeLogger::GetLogger(), "WeTask::FromXml - EOF");
            inParsing = false;
            break;
        case wstTagStart:
            name = sc.GetTagName();
            if (parseLevel == 0)
            {
                if (iequals(name, "task"))
                {
                    parseLevel = 1;
                    dat = "";
                }
                else {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "WeTask::FromXml unexpected tagStart: " << name);
                    inParsing = false;
                }
                break;
            }
            if (parseLevel == 1)
            {
                parseLevel = 2;
                dat = "";
                if (iequals(name, "options"))
                {
                    iweOptionsProvider::FromXml(sc, token);
                    parseLevel = 1;
                }
                break;
            }
            LOG4CXX_WARN(WeLogger::GetLogger(), "WeTask::FromXml unexpected tagStart: " << name);
            inParsing = false;
        	break;
        case wstTagEnd:
            name = sc.GetTagName();
            if (parseLevel == 1)
            {
                if (iequals(name, "task"))
                {
                    parseLevel = 0;
                    dat = "";
                    inParsing = false;
                }
                else {
                    LOG4CXX_WARN(WeLogger::GetLogger(), "WeTask::FromXml unexpected wstTagEnd: " << name);
                    inParsing = false;
                }
            }
            if (parseLevel == 2)
            {
                dat = WeUnscreenXML(dat);
                if (iequals(name, "name"))
                {
                    Option(weoName, dat);
                }
                if (iequals(name, "status"))
                {
                    intData = boost::lexical_cast<int>(dat);
                    Option(weoTaskStatus, intData);
                }
                if (iequals(name, "completion"))
                {
                    intData = boost::lexical_cast<int>(dat);
                    Option(weoTaskCompletion, intData);
                }
                parseLevel = 1;
            }
            break;
        case wstAttr:
            name = sc.GetAttrName();
            val = sc.GetValue();
            val = WeUnscreenXML(val);
            if (parseLevel == 1)
            {
                if (iequals(name, "id"))
                {
                    Option(weoID, val);
                }
            }
            break;
        case wstWord:
        case wstSpace:
            dat += sc.GetValue();
            break;
        default:
            break;
        }
        token = -1;
    }
}

void WeTask::WaitForData()
{
    if (event_ptr != NULL && mutex_ptr != NULL) {
        boost::mutex *mt = (boost::mutex*)mutex_ptr;
        boost::condition_variable *cond = (boost::condition_variable*)event_ptr;
        boost::unique_lock<boost::mutex> lock(*mt);
        while(taskList.size() == 0 && ! processThread) {
            cond->wait(lock);
        }
    }
}

void WeTask::CalcStatus()
{
    WeRequestMap::iterator tsk_it;
    size_t count = 0;
    for (tsk_it = taskList.begin(); tsk_it != taskList.end(); tsk_it++)
    {
        if (tsk_it->second != NULL)
        {
            count++;
        }
    }
    size_t task_list_max_size = taskList.size();
    int idata = (task_list_max_size - count) * 100 / task_list_max_size;
    LOG4CXX_DEBUG(WeLogger::GetLogger(), "WeTask::CalcStatus: rest " << count << " queries  from " <<
                task_list_max_size << " (" << idata << "%)");
    Option(weoTaskCompletion, idata);
}
