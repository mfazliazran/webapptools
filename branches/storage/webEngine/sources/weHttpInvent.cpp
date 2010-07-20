/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <webEngine.h>
#include <weHelper.h>
#include <weDispatch.h>
#include <weTask.h>
#include <weHttpInvent.h>
#include <weScan.h>

static string xrc = "<plugin><category label='Basic settings' name='generic'>\
 <option name='httpInventory/BaseURL' label='init transport_url' type='8' control='string' mode='window'>/</option>\
 <category label='Crawler' name='crawler'>\
  <option name='' label='Crawling mode'>&lt;composed&gt;\
   <option name='httpInventory/StayInDir' label='Stay in directory' type='6' control='bool' chkbox='1'>1</option>\
   <option name='httpInventory/StayInHost' label='Stay in host' type='6' control='bool' chkbox='1'>0</option>\
   <option name='httpInventory/StayInIP' label='Stay in domain and IP' type='6' control='bool' chkbox='1'>0</option>\
   <option name='httpInventory/StayInDomain' label='Stay in domain' type='6' control='bool' chkbox='1'>0</option>\
  </option>\
  <option name='httpInventory/ScanDepth' label='Scan depth' type='2' control='int'>-1</option>\
  <option name='httpInventory/noParamUrl' label='Ignore transport_url parameters' type='6' control='bool' chkbox='1'>0</option>\
 </category>\
 <option name='httpInventory/ParallelReq' label='Number of downloaders' type='2' control='int' min='1' max='100'>10</option>\
 <option name='httpInventory/AllowedCType' label='Allowed content-type' type='2' control='enum'>Any;empty and \"text/*\";only \"text/*\";empty and \"text/html\";only \"text/html\"</option>\
 <option name='httpInventory/Subdomains' label='Allowed subdomains list' type='8' control='string' mode='window'></option>\
</category></plugin>";

using namespace boost;

namespace webEngine {

HttpInventory::HttpInventory(engine_dispatcher* krnl, void* handle /*= NULL*/) :
    i_inventory(krnl, handle)
{
    pluginInfo.interface_name = "httpInventory";
    pluginInfo.interface_list.push_back("httpInventory");
    pluginInfo.plugin_desc = "Inventory through HTTP protocol";
    pluginInfo.plugin_id = "AB7ED6E5A7B3"; //{7318EAB5-4253-4a31-8284-AB7ED6E5A7B3}
}

HttpInventory::~HttpInventory(void)
{
}

i_plugin* HttpInventory::get_interface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "HttpInventory::get_interface " << ifName);
    if (iequals(ifName, "httpInventory"))
    {
        LOG4CXX_DEBUG(logger, "HttpInventory::get_interface found!");
        usageCount++;
        return (this);
    }
    return i_inventory::get_interface(ifName);
}

const string HttpInventory::get_setup_ui( void )
{
    return xrc;
}

void HttpInventory::init( task* tsk )
{
    transport_url   start_url;

    if (tsk)
    {
        parent_task = tsk;
        if (kernel)
        {
            string host;
            we_option opt = tsk->Option("scan_host");
            SAFE_GET_OPTION_VAL(opt, host, "");
            if (host != "")
            {
                string path;
                // create list of the blocked extension
                opt = tsk->Option("httpInventory/"weoDeniedFileTypes);
                SAFE_GET_OPTION_VAL(opt, path, 1);
                if (path != "")
                {
                    size_t pos = path.find(';');
                    ext_deny.clear();
                    while(pos != string::npos) {
                        string ext = path.substr(0, pos);
                        ext_deny.push_back(ext);
                        if (pos < path.length())
                        {
                            path = path.substr(pos+1);
                        }
                        else {
                            path = "";
                        }
                        pos = path.find(';');
                    }
                }

                // create list of allowed sub-domains
                opt = tsk->Option("httpInventory/"weoDomainsAllow);
                SAFE_GET_OPTION_VAL(opt, path, 1);
                if (path != "")
                {
                    size_t pos = path.find(';');
                    domain_allow.clear();
                    while(pos != string::npos) {
                        string ext = path.substr(0, pos);
                        domain_allow.push_back(ext);
                        if (pos < path.length())
                        {
                            path = path.substr(pos+1);
                        }
                        else {
                            path = "";
                        }
                        pos = path.find(';');
                    }
                }

                // processing options
                opt_in_host = parent_task->IsSet("httpInventory/"weoStayInHost);
                opt_in_domain = parent_task->IsSet("httpInventory/"weoStayInDomain);
                opt_ignore_param = parent_task->IsSet("httpInventory/"weoIgnoreUrlParam);
                opt = parent_task->Option("httpInventory/"weoScanDepth);
                SAFE_GET_OPTION_VAL(opt, opt_max_depth, 0);
                opt = parent_task->Option("httpInventory/AllowedCType");
                SAFE_GET_OPTION_VAL(opt, opt_ctype_method, 0); // default - any type


                opt = tsk->Option("httpInventory/BaseURL");
                SAFE_GET_OPTION_VAL(opt, path, "");
                start_url.host = host;
                start_url.request = path;
                host += path;
                LOG4CXX_INFO(logger, "HttpInventory::init: init scanning from: " << host << " ==> " << start_url.tostring());
                HttpRequest* req = new HttpRequest;
                req->RequestUrl(start_url);
                LOG4CXX_TRACE(logger, "HttpInventory::init: init request = " << req->RequestUrl().tostring());
//                req->processor = HttpInventory::response_dispatcher;
//                req->context = (void*)this;
                parent_task->register_url(start_url.tostring());
                parent_task->get_request_async(i_request_ptr(req));
            }
            else {
                LOG4CXX_WARN(logger, "HttpInventory::init: Can't find hostname. Finishing.");
            }
        }
        else {
            LOG4CXX_WARN(logger, "HttpInventory::init: No kernel given - can't read options. Finishing.");
        }
    }
    else {
        LOG4CXX_WARN(logger, "HttpInventory::init: No parent task - can't process requests. Finishing.");
    }
}

