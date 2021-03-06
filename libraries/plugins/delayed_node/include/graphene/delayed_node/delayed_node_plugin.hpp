/*
 * Copyright (c) 2021 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <graphene/app/plugin.hpp>

namespace graphene { namespace delayed_node {
namespace detail { struct delayed_node_plugin_impl; }

class delayed_node_plugin : public graphene::app::plugin
{
   std::unique_ptr<detail::delayed_node_plugin_impl> my;
public:
   explicit delayed_node_plugin(graphene::app::application& app);
   ~delayed_node_plugin() override;

   std::string plugin_name()const override { return "delayed_node"; }
   void plugin_set_program_options(boost::program_options::options_description&,
                                   boost::program_options::options_description& cfg) override;
   void plugin_initialize(const boost::program_options::variables_map& options) override;
   void plugin_startup() override;
   void mainloop();

protected:
   void connection_failed();
   void connect();
   void sync_with_trusted_node();
};

} } //graphene::account_history

