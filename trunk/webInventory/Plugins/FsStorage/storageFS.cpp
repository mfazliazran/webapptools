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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>
#include <strstream>
#include "storageFS.h"
#include "version.h"
#include "fsStorage.xpm"
//#include "demoPlugin.xrc"
#include <weDispatch.h>

FsStorage::FsStorage( WeDispatch* krnl, void* handle /*= NULL*/ ) :
    iweStorage(krnl, handle)
{
    pluginInfo.IfaceName = "FsStorage";
    pluginInfo.IfaceList.push_back("FsStorage");
    pluginInfo.PluginDesc = "FileSystem storage";
    pluginInfo.PluginId = "1FBAB8DCF440";
    pluginInfo.PluginIcon = WeXpmToStringList(fsStorage_xpm, sizeof(fsStorage_xpm) / sizeof(char*) );
    db_dir = "";
    if (krnl != NULL) {
        logger = krnl->GetLogger();
    }
    else {
        logger = WeLogger::GetLogger();
    }
    LOG4CXX_TRACE(logger, "FsStorage plugin created");
}

FsStorage::~FsStorage(void)
{
    LOG4CXX_TRACE(logger, "FsStorage plugin destroyed");
}

void* FsStorage::GetInterface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "FsStorage::GetInterface " << ifName);
    if (iequals(ifName, "FsStorage"))
    {
        LOG4CXX_DEBUG(logger, "FsStorage::GetInterface found!");
        usageCount++;
        return (void*)(this);
    }
    return iweStorage::GetInterface(ifName);
}

const string FsStorage::GetSetupUI( void )
{
    /// @todo: change XRC to set the fields values
    return "";
}

void FsStorage::ApplySettings( const string& xmlData )
{

}

bool FsStorage::InitStorage(const string& params)
{
    bool retval = false;
    
    LOG4CXX_TRACE(logger, "FsStorage::InitStorage params = " << params);
    try {
        db_dir = params;
        fs::path dir_path(db_dir);
        if ( !fs::exists(dir_path) ) {
            fs::create_directory(db_dir.c_str());
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                dir_path = dir_path.remove_filename();
                db_dir = dir_path.string();
            }
        }
        LOG4CXX_TRACE(logger, "FsStorage::InitStorage: base dir is " << db_dir);

        // check tasks storage presence
        dir_path = db_dir;
        dir_path /= weObjTypeTask;
        if ( !fs::exists(dir_path) ) {
            fs::create_directory(dir_path);
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                string msg = dir_path.string() + "isn't a directory";
                throw std::exception(msg.c_str());
            }
        }
        LOG4CXX_TRACE(logger, "FsStorage::InitStorage: tasks storage dir is " << dir_path.string());

        // check sysoptions storage presence
        dir_path = db_dir;
        dir_path /= weObjTypeSysOption;
        if ( !fs::exists(dir_path) ) {
            fs::create_directory(dir_path);
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                string msg = dir_path.string() + "isn't a directory";
                throw std::exception(msg.c_str());
            }
        }
        LOG4CXX_TRACE(logger, "FsStorage::InitStorage: system options storage dir is " << dir_path.string());

        // check dictionaries storage presence
        dir_path = db_dir;
        dir_path /= weObjTypeDictionary;
        if ( !exists(dir_path) ) {
            fs::create_directory(dir_path);
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                string msg = dir_path.string() + "isn't a directory";
                throw std::exception(msg.c_str());
            }
        }
        LOG4CXX_TRACE(logger, "FsStorage::InitStorage: dictionaries storage dir is " << dir_path.string());

        // check auths storage presence
        dir_path = db_dir;
        dir_path /= weObjTypeAuthInfo;
        if ( !fs::exists(dir_path) ) {
            fs::create_directory(dir_path);
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                string msg = dir_path.string() + "isn't a directory";
                throw std::exception(msg.c_str());
            }
        }
        LOG4CXX_TRACE(logger, "FsStorage::InitStorage: authorization storage dir is " << dir_path.string());

        // check scans storage presence
        dir_path = db_dir;
        dir_path /= weObjTypeScan;
        if ( !fs::exists(dir_path) ) {
            fs::create_directory(dir_path);
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                string msg = dir_path.string() + "isn't a directory";
                throw std::exception(msg.c_str());
            }
        }
        LOG4CXX_TRACE(logger, "FsStorage::InitStorage: scans storage dir is " << dir_path.string());
    }
    catch(std::exception& e) {
        LOG4CXX_ERROR(logger, "FsStorage::InitStorage: " << e.what());
        retval = false;
    }
    return retval;
}

