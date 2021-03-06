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

#ifndef KASHYYYK_YYY_SERVER_UI_HPP
#define KASHYYYK_YYY_SERVER_UI_HPP
#pragma once

/*---------------------------------------------------------------------------*/

#include "yyy_channel_ui.hpp"
#include "yyy_maintainer.hpp"

#include "chat/yyy_chat_message.h"

#include "ui/yyy_server_tree.hpp"

#include "kashyyyk2.hpp"

#include <list>
#include <assert.h>

/*---------------------------------------------------------------------------*/

class Fl_Tree_Item;

/*---------------------------------------------------------------------------*/

class Fl_Tree;

/*---------------------------------------------------------------------------*/

namespace YYY {

/*---------------------------------------------------------------------------*/

class ServerCore;

/*---------------------------------------------------------------------------*/

class ServerUI {
    ServerTree::ServerData *m_ui_data;

    // Set as an enum to ensure inlining.
    template<enum ServerTree::ServerStatus status>
    void SetUINotificationLevel(){
        const int level = (int)(m_ui_data->status),
            target = (int)status;
        if(target > level ||
            (ServerTree::IsConnected(status) && !ServerTree::IsConnected(m_ui_data->status))){
            m_ui_data->status = status;
            server_tree->redraw();
            if(server_tree->isSelected(m_core.name()))
                server_tree->updateServerMenus();
        }
    }

public:
    
    // These functions will handle a click for the tree itself, and will call
    // the handlers for the appropriate ServerUI's.
    static void HandleTreeClick(Fl_Tree &tree);

    void handleMessage(const struct YYY_Message &msg);
    
    //! @brief Performs initial setup operations on the ui.
    void setup(const char *name, size_t name_len, ServerController &controller);
};

} // namespace YYY

#endif // KASHYYYK_YYY_SERVER_UI_HPP
