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
#include <graphene/chain/database.hpp>

#include <fc/thread/future.hpp>

namespace graphene { namespace blockproducer_plugin {

namespace block_production_condition
{
   enum block_production_condition_enum
   {
      produced = 0,
      not_synced = 1,
      not_my_turn = 2,
      not_time_yet = 3,
      no_private_key = 4,
      low_participation = 5,
      lag = 6,
      exception_producing_block = 7,
      shutdown = 8,
      no_network = 9
   };
}

class blockproducer_plugin : public graphene::app::plugin {
public:
   using graphene::app::plugin::plugin;
   ~blockproducer_plugin() override { cleanup(); }

   std::string plugin_name()const override;

   void plugin_set_program_options(
      boost::program_options::options_description &command_line_options,
      boost::program_options::options_description &config_file_options
      ) override;

   void set_block_production(bool allow) { _production_enabled = allow; }
   void stop_block_production();

   void plugin_initialize( const boost::program_options::variables_map& options ) override;
   void plugin_startup() override;
   void plugin_shutdown() override { cleanup(); }

   inline const fc::flat_map< chain::blockproducer_id_type, fc::optional<chain::public_key_type> >& get_blockproducer_key_cache()
   { return _blockproducer_key_cache; }

private:
   void cleanup() { stop_block_production(); }

   void schedule_production_loop();
   block_production_condition::block_production_condition_enum block_production_loop();
   block_production_condition::block_production_condition_enum maybe_produce_block( fc::limited_mutable_variant_object& capture );
   void add_private_key(const std::string& key_id_to_wif_pair_string);

   /// Fetch signing keys of all blockproducers in the cache from object database and update the cache accordingly
   void refresh_blockproducer_key_cache();

   boost::program_options::variables_map _options;
   bool _production_enabled = false;
   bool _shutting_down = false;
   uint32_t _required_blockproducer_participation = 33 * GRAPHENE_1_PERCENT;
   uint32_t _production_skip_flags = graphene::chain::database::skip_nothing;

   std::map<chain::public_key_type, fc::ecc::private_key, chain::pubkey_comparator> _private_keys;
   std::set<chain::blockproducer_id_type> _blockproducers;
   fc::future<void> _block_production_task;

   /// For tracking signing keys of specified blockproducers, only update when applied a block
   fc::flat_map< chain::blockproducer_id_type, fc::optional<chain::public_key_type> > _blockproducer_key_cache;

};

} } //graphene::blockproducer_plugin
