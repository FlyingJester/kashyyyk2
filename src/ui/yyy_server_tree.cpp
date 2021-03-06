/* 
 *  Copyright (c) 2014-2017 Martin McDonough.  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 * 
 * - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 
 * - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 * - Products derived from this software may not be called "Kashyyyk", nor may
 *     "YYY" appear in their name, without prior written permission of
 *     the copyright holders.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
/*---------------------------------------------------------------------------*/

#include "yyy_server_tree.hpp"

#include "kashyyyk2.hpp"

#include "utils/yyy_alloca.h"

#include <algorithm>
#include <iterator>

#include <string.h>
#include <assert.h>

#ifndef YYY_FASTCALL
#if (defined _MSC_VER) || (defined __WATCOMC__)
#define YYY_FASTCALL __fastcall
#else
#define YYY_FASTCALL
#endif
#endif // YYY_FASTCALL

#ifdef __GNUC__

#include <alloca.h>
#define YYY_HAS_ALLOCA 1

#elif (defined __WATCOMC__) || (defined _MSC_VER)

#include <malloc.h>
#define YYY_HAS_ALLOCA 1

#else
// Do someting here if alloca isn't defined!
#endif

#ifdef YYY_HAS_ALLOCA
#define YYY_ALLOCA alloca
#define YYY_ALLOCA_FREE(X) ((void)(X))
#else
#define YYY_ALLOCA malloc
#define YYY_ALLOCA_FREE free
#endif