void HttpInventory::process( task* tsk, scan_data_ptr scData )
{
    HttpResponse* htResp;

    try {
        htResp = reinterpret_cast<HttpResponse*>(scData->response.get());
    }
    catch (...) {
        LOG4CXX_ERROR(logger, "HttpInventory::process: The response from " << scData->response->BaseUrl().tostring() << " isn't the HttpResponse!");
        return;
    }
    if (htResp == NULL) {
        LOG4CXX_TRACE(logger, "HttpInventory::process: subsequent processing, or transport fail - no responce given.");
        return;
    }
    // process response
    LOG4CXX_TRACE(logger, "HttpInventory::process: process response with code=" << htResp->HttpCode());
    if (scData->parent_id == "")
    {
        scData->parent_id = htResp->ID();
    }
    if (scData->data_id == "")
    {
        scData->data_id = kernel->storage()->generate_id(weObjTypeScan);
    }
    scData->resp_code = htResp->HttpCode();
    scData->download_time = htResp->DownloadTime();
    scData->data_size = htResp->Data().size();
    scData->scan_depth = htResp->depth();
    scData->content_type = htResp->ContentType();

    /// @todo process options
    if (htResp->HttpCode() >= 300 && htResp->HttpCode() < 400) {
        // redirections
        LOG4CXX_TRACE(logger, "HttpInventory::process: process redirect");
        string url = htResp->Headers().find_first("Location");
        if (!url.empty()) {
            LOG4CXX_DEBUG(iLogger::GetLogger(), "HttpInventory::process: redirected to " << url);
            bool to_process = true;
            transport_url baseUrl = htResp->BaseUrl();
            baseUrl.assign_with_referer(url);
            LOG4CXX_DEBUG(logger, "HttpInventory::process: reconstructed url is " << baseUrl.tostring());
            add_url(baseUrl, htResp, scData);
        }
    }
    if ((htResp->HttpCode() > 0 && htResp->HttpCode() < 300) || htResp->Data().size() > 0)
    {
        shared_ptr<html_document> parser;
        entity_list lst;

        try{
            parser = boost::shared_dynamic_cast<html_document>(scData->parsed_data);
        }
        catch (bad_cast) {
            LOG4CXX_ERROR(logger, "HttpInventory::process: can't process given document as html_document");
            parser.reset();
        }
        if (parser) {
            LOG4CXX_DEBUG(logger, "HttpInventory::process: search for links");
            lst = parser->FindTags("a");
            if (lst.size() > 0) {
                base_entity* ent = NULL;
                entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("href");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_TRACE(logger, "HttpInventory::process: <a href...> tag. url=" << link.tostring());
                        add_url(link, htResp, scData);
                    } // end href attribute
                } // end <a ...> loop
                ClearEntityList(lst);
            } // end <a ...> tags processing
            // todo: process other kind of links
            // CSS
            // scripts
            // objects
            // images
            lst = parser->FindTags("img");
            if (lst.size() > 0)
            {
                webEngine::base_entity* ent = NULL;
                webEngine::entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("src");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_DEBUG(logger, "HttpInventory::process: IMG url = " << link.tostring());
                        add_url(link, htResp, scData);
                    } // end src attribute
                } // end <img ...> loop
                ClearEntityList(lst);
            } // end <img ...> tags processing
            // frames
            lst = parser->FindTags("frame");
            if (lst.size() > 0)
            {
                webEngine::base_entity* ent = NULL;
                webEngine::entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("src");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_DEBUG(logger, "HttpInventory::process: FRAME url = " << link.tostring());
                        add_url(link, htResp, scData);
                    } // end src attribute
                } // end <iframe ...> loop
                ClearEntityList(lst);
            } // end <iframe ...> tags processing
            // iframes
            lst = parser->FindTags("iframe");
            if (lst.size() > 0)
            {
                webEngine::base_entity* ent = NULL;
                webEngine::entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("src");
                    if (href != "") {
                        transport_url link;
                        link.assign_with_referer(href, &(htResp->RealUrl()));
                        LOG4CXX_DEBUG(logger, "HttpInventory::process: IFRAME url = " << link.tostring());
                        add_url(link, htResp, scData);
                    } // end src attribute
                } // end <iframe ...> loop
                ClearEntityList(lst);
            } // end <iframe ...> tags processing
            // forms
            // meta
            lst = parser->FindTags("meta");
            if (lst.size() > 0)
            {
                base_entity* ent = NULL;
                entity_list::iterator iEnt;
                string href;
                for (iEnt = lst.begin(); iEnt != lst.end(); iEnt++) {
                    href = (*iEnt)->attr("http-equiv");
                    if (boost::iequals(href, "refresh")) {
                        href = (*iEnt)->attr("content");
                        if (href != "") {
                            transport_url link;
                            string lc = href;
                            to_lower(lc);
                            size_t np = lc.find("url=");
                            if (np != std::string::npos) {
                                href = href.substr(np+4);
                            }
                            link.assign_with_referer(href, &(htResp->RealUrl()));
                            LOG4CXX_DEBUG(logger, "HttpInventory::process: META url = " << link.tostring());
                            add_url(link, htResp, scData);
                        }
                    } // end http-equiv attribute
                } // end <meta ...> loop
                ClearEntityList(lst);
            } // end <meta ...> tags processing
            // etc ???
        } // if document parsed
    } // if HTTP code valid

    // update scan_data
    parent_task->set_scan_data(scData->object_url, scData);
}

