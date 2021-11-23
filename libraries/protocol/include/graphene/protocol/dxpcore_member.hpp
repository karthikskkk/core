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
#include <graphene/protocol/base.hpp>
#include <graphene/protocol/asset.hpp>
#include <graphene/protocol/chain_parameters.hpp>

namespace graphene { namespace protocol { 

   /**
    * @brief Create a dxpcore_member object, as a bid to hold a dxpcore_member seat on the network.
    * @ingroup operations
    *
    * Accounts which wish to become dxpcore_members may use this operation to create a dxpcore_member object which stakeholders may
    * vote on to approve its position as a dxpcore_member.
    */
   struct dxpcore_member_create_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 5000 * GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset                                 fee;
      /// The account which owns the dxpcore_member. This account pays the fee for this operation.
      account_id_type                       dxpcore_member_account;
      string                                url;

      account_id_type fee_payer()const { return dxpcore_member_account; }
      void            validate()const;
   };

   /**
    * @brief Update a dxpcore_member object.
    * @ingroup operations
    *
    * Currently the only field which can be updated is the `url`
    * field.
    */
   struct dxpcore_member_update_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset                                 fee;
      /// The dxpcore member to update.
      dxpcore_member_id_type              dxpcore_member;
      /// The account which owns the dxpcore_member. This account pays the fee for this operation.
      account_id_type                       dxpcore_member_account;
      optional< string >                    new_url;

      account_id_type fee_payer()const { return dxpcore_member_account; }
      void            validate()const;
   };

   /**
    * @brief Used by dxpcore_members to update the global parameters of the blockchain.
    * @ingroup operations
    *
    * This operation allows the dxpcore_members to update the global parameters on the blockchain. These control various
    * tunable aspects of the chain, including block and maintenance intervals, maximum data sizes, the fees charged by
    * the network, etc.
    *
    * This operation may only be used in a proposed transaction, and a proposed transaction which contains this
    * operation must have a review period specified in the current global parameters before it may be accepted.
    */
   struct dxpcore_member_update_global_parameters_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset             fee;
      chain_parameters  new_parameters;

      account_id_type fee_payer()const { return account_id_type(); }
      void            validate()const;
   };

   /// TODO: dxpcore_member_resign_operation : public base_operation

} } // graphene::protocol

FC_REFLECT( graphene::protocol::dxpcore_member_create_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::protocol::dxpcore_member_update_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::protocol::dxpcore_member_update_global_parameters_operation::fee_parameters_type, (fee) )

FC_REFLECT( graphene::protocol::dxpcore_member_create_operation,
            (fee)(dxpcore_member_account)(url) )
FC_REFLECT( graphene::protocol::dxpcore_member_update_operation,
            (fee)(dxpcore_member)(dxpcore_member_account)(new_url) )
FC_REFLECT( graphene::protocol::dxpcore_member_update_global_parameters_operation, (fee)(new_parameters) )

GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::dxpcore_member_create_operation::fee_parameters_type )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::dxpcore_member_update_operation::fee_parameters_type )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::dxpcore_member_update_global_parameters_operation::fee_parameters_type )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::dxpcore_member_create_operation )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::dxpcore_member_update_operation )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::dxpcore_member_update_global_parameters_operation )