namespace YYY {

/*---------------------------------------------------------------------------*/

Fl_Menu_Item ServerTree::s_right_click_menu[9] = {
    {"Disconnect"},
    {"Reconnect"},
    {"Join on Startup", 0, NULL, NULL, FL_MENU_INACTIVE|FL_MENU_TOGGLE},
    {"Join...", 0, NULL, NULL, FL_MENU_DIVIDER},
    {"Remove", 0, NULL, NULL, FL_MENU_INACTIVE},
    {"Properties"},
    {NULL},
    {NULL},
    {NULL}
};

#define YYY_DISCONNECT 0
#define YYY_RECONNECT 2
#define YYY_JOIN 3
#define YYY_JOIN_ON_STARTUP 4
#define YYY_PROPERTIES 5

/*---------------------------------------------------------------------------*/

ServerTree::ServerTree(int a_x, int a_y, int a_w, int a_h, const char *a_title)
  : Fl_Tree(a_x, a_y, a_w, a_h, a_title)
  , m_last_clicked(NULL)
  , m_flash_tick(false)
  , m_num_connecting(0){
    
    selectmode(FL_TREE_SELECT_SINGLE);
    showroot(0);
    
}

/*---------------------------------------------------------------------------*/
    
struct ServerTree::ServerData *ServerTree::addConnectingServer(const char *uri,
    const char *name, unsigned name_len){
    
    if(m_num_connecting++ == 0)
        Fl::add_timeout(0.5, TimeoutCallback, this);
    
    // Store the URI of the server
    Fl_Tree_Item *const item = add(root(), std::string(name, name_len).c_str());
    
    const unsigned uri_len = strlen(uri);
    ServerData *const data = (ServerData *)malloc(sizeof(ServerData *) + uri_len);
    data->arg = NULL;
    data->status = eConnecting;
    memcpy(const_cast<char*>(data->uri), uri, uri_len);
    item->user_data(data);
    
    // Refresh the view
    updateChildren();

    return data;
}

/*---------------------------------------------------------------------------*/

void ServerTree::setSelected(Fl_Tree_Item *item){
    Fl_Tree::select_only(item);
    if(item != NULL){
        ServerData *const server_data = (ServerData*)item->user_data();
        if(server_data->arg == NULL)
            YYY_SetNoServer();
        else if(IsConnected(server_data))
            YYY_SetServerConnected();
        else
            YYY_SetServerDisconnected();
    }
}

/*---------------------------------------------------------------------------*/

int ServerTree::handle(int e){

    if(children() == 0)
        YYY_SetNoServer();

    // Check that the last clicked item is still in the tree.
    bool still_exists = false;
    if(m_last_clicked != NULL){
        for(Fl_Tree_Item *item = first(); item; item = next(item)){
            if(item == m_last_clicked){
                still_exists = true;
                break;
            }
        }
    }

    if(m_last_clicked == NULL || !still_exists){
        m_last_clicked = first();
        if(m_last_clicked == root())
            m_last_clicked = next(m_last_clicked);
        select_only(m_last_clicked);
    }
    if(e == FL_PUSH){
        puts("Push!");
        if(Fl::event_button() == FL_RIGHT_MOUSE){
        
            Fl_Tree_Item *const l_item = find_clicked();
            if(l_item == NULL)
                return Fl_Tree::handle(e);
        
            const unsigned l_x = Fl::event_x(), l_y = Fl::event_y();
            const Fl_Menu_Item *l_menu_item =
                s_right_click_menu->popup(l_x, l_y, NULL, 0, 0);
            if(l_menu_item == NULL)
                return Fl_Tree::handle(e);
            /*
            const unsigned l_index =
                std::distance(static_cast<const Fl_Menu_Item *>(s_right_click_menu),
                    l_menu_item);
            */
            return 1;
        }
    }
    
    // Update the m_last_clicked.
    m_last_clicked = first_selected_item();
    if(m_last_clicked == root())
        m_last_clicked = next(m_last_clicked);

    // Do the default handle
    const int ret = Fl_Tree::handle(e);

    // If we deselected everything, reselect the last item.
    if(first_selected_item() == NULL)
        select_only(m_last_clicked);
    return ret;
}

/*---------------------------------------------------------------------------*/

ServerTree::ServerData *ServerTree::getData(const std::string &server_name){
    Fl_Tree_Item *const l_root = root();
    const unsigned num = l_root->children();
    for(unsigned i = 0; i < num; i++){
        Fl_Tree_Item *const l_child = l_root->child(i);
        if(l_child->label() == server_name)
            return static_cast<ServerTree::ServerData*>(l_child->user_data());
    }
    
    return NULL;
}

/*---------------------------------------------------------------------------*/

ServerTree::ServerData *ServerTree::getData(const char *name, unsigned name_len){
    Fl_Tree_Item *const l_root = root();
    const unsigned num = l_root->children();
    for(unsigned i = 0; i < num; i++){
        Fl_Tree_Item *const l_child = l_root->child(i);
        const char *const l_label = l_child->label();
        if(strncmp(l_label, name, name_len) == 0 && l_label[name_len] == '\0')
            return static_cast<ServerTree::ServerData*>(l_child->user_data());
    }
    
    return NULL;
}

/*---------------------------------------------------------------------------*/

ServerTree::ServerData *ServerTree::getSelectedServer(){
    
    // If nothing is selected, do nothing. That shouldn't happen, though.
    // It is possible this happens on some old-ish versions of FLTK when there is nothing in the
    // browser yet.
    if(Fl_Tree_Item *l_selected = first_selected_item()){
        // Determine the server that selected represents.
        {
            const Fl_Tree_Item *const l_root = ServerTree::root();
            assert(l_root != NULL);
            
            // This probably shouldn't have happened either, but it is possible in early FLTK 1.3
            if(l_selected == l_root)
                return NULL;

            {
                // Check if the parent is the child of root. That means this item is a server.
                Fl_Tree_Item *const l_parent = l_selected->parent();
                if(l_parent != l_root){
                    // Otherwise, the parent of this item should be a server.
                    l_selected = l_parent->parent();
                }
            }

            assert(l_selected->parent() == l_root);
        }
        return static_cast<ServerData*>(l_selected->user_data());
    }
    else{
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

ServerTree::ChannelData *ServerTree::getSelectedChannel(ServerData **out_server_data){
    
    // If nothing is selected, do nothing. That shouldn't happen, though.
    // It is possible this happens on some old-ish versions of FLTK when there is nothing in the
    // browser yet.
    if(Fl_Tree_Item *l_selected = first_selected_item()){
        // Determine the server that selected represents.
        {
            const Fl_Tree_Item *const l_root = ServerTree::root();
            assert(l_root != NULL);

            // This probably shouldn't have happened either, but it is possible in early FLTK 1.3
            if(l_selected == l_root)
                return NULL;

            {
                // Check if the parent is the child of root. That means this item is a server.
                Fl_Tree_Item *const l_parent = l_selected->parent();
                if(l_parent != l_root){

                    // The parent (which is a server) should have the root as a parent.
                    assert(l_parent->parent() == l_root);
                    if(out_server_data != NULL)
                        out_server_data[0] = static_cast<ServerData*>(l_parent->user_data());
                    return static_cast<ChannelData*>(l_selected->user_data());
                }
            }

            // The selected is a server then.
            assert(l_selected->parent() == l_root);
        }
        if(out_server_data != NULL)
            out_server_data[0] = static_cast<ServerData*>(l_selected->user_data());
        return NULL;
    }
    else{
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

ServerTree::ChannelData *ServerTree::addChannel(const char *server,
    unsigned server_len,
    const char *channel,
    unsigned channel_len,
    ChannelController *user_data){
    
    // Store the URI of the server
    
    char *const path = (char*)YYY_ALLOCA(server_len + 1 + channel_len + 1);
    memcpy(path, server, server_len);
    path[server_len] = '/';
    memcpy(path + server_len + 1, channel, channel_len);
    path[server_len + 1 + channel_len] = 0;

    Fl_Tree_Item *const item = add(path);

    YYY_ALLOCA_FREE(path);
    
    ChannelData *const data = new ChannelData();
    data->arg = user_data;
    data->status = eConnected;
    item->user_data(data);
    
    // Refresh the view
    updateChildren();

    return data;
}

/*---------------------------------------------------------------------------*/

ServerTree::ServerStatus ServerTree::getServerStatus(const std::string &server_name) const{
    const ServerTree::ServerData *const data =
        const_cast<ServerTree*>(this)->getData(server_name);
    if(data)
        return data->status;
    else
        return eFailed;
}

/*---------------------------------------------------------------------------*/

ServerTree::ServerStatus ServerTree::getServerStatus(const char *name, unsigned name_len) const{
    const ServerTree::ServerData *const data =
        const_cast<ServerTree*>(this)->getData(name, name_len);
    if(data)
        return data->status;
    else
        return eFailed;
}

/*---------------------------------------------------------------------------*/

// Note that Fl::lock should be called for multithreaded access to this.
ServerTree::ServerData *ServerTree::setServerStatus(const std::string &server_name,
    ServerTree::ServerStatus status,
    ServerController *arg){
    
    ServerTree::ServerData *const data = getData(server_name);
    if(data){
        data->status = status;
        data->arg = arg;
    }
    return data;
}

/*---------------------------------------------------------------------------*/

ServerTree::ServerData *ServerTree::setServerStatus(const char *name,
    unsigned name_len,
    ServerTree::ServerStatus status,
    ServerController *arg){

    ServerTree::ServerData *const data = getData(name, name_len);
    if(data){
        data->status = status;
        data->arg = arg;
    }
    return data;
}

/*---------------------------------------------------------------------------*/
// Fastcall forces flash to be on the stack, which for Watcom allows it to be
// put in the same stack slot for the entire body of updateChildren.
static void YYY_FASTCALL yyy_apply_item_color(Fl_Tree_Item *const item,
    ServerTree::ServerStatus s,
    bool flash){
    
    switch(s){
        case ServerTree::eConnected:
            item->labelcolor(FL_FOREGROUND_COLOR);
            return;
        case ServerTree::eMetaUpdate:
            item->labelcolor(FL_GREEN);
            return;
        case ServerTree::eUpdate:
            item->labelcolor(FL_BLUE);
            return;
        case ServerTree::eNotify:
            item->labelcolor(FL_RED);
            return;
        case ServerTree::eConnecting:
            item->labelcolor(flash ? FL_YELLOW : FL_DARK2);
            return;
        case ServerTree::eFailed:
            item->labelcolor(FL_DARK2);
            {
                const char *const l = item->label();
                const unsigned l_size = strlen(l);
                
                // HACK
                if(l_size && l[l_size - 1] == ')')
                    return;
                
                // Use an array to get sizeof() support
                const char suffix[] = " (OFFLINE)";
                char *const buffer = (char*)YYY_ALLOCA(l_size + sizeof(suffix));
                // No need for a NULL char yet since we have the size.
                memcpy(buffer, l, l_size);
                // We also get the NULL char from the end of suffix with this.
                memcpy(buffer + l_size, suffix, sizeof(suffix));
                item->label(buffer);
                YYY_ALLOCA_FREE(buffer);
            }
            return;
    }
    return;
}

/*---------------------------------------------------------------------------*/

void ServerTree::updateChildren(){
    
    assert(root() != NULL);
    Fl_Tree_Item &l_root = *root();
    const unsigned num_servers = l_root.children();
    
    for(unsigned i = 0; i < num_servers; i++){
        Fl_Tree_Item *const l_server = l_root.child(i);
        const unsigned num_channels = l_server->children();
        
        ServerTree::ServerData *const l_data =
            static_cast<ServerTree::ServerData *>(l_server->user_data());
        
        int s = (int)l_data->status;
        
        if(s <= (int)eChannelEnd){
            s = 0;
            for(unsigned e = 0; e < num_channels; e++){
                Fl_Tree_Item *const l_channel = l_server->child(e);
                assert(l_channel->children() == 0);
                const int other_s =
                    static_cast<ChannelData*>(l_channel->user_data())->status;
                assert(other_s <= (int)eChannelEnd);
                if(other_s > s)
                    s = other_s;
                yyy_apply_item_color(l_channel, (ServerTree::ServerStatus)other_s, false);
            }
        }
        l_data->status = (ServerTree::ServerStatus)s;
        yyy_apply_item_color(l_server, (ServerTree::ServerStatus)s, m_flash_tick);
    }
}

/*---------------------------------------------------------------------------*/

bool ServerTree::isSelected(const ServerController &controller, bool channel_only) const{
    if(const Fl_Tree_Item *const selected = getSelected()){
        const Fl_Tree_Item *server = NULL;

        // If the selected item is a server, then use that as the check. Otherwise, if channel_only
        // is set then we will check against the selected item's parent (server).
        if(selected->parent() == root()){
            server = selected;
        }
        else if(!channel_only){
            server = selected->parent();
        }
        else{
            return false;
        }
        
        const ServerData *const server_data = (ServerData*)server->user_data();
        return server_data->arg == &controller;
    }
    
    return false;
}

/*---------------------------------------------------------------------------*/

bool ServerTree::isServerSelected(const std::string &server_name) const {
    if(const Fl_Tree_Item *const selected = getSelected()){
        if(selected->parent() == root()){
            return server_name == selected->label();
        }
    }
    return false;
}

/*---------------------------------------------------------------------------*/

bool ServerTree::isSelected(const ChannelController &controller) const{
    if(const Fl_Tree_Item *const selected = getSelected()){
        if(selected->parent() == root()){
            return false;
        }
        else{
            const ChannelData *const channel_data = (ChannelData*)selected->user_data();
            return channel_data->arg == &controller;
        }
    }
    
    return false;
}

} // namespace YYY
