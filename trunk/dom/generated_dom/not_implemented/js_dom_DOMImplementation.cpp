
/*
  DO NOT EDIT!
  This file has been generated by generate_sources.py script.
  $Id$
*/

#include <html_js.h>
using namespace v8;
bool js_dom_DOMImplementation::hasFeature(std::string val_feature, std::string val_version)
{
    return dom::DOMImplementation::hasFeature(val_feature, val_version);
}
v8::Handle<v8::Value> js_dom_DOMImplementation::createDocumentType(std::string val_qualifiedName, std::string val_publicId, std::string val_systemId)
{
    return dom::DOMImplementation::createDocumentType(val_qualifiedName, val_publicId, val_systemId);
}
v8::Handle<v8::Value> js_dom_DOMImplementation::createDocument(std::string val_namespaceURI, std::string val_qualifiedName, v8::Handle<v8::Value> val_doctype)
{
    return dom::DOMImplementation::createDocument(val_namespaceURI, val_qualifiedName, val_doctype);
}
