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
#include <graphene/chain/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/protocol/vote.hpp>

namespace graphene { namespace chain {
   using namespace graphene::db;

   /**
    *  @brief tracks information about a dxpcore_member account.
    *  @ingroup object
    *
    *  A dxpcore_member is responsible for setting blockchain parameters and has
    *  dynamic multi-sig control over the dxpcore account.  The current set of
    *  active dxpcore_members has control.
    *
    *  dxpcore_members were separated into a separate object to make iterating over
    *  the set of dxpcore_member easy.
    */
   class dxpcore_member_object : public abstract_object<dxpcore_member_object>
   {
      public:
         static constexpr uint8_t space_id = protocol_ids;
         static constexpr uint8_t type_id  = dxpcore_member_object_type;

         account_id_type  dxpcore_member_account;
         vote_id_type     vote_id;
         uint64_t         total_votes = 0;
         string           url;
   };

   struct by_account;
   struct by_vote_id;
   using dxpcore_member_multi_index_type = multi_index_container<
      dxpcore_member_object,
      indexed_by<
         ordered_unique< tag<by_id>,
            member<object, object_id_type, &object::id>
         >,
         ordered_unique< tag<by_account>,
            member<dxpcore_member_object, account_id_type, &dxpcore_member_object::dxpcore_member_account>
         >,
         ordered_unique< tag<by_vote_id>,
            member<dxpcore_member_object, vote_id_type, &dxpcore_member_object::vote_id>
         >
      >
   >;
   using dxpcore_member_index = generic_index<dxpcore_member_object, dxpcore_member_multi_index_type>;
} } // graphene::chain

MAP_OBJECT_ID_TO_TYPE(graphene::chain::dxpcore_member_object)

FC_REFLECT_TYPENAME( graphene::chain::dxpcore_member_object )

GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::chain::dxpcore_member_object )
