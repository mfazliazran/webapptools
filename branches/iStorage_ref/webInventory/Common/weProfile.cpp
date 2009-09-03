/*
    scanServer is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of scanServer

    scanServer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    scanServer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with inventoryScanner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <weHelper.h>
#include "weProfile.h"

std::string WeProfile::ToXml( void )
{
    string result;
    string sdata;

    LOG4CXX_TRACE(iLogger::GetLogger(), "WeProfile::ToXml");
    wOption opt = Option(weoID);
    SAFE_GET_OPTION_VAL(opt, sdata, "0");
    result = "<profile id='" + sdata + "'>\n";
    result += iOptionsProvider::ToXml();
    result += "</profile>\n";

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn void WeProfile::FromXml( string input )
///
/// @brief  Initializes this object from the given from XML. 
///
/// This function reconstructs object back from the XML generated by the @b ToXml function
///
/// @param  input - The input XML. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void WeProfile::FromXml( string input )
{
    StrStream st(input.c_str());
    TagScanner sc(st);

    LOG4CXX_TRACE(iLogger::GetLogger(), "WeProfile::FromXml - string");
    FromXml(sc);
}

void WeProfile::FromXml( TagScanner& sc, int token /*= -1 */ )
{
    int pos;
    int parseLevel = 0;
    bool inParsing = true;
    string name, val, dat;

    LOG4CXX_TRACE(iLogger::GetLogger(), "WeProfile::FromXml - TagScanner");
    while (inParsing)
    {
        pos = sc.GetPos();
        if (token == -1) {
            token = sc.GetToken();
        }
        switch(token)
        {
        case wstError:
            LOG4CXX_WARN(iLogger::GetLogger(), "WeProfile::FromXml parsing error");
            inParsing = false;
            break;
        case wstEof:
            LOG4CXX_TRACE(iLogger::GetLogger(), "WeProfile::FromXml - EOF");
            inParsing = false;
            break;
        case wstTagStart:
            name = sc.GetTagName();
            if (parseLevel == 0)
            {
                if (iequals(name, "profile"))
                {
                    parseLevel = 1;
                    dat = "";
                }
                else {
                    LOG4CXX_WARN(iLogger::GetLogger(), "WeProfile::FromXml unexpected tagStart: " << name);
                    inParsing = false;
                }
                break;
            }
            if (parseLevel == 1)
            {
                parseLevel = 2;
                if (iequals(name, "options"))
                {
                    iOptionsProvider::FromXml(sc, token);
                    parseLevel = 1;
                }
                dat = "";
                break;
            }
            LOG4CXX_WARN(iLogger::GetLogger(), "WeProfile::FromXml unexpected tagStart: " << name);
            inParsing = false;
            break;
        case wstTagEnd:
            name = sc.GetTagName();
            if (parseLevel == 1)
            {
                if (iequals(name, "profile"))
                {
                    parseLevel = 0;
                    dat = "";
                    inParsing = false;
                }
                else {
                    LOG4CXX_WARN(iLogger::GetLogger(), "WeProfile::FromXml unexpected wstTagEnd: " << name);
                    inParsing = false;
                }
            }
            if (parseLevel == 2)
            {
                parseLevel = 1;
            }
            break;
        case wstAttr:
            name = sc.GetAttrName();
            val = sc.GetValue();
            val = UnscreenXML(val);
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