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
    * @defgroup benefactors The Blockchain Benefactor System
    * @ingroup operations
    *
    * Dxperts blockchains allow the creation of special "benefactors" which are elected positions paid by the blockchain
    * for services they provide. There may be several types of benefactors, and the semantics of how and when they are paid
    * are defined by the @ref graphene::chain::benefactor_type enumeration.
    * All benefactors are elected by core stakeholder approval, by
    * voting for or against them.
    *
    * Benefactors are paid from the blockchain's daily budget if their total approval (votes for - votes against) is
    * positive, ordered from most positive approval to least, until the budget is exhausted. Payments are processed at
    * the blockchain maintenance interval. If a benefactor does not have positive approval during payment processing, or if
    * the chain's budget is exhausted before the benefactor is paid, that benefactor is simply not paid at that interval.
    * Payment is not prorated based on percentage of the interval the benefactor was approved. If the chain attempts to pay
    * a benefactor, but the budget is insufficient to cover its entire pay, the benefactor is paid the remaining budget funds,
    * even though this does not fulfill his total pay. The benefactor will not receive extra pay to make up the difference
    * later. Benefactor pay is placed in a vesting balance and vests over the number of days specified at the benefactor's
    * creation.
    *
    * Once created, a benefactor is immutable and will be kept by the blockchain forever.
    *
    * @{
    */


   struct vesting_balance_benefactor_initializer
   {
      vesting_balance_benefactor_initializer(uint16_t days=0):pay_vesting_period_days(days){}
      uint16_t pay_vesting_period_days = 0;
   };

   struct burn_benefactor_initializer
   {};

   struct refund_benefactor_initializer
   {};


   typedef static_variant< 
      refund_benefactor_initializer,
      vesting_balance_benefactor_initializer,
      burn_benefactor_initializer > benefactor_initializer;


   /**
    * @brief Create a new benefactor object
    * @ingroup operations
    */
   struct benefactor_create_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 5000*GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset                fee;
      account_id_type      owner;
      time_point_sec       work_begin_date;
      time_point_sec       work_end_date;
      share_type           daily_pay;
      string               name;
      string               url;
      /// This should be set to the initializer appropriate for the type of benefactor to be created.
      benefactor_initializer   initializer;

      account_id_type   fee_payer()const { return owner; }
      void              validate()const;
   };
   ///@}

} }

FC_REFLECT( graphene::protocol::vesting_balance_benefactor_initializer, (pay_vesting_period_days) )
FC_REFLECT( graphene::protocol::burn_benefactor_initializer, )
FC_REFLECT( graphene::protocol::refund_benefactor_initializer, )
FC_REFLECT_TYPENAME( graphene::protocol::benefactor_initializer )

FC_REFLECT( graphene::protocol::benefactor_create_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::protocol::benefactor_create_operation,
            (fee)(owner)(work_begin_date)(work_end_date)(daily_pay)(name)(url)(initializer) )

GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::benefactor_create_operation::fee_parameters_type )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::protocol::benefactor_create_operation )
