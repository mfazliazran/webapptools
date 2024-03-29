
/*
  $Id$
*/

#include "precomp.h"
using namespace v8;

js_dom_Node::js_dom_Node() {}

js_dom_Node::~js_dom_Node() {}

v8::Handle<v8::Value> js_dom_Node::insertBefore(v8::Handle<v8::Value> val_newChild, v8::Handle<v8::Value> val_refChild)
{
    return dom::Node::insertBefore(val_newChild, val_refChild);
}
v8::Handle<v8::Value> js_dom_Node::replaceChild(v8::Handle<v8::Value> val_newChild, v8::Handle<v8::Value> val_oldChild)
{
    return dom::Node::replaceChild(val_newChild, val_oldChild);
}
v8::Handle<v8::Value> js_dom_Node::removeChild(v8::Handle<v8::Value> val_oldChild)
{
    return dom::Node::removeChild(val_oldChild);
}
v8::Handle<v8::Value> js_dom_Node::appendChild(v8::Handle<v8::Value> val_newChild)
{
    HandleScope scope;
    v8_wrapper::tree_node* node_ptr = NULL;
    if(!val_newChild.IsEmpty() && val_newChild->IsObject()) {
        Local<External> wrap = Local<External>::Cast(val_newChild->ToObject()->GetInternalField(0));
        v8_wrapper::tree_node* node_ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
    }
    if(node_ptr) {
        m_child_list.push_back(node_ptr->shared_from_this());
        node_ptr->m_parent = shared_from_this();
        return node_ptr->m_this;
    }
    return v8::Handle<v8::Value>();
}
bool js_dom_Node::hasChildNodes()
{
    return !m_child_list.empty();
}
v8::Handle<v8::Value> js_dom_Node::cloneNode(bool val_deep)
{
    return dom::Node::cloneNode(val_deep);
}
void js_dom_Node::normalize()
{
    dom::Node::normalize();
}
bool js_dom_Node::isSupported(std::string val_feature, std::string val_version)
{
    return dom::Node::isSupported(val_feature, val_version);
}
bool js_dom_Node::hasAttributes()
{
    return dom::Node::hasAttributes();
}

short unsigned int js_dom_Node::compareDocumentPosition(v8::Handle<v8::Value> val_other)
{
    return dom::Node::compareDocumentPosition(val_other);
}