void FsStorage::Flush(const string& params /*= ""*/)
{
    // nothing to do. Just reimplement base class method
}

string FsStorage::GenerateID(const string& objType /*= ""*/)
{
    string retval = "";
    fs::path dir_path(db_dir);
    
    dir_path /= objType;
    
    do {
        retval = lexical_cast<string>(++lastId);
    } while (fs::exists(dir_path / retval));
    
    return retval;
}

int FsStorage::Query(const string& objType, const string& objId, Operation op, const string& xmlData)
{
    int retval = 0;
    fs::path dir_path(db_dir);
 
    LOG4CXX_DEBUG(logger, "FsStorage::Query objType=" << objType << "; ID=" << objId << "; operation=" << op);
    dir_path /= objType;
    if (op == iweStorage::remove)
    {
        retval += FileRemove(dir_path, objId);
    }
    else {
        retval += FileSave(dir_path, objId, xmlData);
    }
    return retval;
}

int FsStorage::Report(const string& repType, const string& objId, const string& xmlData, string& result)
{
    int retval = 0;
    fs::path dir_path(db_dir);
 
    dir_path /= repType;

    result = "";
    if (objId == "*")
    {
        // list all files in the directory
        fs::directory_iterator end_iter;
        for ( fs::directory_iterator dir_itr( dir_path );
            dir_itr != end_iter;
            ++dir_itr )
        {
            try {
                if ( fs::is_regular_file( dir_itr->status() ) )
                {
                    result += FsStorage::FileRead(dir_itr->path().string());
                    retval++;
                }
            }
            catch ( const std::exception & e )
            {
                LOG4CXX_ERROR(logger, "FsStorage::Report " << dir_itr->path().string() << " " << e.what());
            }
        }
    }
    else
    {
        dir_path /= objId;

        if (exists(dir_path))
        {
            // read file
            result += FsStorage::FileRead(dir_path.string());
            retval++;
        }
    }

    return retval;
}

string FsStorage::FileRead(const string& fname)
{
    string retval = "";
    size_t sz;
    
    LOG4CXX_TRACE(logger, "FsStorage::FileRead " << fname);
    sz = fs::file_size(fname);
    if (sz > 0) {
        char* content = new char[sz];
        ifstream ifs(fname.c_str());
        ifs.read(content, sz);
        retval = content;
        delete content;
    }
    
    return retval;
}

int FsStorage::FileRemove( const fs::path& fspath, const string& fname )
{
    fs::path fp;
    int retval = 0;

    fp = fspath;
    // remove files
    LOG4CXX_TRACE(logger, "FsStorage::FileRemove " << fp.string() << " --- " << fname);
    if (fname == "*")
    {
        fs::directory_iterator end_iter;
        for ( fs::directory_iterator dir_itr( fp );
            dir_itr != end_iter;
            ++dir_itr )
        {
            try {
                if ( fs::is_regular_file( dir_itr->status() ) )
                {
                    fs::remove(dir_itr->path());
                    retval++;
                }
            }
            catch ( const std::exception & e )
            {
                LOG4CXX_ERROR(logger, "FsStorage::FileRemove " << dir_itr->path().string() << " " << e.what());
            }
        }
    }
    else {
        try {
            fp /= fname;
            fs::remove(fp);
            retval++;
        }
        catch ( const std::exception & e )
        {
            LOG4CXX_ERROR(logger, "FsStorage::FileRemove " << fp.string() << " " << e.what());
        }
    }
    return retval;
}

int FsStorage::FileSave( const fs::path& fspath, const string& fname, const string& content )
{
    fs::path fp;
    int retval = 0;

    fp = fspath;
    // save data
    LOG4CXX_TRACE(logger, "FsStorage::FileSave " << fp.string() << " --- " << fname);
    fp /= fname;
    try {
        ofstream ofs(fp.string().c_str());
        ofs << content;
        retval = 1;
    }
    catch(const std::exception& e)
    {
        LOG4CXX_ERROR(logger, "FsStorage::FileSave can't save " << fp.string() << " : " << e.what());
    }
    return retval;
}