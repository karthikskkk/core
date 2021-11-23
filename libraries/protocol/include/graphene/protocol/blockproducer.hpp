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

namespace graphene { namespace protocol { 

  /**
    * @brief Create a blockproducer object, as a bid to hold a blockproducer position on the network.
    * @ingroup operations
    *
    * Accounts which wish to become blockproducers may use this operation to create a blockproducer object which stakeholders may
    * vote on to approve its position as a blockproducer.
    */
   struct blockproducer_create_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 5000 * GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset             fee;
      /// The account which owns the blockproducer. This account pays the fee for this operation.
      account_id_type   blockproducer_account;
      string            url;
      public_key_type   block_signing_key;

      account_id_type fee_payer()const { return blockproducer_account; }
      void            validate()const;
   };

  /**
    * @brief Update a blockproducer object's URL and block signing key.
    * @ingroup operations
    */
   struct blockproducer_update_operation : public base_operation
   {
      struct fee_parameters_type
      {
         share_type fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
      };

      asset             fee;
      /// The blockproducer object to update.
      blockproducer_id_type   blockproducer;
      /// The account which owns the blockproducer. This account pays the fee for this operation.
      account_id_type   blockproducer_account;
      /// The new URL.
      optional< string > new_url;
      /// The new block signing key.
      optional< public_key_type > new_signing_key;

      account_id_type fee_payer()const { return blockproducer_account; }
      void            validate()const;
   };

   /// TODO: blockproducer_resign_operation : public base_operation

} } // graphene::protocol

FC_REFLECT( graphene::protocol::blockproducer_create_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::protocol::blockproducer_create_operation, (fee)(blockproducer_account)(url)(block_signing_key) )

FC_REFLECT( graphene::protocol::blockproducer_update_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::protocol::blockproducer_update_operation, (fee)(blockproducer)(blockproducer_account)(new_url)(new_signing_key) )

GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::blockproducer_create_operation::fee_parameters_type )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::blockproducer_update_operation::fee_parameters_type )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::blockproducer_create_operation )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::blockproducer_update_operation )
