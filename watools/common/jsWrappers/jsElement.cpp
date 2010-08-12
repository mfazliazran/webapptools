#include <boost/algorithm/string.hpp>
#include <weLogger.h>
#include "jsGlobal.h"
#include "jsElement.h"
#include "jsBrowser.h"
#include "jsCssStyle.h"

using namespace v8;
using namespace webEngine;

/*extern bool ExecuteString(Handle<String> source,
                          Handle<Value> name,
                          bool print_result,
                          bool report_exceptions);*/

static char* func_list[] = {
    "appendChild",
    "blur",
    "click",
    "cloneNode",
    "detachEvent", // from jQuery - event processing
    "focus",
    "getAttribute",
    "getElementsByTagName",
    "hasChildNodes",
    "insertBefore",
    "item",
    "normalize",
    "removeAttribute",
    "removeChild",
    "replaceChild",
    "setAttribute",
    "toString"
};
static char* rw_list[] = {
    "accessKey",//
    "className",//
    "dir",
    "disabled",
    "height",//
    "id",//
    "innerHTML",
    "lang",
    "style",
    "tabIndex",
    "title",//
    "width"//
};
static char* ro_list[] = {
    "attributes",
    "childNodes",

    "clientHeight",
    "clientWidth",
    "firstChild",
    "lastChild",
    "length",
    "nextSibling",
    "nodeName",
    "nodeType",
    "nodeValue",
    "offsetHeight",
    "offsetLeft",
    "offsetParent",
    "offsetTop",
    "offsetWidth",
    "ownerDocument",
    "parentNode",
    "previousSibling",
    "scrollHeight",
    "scrollLeft",
    "scrollTop",
    "tagName"
};

std::vector<std::string> jsElement::funcs(func_list, func_list + sizeof func_list / sizeof func_list[ 0 ]);
std::vector<std::string> jsElement::rw_props(rw_list, rw_list + sizeof rw_list / sizeof rw_list[ 0 ]);
std::vector<std::string> jsElement::ro_props(ro_list, ro_list + sizeof ro_list / sizeof ro_list[ 0 ]);

Persistent<FunctionTemplate> jsElement::object_template;
bool jsElement::is_init = false;

Persistent<FunctionTemplate> jsAttribute::object_template;
bool jsAttribute::is_init = false;

static void AddChildren(entity_list &curr)
{
    entity_list chld;
    Handle<Object> val;

    for (size_t i = 0; i < curr.size(); ++i) {
        val = wrap_entity(boost::shared_dynamic_cast<html_entity>(curr[i]));
        append_object(val);
        chld = curr[i]->Children();
        if (chld.size() > 0) {
            AddChildren(chld);
        }
        ClearEntityList(chld);
    }
}

Handle<Value> Image(const Arguments& args)
{
    HandleScope scope;
    Handle<Object> res;
    Local<Context> ctx = v8::Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
        v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(eObj->GetInternalField(0));
        jsBrowser* jsExec = static_cast<jsBrowser*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets jsBrowser");
        jsElement *p = new jsElement(jsExec->window->document->doc);
        p->entity()->Name("img");
        res = wrap_object<jsElement>(p);
        append_object(res);
    }
    return scope.Close(res);
}

Handle<Value> Element(const Arguments& args)
{
    HandleScope scope;
    Handle<Object> res;
    Local<Context> ctx = Context::GetCurrent();
    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets v8_context");
    if (exec->IsObject())
    {
        Local<Object> eObj = Local<Object>::Cast(exec);
        v8::Local<v8::External> wrap = Local<External>::Cast(eObj->GetInternalField(0));
        jsBrowser* jsExec = static_cast<jsBrowser*>(wrap->Value());
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "js::Window: gets jsBrowser");
        if (args.Length() > 0) {
            html_entity_ptr hent(new html_entity(jsExec->window->document->doc));
            string tag = value_to_string(args[0]->ToString());
            hent->Name(tag);
            res = wrap_entity(hent);
            append_object(res);
        }
    }
    return scope.Close(res);
}

jsElement::jsElement(html_document_ptr document)
{
    if (!is_init) {
        init();
    }
    // todo - link to the document
    html_ent.reset(new html_entity());
    html_ent->Parent(document);
    document->Children().push_back(html_ent);
    css_style = new jsCssStyle(html_ent);
}

