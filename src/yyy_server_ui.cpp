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

#include "yyy_server_ui.hpp"

#include "yyy_server_core.hpp"

#ifdef __WATCOMC__
// Silences warnings about unary operator
#include <FL/Fl_Image.H>
#endif

#include "utils/yyy_alloca.h"
#include "utils/yyy_fl_locker.hpp"

#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Tree.H>

#include <assert.h>

/*---------------------------------------------------------------------------*/

namespace YYY {


void ServerUI::handleMessage(const struct YYY_Message &msg){

}

void ServerUI::setup(const char *name, size_t name_len, ServerController &controller){
    
    {
        FlLocker locker;
        m_ui_data = server_tree->connectionSucceeded(name, name_len, &controller);
        server_tree->redraw();
    }
}

} // namespace YYY
