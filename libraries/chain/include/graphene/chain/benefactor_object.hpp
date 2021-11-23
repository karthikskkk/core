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
class database;

/**
  * @defgroup benefactor_types Implementations of the various benefactor types in the system
  *
  * The system has various benefactor types, which do different things with the money they are paid. These benefactor types
  * and their semantics are specified here.
  *
  * All benefactor types exist as a struct containing the data this benefactor needs to evaluate, as well as a method
  * pay_benefactor, which takes a pay amount and a non-const database reference, and applies the benefactor's specific pay
  * semantics to the benefactor_type struct and/or the database. Furthermore, all benefactor types have an initializer,
  * which is a struct containing the data needed to create that kind of benefactor.
  *
  * Each initializer type has a method, init, which takes a non-const database reference, a const reference to the
  * benefactor object being created, and a non-const reference to the specific *_benefactor_type object to initialize. The
  * init method creates any further objects, and initializes the benefactor_type object as necessary according to the
  * semantics of that particular benefactor type.
  *
  * To create a new benefactor type, define a my_new_benefactor_type struct with a pay_benefactor method which updates the
  * my_new_benefactor_type object and/or the database. Create a my_new_benefactor_type::initializer struct with an init
  * method and any data members necessary to create a new benefactor of this type. Reflect my_new_benefactor_type and
  * my_new_benefactor_type::initializer into FC's type system, and add them to @ref benefactor_type and @c
  * benefactor_initializer respectively. Make sure the order of types in @ref benefactor_type and @c benefactor_initializer
  * remains the same.
  * @{
  */
/**
 * @brief A benefactor who returns all of his pay to the reserve
 *
 * This benefactor type pays everything he receives back to the network's reserve funds pool.
 */
struct refund_benefactor_type
{
   /// Record of how much this benefactor has burned in his lifetime
   share_type total_burned;

   void pay_benefactor(share_type pay, database&);
};

/**
 * @brief A benefactor who sends his pay to a vesting balance
 *
 * This benefactor type takes all of his pay and places it into a vesting balance
 */
struct vesting_balance_benefactor_type
{
   /// The balance this benefactor pays into
   vesting_balance_id_type balance;

   void pay_benefactor(share_type pay, database& db);
};

/**
 * @brief A benefactor who permanently destroys all of his pay
 *
 * This benefactor sends all pay he receives to the null account.
 */
struct burn_benefactor_type
{
   /// Record of how much this benefactor has burned in his lifetime
   share_type total_burned;

   void pay_benefactor(share_type pay, database&);
};
///@}

// The ordering of types in these two static variants MUST be the same.
typedef static_variant<
   refund_benefactor_type,
   vesting_balance_benefactor_type,
   burn_benefactor_type
> benefactor_type;


/**
 * @brief Benefactor object contains the details of a blockchain benefactor. See @ref benefactors. for details.
 */
class benefactor_object : public abstract_object<benefactor_object>
{
   public:
      static constexpr uint8_t space_id = protocol_ids;
      static constexpr uint8_t type_id =  benefactor_object_type;

      /// ID of the account which owns this benefactor
      account_id_type benefactor_account;
      /// Time at which this benefactor begins receiving pay, if elected
      time_point_sec work_begin_date;
      /// Time at which this benefactor will cease to receive pay. Benefactor will be deleted at this time
      time_point_sec work_end_date;
      /// Amount in CORE this benefactor will be paid each day
      share_type daily_pay;
      /// ID of this benefactor's pay balance
      benefactor_type benefactor;
      /// Human-readable name for the benefactor
      string name;
      /// URL to a web page representing this benefactor
      string url;

      /// Voting ID which represents approval of this benefactor
      vote_id_type vote_for;
      /// Voting ID which represents disapproval of this benefactor
      vote_id_type vote_against;

      uint64_t total_votes_for = 0;
      uint64_t total_votes_against = 0;

      bool is_active(fc::time_point_sec now)const {
         return now >= work_begin_date && now <= work_end_date;
      }

      share_type approving_stake()const {
         return int64_t( total_votes_for ) - int64_t( total_votes_against );
      }
};

struct by_account;
struct by_vote_for;
struct by_vote_against;
struct by_end_date;
typedef multi_index_container<
   benefactor_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
      ordered_non_unique< tag<by_account>, member< benefactor_object, account_id_type, &benefactor_object::benefactor_account > >,
      ordered_unique< tag<by_vote_for>, member< benefactor_object, vote_id_type, &benefactor_object::vote_for > >,
      ordered_unique< tag<by_vote_against>, member< benefactor_object, vote_id_type, &benefactor_object::vote_against > >,
      ordered_non_unique< tag<by_end_date>, member< benefactor_object, time_point_sec, &benefactor_object::work_end_date> >
   >
> benefactor_object_multi_index_type;

using benefactor_index = generic_index<benefactor_object, benefactor_object_multi_index_type>;

} } // graphene::chain

MAP_OBJECT_ID_TO_TYPE(graphene::chain::benefactor_object)

FC_REFLECT_TYPENAME( graphene::chain::refund_benefactor_type )
FC_REFLECT_TYPENAME( graphene::chain::vesting_balance_benefactor_type )
FC_REFLECT_TYPENAME( graphene::chain::burn_benefactor_type )
FC_REFLECT_TYPENAME( graphene::chain::benefactor_type )
FC_REFLECT_TYPENAME( graphene::chain::benefactor_object )

GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::chain::benefactor_object )