jsElement::jsElement(html_entity_ptr ent)
{
    if (!is_init) {
        init();
    }
    html_ent = ent;
    css_style = new jsCssStyle(html_ent);
}

jsElement::~jsElement(void)
{
    delete css_style;
}

void jsElement::init()
{
    is_init = true;
    Handle<FunctionTemplate> _object = FunctionTemplate::New();
    //get the location's instance template
    Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
    //set its internal field count to one (we'll put references to the C++ point here later)
    _proto->SetInternalFieldCount(1);

    // Add accessors for each of the fields.
    _proto->Set(String::New("toString"), FunctionTemplate::New(jsElement::ToString));
    _proto->Set(String::New("appendChild"), FunctionTemplate::New(jsElement::AppendChild));
    _proto->Set(String::New("blur"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("click"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("cloneNode"), FunctionTemplate::New(jsElement::CloneNode));
    // from jQuery - event processing
    _proto->Set(String::New("detachEvent"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("focus"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("getAttribute"), FunctionTemplate::New(jsElement::GetAttribute));
    _proto->Set(String::New("getElementsByTagName"), FunctionTemplate::New(jsElement::GetElemsByName));
    _proto->Set(String::New("hasChildNodes"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("insertBefore"), FunctionTemplate::New(jsElement::InsertBefore));
    _proto->Set(String::New("item"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("normalize"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("removeAttribute"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("removeChild"), FunctionTemplate::New(jsElement::RemoveChild));
    _proto->Set(String::New("replaceChild"), FunctionTemplate::New(jsElement::PlaceHolder));
    _proto->Set(String::New("setAttribute"), FunctionTemplate::New(jsElement::SetAttribute));
    _proto->SetNamedPropertyHandler(jsElement::PropertyGet, jsElement::PropertySet, NULL, NULL, jsElement::PropertyEnum);

    object_template = Persistent<FunctionTemplate>::New(_object);
}

Handle<Value> jsElement::GetProperty( Local<String> name, const AccessorInfo &info )
{
    Handle<Value> val;
    std::string key = value_to_string(name);

    vector<string>::iterator iter;
    // Look up the value in the RW propeties list.
    iter = find(rw_props.begin(), rw_props.end(), key);
    if (iter != rw_props.end()) {
        if (key == "id" || key == "height" || key == "width" || key == "title" || key == "accessKey") {
            // "attributes" properties
            string sval = entity()->attr(key);
            val = Local<Value>::New(String::New(sval.c_str()));
        }
        else if (key == "className") {
            // "attributes" property, but fix name
            string sval = entity()->attr("class");
            val = Local<Value>::New(String::New(sval.c_str()));
        }
        else if (key == "style") {
            val = wrap_object<jsCssStyle>(css_style);
        }
        else if (key == "innerHTML") {
            string sval = entity()->InnerHtml();
            val = Local<Value>::New(String::New(sval.c_str()));
        }
    }
    else {
        // Look up the value in the RO propeties list.
        iter = find(ro_props.begin(), ro_props.end(), key);
        if (iter != ro_props.end()) {
            val = Local<Value>::New(Undefined());
            if (key == "attributes") {
                Handle<Array> elems = Local<Array>::New(Array::New());

                AttrMap::iterator attrib = entity()->attr_list().begin();
                int i = 0;
                while(attrib != entity()->attr_list().end() ) {
                    string atnm = (*attrib).first;

                    jsAttribute* attr = new jsAttribute(this, atnm);
                    Handle<Object> w = wrap_object<jsAttribute>(attr);

                    //elems->Set(Number::New(i), w);
                    elems->Set(String::New(atnm.c_str()), w);

                    ++i;
                    ++attrib;
                }
                val = elems;
            }
            else if (key == "childNodes") {
                Handle<Array> elems = Local<Array>::New(Array::New());

                entity_list ptrs = entity()->Children();
                for(size_t i = 0; i < ptrs.size(); ++i) {
                    Handle<Object> w = wrap_entity(boost::shared_dynamic_cast<html_entity>(ptrs[i]));
                    elems->Set(Number::New(i), w);
                }
                val = elems;
            }
            else if (key == "firstChild") {
                if (entity()->Children().size() > 0) {
                    val = wrap_entity(boost::shared_dynamic_cast<html_entity>(entity()->Child(0)));
                }
            }
            else if (key == "lastChild") {
                if (entity()->Children().size() > 0) {
                    int idx = entity()->Children().size() - 1;
                    val = wrap_entity(boost::shared_dynamic_cast<html_entity>(entity()->Child(idx)));
                }
            }
            else if (key == "nextSibling") {
                LOG4CXX_TRACE(iLogger::GetLogger(), "jsElement::nextSibling");
                if (!entity()->Parent().expired()) {
                    entity_list::iterator bg, en, rf;
                    base_entity_ptr prnt = entity()->Parent().lock();
                    bg = prnt->Children().begin();
                    en = prnt->Children().end();
                    rf = find(bg, en, entity());
                    if (rf != en) {
                        LOG4CXX_TRACE(iLogger::GetLogger(), "jsElement::nextSibling - fount itself");
                        ++rf; // get next sibling
                        if (rf != en) {
                            val = wrap_entity(boost::shared_dynamic_cast<html_entity>(*rf));
                        }
                    }
                }
                else {
                    LOG4CXX_DEBUG(iLogger::GetLogger(), "jsElement::nextSibling - parent reference is expired!");
                }
            }
            else if (key == "nodeName") {
                string sval = entity()->Name();
                boost::to_upper(sval);
                val = String::New(sval.c_str());
            }
            else if (key == "nodeType") {
                string sval = entity()->Name();
                int ntp = ELEMENT_NODE;
                if (sval == "#document") {
                    ntp = DOCUMENT_NODE;
                }
                else if (sval == "#text") {
                    ntp = TEXT_NODE;
                }
                else if (sval == "#comment") {
                    ntp = COMMENT_NODE;
                }
                else if (sval == "#cdata") {
                    ntp = CDATA_SECTION_NODE;
                }
                else if (sval == "#fragment") {
                    ntp = DOCUMENT_FRAGMENT_NODE;
                }
                val = Number::New(ntp);
            }
            else if (key == "nodeValue") {
                string sval = entity()->Name();
                if (sval == "#text" || sval == "#cdata") {
                    sval = entity()->attr("");
                }
                else {
                    sval = entity()->attr("value");
                }
                val = String::New(sval.c_str());
            }
            else if (key == "ownerDocument") {
                html_document* d = (html_document*)entity()->GetRootDocument();
                if (d == NULL) {
                    // get main window's document
                    Local<Context> ctx = v8::Context::GetCurrent();
                    Local<Value> exec = ctx->Global()->Get(String::New("v8_context"));
                    Local<Object> eObj = Local<Object>::Cast(exec);
                    Local<External> wrap = Local<External>::Cast(eObj->GetInternalField(0));
                    jsBrowser* jsExec = static_cast<jsBrowser*>(wrap->Value());
                    d = jsExec->window->document->doc.get();
                }
                LOG4CXX_DEBUG(iLogger::GetLogger(), "jsElement::ownerDocument = " << d);
                jsDocument* jd = new jsDocument(NULL);
                jd->doc.reset(d);
                val = wrap_object<jsDocument>(jd);
            }
            else if (key == "tagName") {
                string sval = entity()->Name();
                val = String::New(sval.c_str());
            }
        }
        else {
            // check special entry for event processing
            if (boost::istarts_with(key, "__event__")) {
                val = Local<Value>::New(evt_handler);
            }
            else {
                // Look up the value in the attributes list.
                AttrMap::iterator itmp;
                itmp = entity()->attr_list().find(key);
                if (itmp != entity()->attr_list().end())
                {
                    val = Local<Value>::New(String::New((*itmp).second.c_str()));
                } // attribute found
            }
        } // RO property search
    } // RW property search

    return val;
}

Handle<Value> jsElement::SetProperty( Local<String> name, Local<Value> value, const AccessorInfo& info )
{
    Handle<Value> val;
    std::string key = value_to_string(name);

    vector<string>::iterator iter;
    // Look up the value in the RO propeties list.
    iter = find(ro_props.begin(), ro_props.end(), key);
    if (iter == ro_props.end()) {
        // Look up the value in the RW propeties list.
        iter = find(rw_props.begin(), rw_props.end(), key);
        if (iter != rw_props.end()) {
            if (key == "id" || key == "height" || key == "width" || key == "title" || key == "accessKey") {
                // "attributes" properties
                string sval = value_to_string(value);
                entity()->attr(key, sval);
            }
            else if (key == "className") {
                // "attributes" property, but fix name
                string sval = value_to_string(value);
                entity()->attr("class", sval);
            }
            else if (key == "style" && value->IsString()) {
                string code = value_to_string(value);
                css_style->computeStyle(code);
            }
            else if (key == "innerHTML" && value->IsString()) {
                // clear previous values
                entity()->ClearChildren();
                string code = value_to_string(value);
                str_tag_stream stream(code.c_str());
                tag_scanner scanner(stream);
                entity()->Parse("", scanner, NULL);
                // add all children as new objects
                entity_list chld = entity()->Children();
                if (chld.size() > 0) {
                    AddChildren(chld);
                }
                ClearEntityList(chld);
            }
        }
        else {
            // check special entry for event processing
            if (boost::istarts_with(key, "__event__")) {
                evt_handler = Persistent<Value>::New(value);
            }
            else {
                // set the attribute
                string sval = value_to_string(value);
                entity()->attr(key, sval);
            }
        } // RO property search
    } // RW property search
    val = value;

    return val;
}

Handle<Value> jsElement::PropertyGet( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;
    string key = value_to_string(name);

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr);
    Handle<Value> val = el->GetProperty(name, info);
    return scope.Close(val);
}

Handle<Value> jsElement::PropertySet( Local<String> name, Local<Value> value, const AccessorInfo& info )
{
    string key = value_to_string(name);
    Handle<Value> retval;

    vector<string>::iterator it = find(jsElement::funcs.begin(), jsElement::funcs.end(), key);
    if (it == jsElement::funcs.end()) {
        Local<Object> self = info.This();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        jsElement* el = static_cast<jsElement*>(ptr); 
        retval = el->SetProperty(name, value, info);
    }
    return retval;
}


Handle<Array> jsElement::PropertyEnum( const AccessorInfo &info )
{
    HandleScope scope;

    Handle<Array> retval = Local<Array>::New(Array::New());

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    map<string, int> prop_list;
    size_t i;
    for (i = 0; i < ro_props.size(); ++i) {
        prop_list[ro_props[i]] = 1;
    }
    for (i = 0; i < rw_props.size(); ++i) {
        prop_list[rw_props[i]] = 1;
    }
    AttrMap::iterator it;
    for (it = el->entity()->attr_list().begin(); it != el->entity()->attr_list().end(); ++it) {
        prop_list[(*it).first] = 1;
    }
    map<string, int>::iterator ins;
    i = 0;
    for (ins = prop_list.begin(); ins != prop_list.end(); ++ins) {
        string val = ins->first;
        retval->Set(Number::New(i), String::New(val.c_str()));
        i++;
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::PlaceHolder( const Arguments& args )
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string ret;
    string attr;
    ret = value_to_string(args.Callee()->ToString());
    ret += " [Element; tag=";
    ret += el->entity()->Name();

    attr = el->entity()->attr("id");
    if (attr != "") {
        ret += "; id=";
        ret += attr;
    }
    attr = el->entity()->attr("name");
    if (attr != "") {
        ret += "; name=";
        ret += attr;
    }
    ret += "]";
    LOG4CXX_DEBUG(iLogger::GetLogger(), "jsElement::PlaceHolder - " << ret);
    return String::New(ret.c_str());
}

Handle<Value> jsElement::AppendChild( const Arguments& args )
{
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "jsElement::AppendChild");
    HandleScope scope;

    Handle<Value> retval;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsElement* el = static_cast<jsElement*>(ptr);
    if (args.Length() > 0 && args[0]->IsObject()) {
        Local<Object> aref = args[0]->ToObject();
        Local<External> awrap = Local<External>::Cast(aref->GetInternalField(0));
        void* aptr = awrap->Value();
        jsElement* chld = static_cast<jsElement*>(aptr);
        el->entity()->Children().push_back(chld->entity());
        chld->entity()->Parent(el->entity());
        retval = args[0];
    }
    else {
        LOG4CXX_ERROR(webEngine::iLogger::GetLogger(), "jsElement::AppendChild exception: argument must be an object!\n");
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::ToString( const Arguments& args )
{
    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string attr;
    string ret = "[Element; tag=";
    ret += el->entity()->Name();

    attr = el->entity()->attr("id");
    if (attr != "") {
        ret += "; id=";
        ret += attr;
    }
    attr = el->entity()->attr("name");
    if (attr != "") {
        ret += "; name=";
        ret += attr;
    }
    ret += "]";
    boost::replace_all(ret, "\n", "\\n");
    boost::replace_all(ret, "\r", "");
    return String::New(ret.c_str());
}

Handle<Value> jsElement::GetAttribute( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> retval;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string attr;
    string key;

    if (args.Length() > 0) {
        key = value_to_string(args[0]);
        attr = el->entity()->attr(key);
        retval = String::New(attr.c_str());
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::SetAttribute( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> retval(Undefined());

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string attr;
    string key;

    if (args.Length() > 0) {
        key = value_to_string(args[0]);
        if (args.Length() > 1) {
            attr = value_to_string(args[1]);
            el->entity()->attr(key, attr);
            retval = String::New(attr.c_str());
        }
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::GetElemsByName( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> retval(Undefined());

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    string key;

    if (args.Length() > 0) {
        Handle<Array> elems = Local<Array>::New(Array::New());

        key = value_to_string(args[0]);
        entity_list ptrs = el->entity()->FindTags(key);
        for(size_t i = 0; i < ptrs.size(); ++i) {
            Handle<Object> w = wrap_entity(boost::shared_dynamic_cast<html_entity>(ptrs[i]));
            elems->Set(Number::New(i), w);
        }
        retval = elems;
    }

    return scope.Close(retval);
}

Handle<Value> jsElement::CloneNode( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> retval(Undefined());

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsElement* el = static_cast<jsElement*>(ptr); 

    /// @todo Warning! Fake clone! Implement real clone operation for entity
    html_entity_ptr hte(new html_entity());
    *hte = *(el->entity());
    retval = wrap_entity(hte);

    return scope.Close(retval);
}

Handle<Value> jsElement::RemoveChild( const Arguments& args )
{
    if (args[0]->IsObject()) {
        try {
            HandleScope scope;

            Local<Object> self = args.This();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            jsElement* el = static_cast<jsElement*>(ptr);

            // get the object to remove 
            Local<Object> child = args[0].As<Object>();
            Local<External> wch = Local<External>::Cast(child->GetInternalField(0));
            jsElement* ch = static_cast<jsElement*>(wch->Value());
            if (el->entity() && ch->entity()) {
                base_entity_ptr pptr = ch->entity()->Parent().lock();
                html_entity_ptr chp = boost::shared_dynamic_cast<html_entity>(pptr);
                if (chp == el->entity()) {
                    // todo - remove it!
                    entity_list::iterator bg, en, rf;
                    bg = el->entity()->Children().begin();
                    en = el->entity()->Children().end();
                    rf = find(bg, en, ch->entity());
                    if (rf != en) {
                        LOG4CXX_TRACE(iLogger::GetLogger(), "jsElement::RemoveChild remove it");
                        el->entity()->Children().erase(rf);
                    }
                    else {
                        LOG4CXX_TRACE(iLogger::GetLogger(), "jsElement::RemoveChild child not found :(");
                    }
                }
                else {
                    LOG4CXX_WARN(iLogger::GetLogger(), "jsElement::RemoveChild argument isn't a child!");
                }
            }
            else {
                LOG4CXX_WARN(iLogger::GetLogger(), "jsElement::RemoveChild objects are not valid! parent=" << el->entity().get() << "; child=" << ch->entity().get() );
            }
        }
        catch(...) {
            LOG4CXX_ERROR(iLogger::GetLogger(), "jsElement::RemoveChild - exception!");
        }
    }

    return args[0];
}

Handle<Value> jsElement::InsertBefore( const Arguments& args )
{
    LOG4CXX_TRACE(iLogger::GetLogger(), "jsElement::InsertBefore");
    HandleScope scope;

    Handle<Value> retval;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    jsElement* el = static_cast<jsElement*>(ptr);
    if (args.Length() > 0 && args[0]->IsObject()) {
        if (args.Length() > 1 && args[1]->IsObject()) {
            // insert
            Local<Object> aref = args[0]->ToObject();
            Local<External> awrap = Local<External>::Cast(aref->GetInternalField(0));
            void* aptr = awrap->Value();
            jsElement* chld = static_cast<jsElement*>(aptr);

            Local<Object> rref = args[1]->ToObject();
            Local<External> rwrap = Local<External>::Cast(rref->GetInternalField(0));
            void* rptr = rwrap->Value();
            jsElement* refer = static_cast<jsElement*>(rptr);

            entity_list::iterator bg, en, rf;
            bg = el->entity()->Children().begin();
            en = el->entity()->Children().end();
            rf = find(bg, en, refer->entity());
            if(rf != en) {
                //insert
                LOG4CXX_TRACE(iLogger::GetLogger(), "jsElement::InsertBefore - insert newChild");
                el->entity()->Children().insert(rf, chld->entity());
                chld->entity()->Parent(el->entity());
            }
            else {
                //append
                LOG4CXX_TRACE(iLogger::GetLogger(), "jsElement::InsertBefore - refChild doesn't found, append newChild");
                el->entity()->Children().push_back(chld->entity());
                chld->entity()->Parent(el->entity());
            }
        }
        else {
            LOG4CXX_DEBUG(iLogger::GetLogger(), "jsElement::InsertBefore - refChild isn't an object, append newChild");
            AppendChild(args);
        }
        retval = args[0];
    }
    else {
        LOG4CXX_ERROR(iLogger::GetLogger(), "jsElement::InsertBefore exception: argument must be an object!");
    }

    return scope.Close(retval);

}

jsAttribute::jsAttribute(jsElement* _prnt, string nm)
{
    if (!is_init) {
        init();
    }
    parent = _prnt;
    name = nm;
}

void jsAttribute::init()
{
    is_init = true;
    Handle<FunctionTemplate> _object = FunctionTemplate::New();
    //get the location's instance template
    Handle<ObjectTemplate> _proto = _object->InstanceTemplate();
    //set its internal field count to one (we'll put references to the C++ point here later)
    _proto->SetInternalFieldCount(1);

    // Add accessors for each of the fields.
    _proto->Set(String::New("toString"), FunctionTemplate::New(jsAttribute::ToString));
    _proto->SetAccessor(String::New("value"), jsAttribute::ValueGet, jsAttribute::ValueSet);
    _proto->SetAccessor(String::New("name"), jsAttribute::NameGet);

    object_template = Persistent<FunctionTemplate>::New(_object);
}

Handle<Value> jsAttribute::ToString( const Arguments& args )
{
    HandleScope scope;
    Handle<Value> res;

    Local<Object> self = args.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsAttribute* el = static_cast<jsAttribute*>(ptr);

    string retval = "[object Attribute name=";
    retval += el->name;

    if (el->parent && el->parent->entity()) {
        string val = el->parent->entity()->attr(el->name);
        retval += "; value=";
        retval += val;
    }

    retval += "]";
    boost::replace_all(retval, "\n", "\\n");
    boost::replace_all(retval, "\r", "");
    res = String::New(retval.c_str());
    return scope.Close(res);
}

Handle<Value> jsAttribute::ValueGet( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;
    Handle<Value> res;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsAttribute* el = static_cast<jsAttribute*>(ptr);

    if (el->parent && el->parent->entity()) {
        string val = el->parent->entity()->attr(el->name);
        boost::replace_all(val, "\n", "\\n");
        boost::replace_all(val, "\r", "");
        res = String::New(val.c_str());;
    }

    return scope.Close(res);
}

void jsAttribute::ValueSet( Local<String> name, Local<Value> value, const AccessorInfo &info )
{
    HandleScope scope;
    Handle<Value> res;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsAttribute* el = static_cast<jsAttribute*>(ptr);

    if (el->parent && el->parent->entity()) {
        string val = value_to_string(value);
        el->parent->entity()->attr(el->name, val);
    }
}

Handle<Value> jsAttribute::NameGet( Local<String> name, const AccessorInfo &info )
{
    HandleScope scope;
    Handle<Value> res;

    Local<Object> self = info.This();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    jsAttribute* el = static_cast<jsAttribute*>(ptr);

    res = String::New(el->name.c_str());
    return scope.Close(res);
}
