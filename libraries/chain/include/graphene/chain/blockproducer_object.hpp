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

#include <graphene/protocol/asset.hpp>
#include <graphene/db/generic_index.hpp>

namespace graphene { namespace chain {
   using namespace graphene::db;

   class blockproducer_object : public abstract_object<blockproducer_object>
   {
      public:
         static constexpr uint8_t space_id = protocol_ids;
         static constexpr uint8_t type_id = blockproducer_object_type;

         account_id_type  blockproducer_account;
         uint64_t         last_aslot = 0;
         public_key_type  signing_key;
         optional< vesting_balance_id_type > pay_vb;
         vote_id_type     vote_id;
         uint64_t         total_votes = 0;
         string           url;
         int64_t          total_missed = 0;
         uint32_t         last_confirmed_block_num = 0;

         blockproducer_object() : vote_id(vote_id_type::blockproducer) {}
   };

   struct by_account;
   struct by_vote_id;
   struct by_last_block;
   using blockproducer_multi_index_type = multi_index_container<
      blockproducer_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_unique< tag<by_account>,
            member<blockproducer_object, account_id_type, &blockproducer_object::blockproducer_account>
         >,
         ordered_unique< tag<by_vote_id>,
            member<blockproducer_object, vote_id_type, &blockproducer_object::vote_id>
         >
      >
   >;
   using blockproducer_index = generic_index<blockproducer_object, blockproducer_multi_index_type>;
} } // graphene::chain

MAP_OBJECT_ID_TO_TYPE(graphene::chain::blockproducer_object)

FC_REFLECT_TYPENAME( graphene::chain::blockproducer_object )

GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::chain::blockproducer_object )