void HttpInventory::add_url( transport_url link, HttpResponse *htResp, boost::shared_ptr< ScanData > scData )
{
    bool allowed = true;
    if (!link.is_host_equal(htResp->RealUrl()))
    {
        if (opt_in_host)
        {
            allowed = false;
        }
        LOG4CXX_TRACE(logger, "HttpInventory::add_url: weoStayInHost check " << allowed << " (" << link.tostring() << ")");
    }
    if (!link.is_domain_equal(htResp->RealUrl()))
    {
        if (opt_in_domain)
        {
            allowed = false;
        }
        LOG4CXX_TRACE(logger, "HttpInventory::add_url: weoStayInDomain check << " << allowed << " (" << link.tostring() << ")");
    }
    if ( opt_max_depth > 0 && htResp->depth() >= opt_max_depth) {
        LOG4CXX_DEBUG(logger, "HttpInventory::add_url: maximum scanning depth reached! (" << opt_max_depth << ")");
        allowed = false;
    }
    if (allowed)
    {    // verify blocked file types
        string path = link.request;
        int pos = path.find_last_of('.');
        if (pos != string::npos && ext_deny.size() > 0)
        {
            path = path.substr(pos+1);
            LOG4CXX_TRACE(logger, "HttpInventory::add_url: Found extension: " << path << "; Deny list size is " << ext_deny.size());
            for (size_t i = 0; i < ext_deny.size(); i++) {
                if (path == ext_deny[i]) {
                    allowed = false;
                    LOG4CXX_DEBUG(logger, "HttpInventory::add_http_url: not need to download " << link.tostring());
                    // make the pseudo-response
                    shared_ptr<ScanData> scn = parent_task->get_scan_data(link.tostring());
                    if (scn->data_id == "")
                    {
                        scn->data_id = kernel->storage()->generate_id(weObjTypeScan);
                        scn->parent_id = scData->data_id;
                        scn->resp_code = 204; // 204 No Content;  The server successfully processed the request, but is not returning any content
                        scn->download_time = 0;
                        scn->data_size = 0;
                        scn->scan_depth = htResp->depth() + 1;
                        scn->content_type = "application/octet-stream";
                        scn->parsed_data.reset();
                        parent_task->set_scan_data(scn->object_url, scn);
                    }
                    break;
                } // if path == ext_deny[i]
            } // for deny list size
        } // if file extension found
    } // if allowed
    if (allowed)
    {
        string u_req = link.tostring();
        if (opt_ignore_param) {
            u_req = link.tostring_noparam();
        }
        LOG4CXX_TRACE(logger, "HttpInventory::add_http_url: weoIgnoreUrlParam check << " << u_req);
        if (!parent_task->is_url_processed(u_req))
        {
            HttpRequest* new_url = new HttpRequest(u_req);
            new_url->depth(htResp->depth() + 1);
            new_url->ID(scData->data_id);
//            new_url->processor = HttpInventory::response_dispatcher;
//            new_url->context = (void*)this;
            parent_task->get_request_async(i_request_ptr(new_url));
        }
        else
        {
            // add parent to existing scan data
        }
        parent_task->register_url(u_req);
    }
}
} // namespace webEngine