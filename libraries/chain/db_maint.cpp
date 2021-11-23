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

#include <fc/uint128.hpp>

#include <graphene/protocol/market.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/fba_accumulator_id.hpp>
#include <graphene/chain/hardfork.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/balance_object.hpp>
#include <graphene/chain/budget_record_object.hpp>
#include <graphene/chain/buyback_object.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/dxpcore_member_object.hpp>
#include <graphene/chain/fba_object.hpp>
#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/special_authority_object.hpp>
#include <graphene/chain/ticket_object.hpp>
#include <graphene/chain/vesting_balance_object.hpp>
#include <graphene/chain/vote_count.hpp>
#include <graphene/chain/blockproducer_object.hpp>
#include <graphene/chain/benefactor_object.hpp>
#include <graphene/chain/custom_authority_object.hpp>

namespace graphene { namespace chain {

template<class Index>
vector<std::reference_wrapper<const typename Index::object_type>> database::sort_votable_objects(size_t count) const
{
   using ObjectType = typename Index::object_type;
   const auto& all_objects = get_index_type<Index>().indices();
   count = std::min(count, all_objects.size());
   vector<std::reference_wrapper<const ObjectType>> refs;
   refs.reserve(all_objects.size());
   std::transform(all_objects.begin(), all_objects.end(),
                  std::back_inserter(refs),
                  [](const ObjectType& o) { return std::cref(o); });
   std::partial_sort(refs.begin(), refs.begin() + count, refs.end(),
                   [this](const ObjectType& a, const ObjectType& b)->bool {
      share_type oa_vote = _vote_tally_buffer[a.vote_id];
      share_type ob_vote = _vote_tally_buffer[b.vote_id];
      if( oa_vote != ob_vote )
         return oa_vote > ob_vote;
      return a.vote_id < b.vote_id;
   });

   refs.resize(count, refs.front());
   return refs;
}

template<class Type>
void database::perform_account_maintenance(Type tally_helper)
{
   const auto& bal_idx = get_index_type< account_balance_index >().indices().get< by_maintenance_flag >();
   if( bal_idx.begin() != bal_idx.end() )
   {
      auto bal_itr = bal_idx.rbegin();
      while( bal_itr->maintenance_flag )
      {
         const account_balance_object& bal_obj = *bal_itr;

         modify( get_account_stats_by_owner( bal_obj.owner ), [&bal_obj](account_statistics_object& aso) {
            aso.core_in_balance = bal_obj.balance;
         });

         modify( bal_obj, []( account_balance_object& abo ) {
            abo.maintenance_flag = false;
         });

         bal_itr = bal_idx.rbegin();
      }
   }

   const auto& stats_idx = get_index_type< account_stats_index >().indices().get< by_maintenance_seq >();
   auto stats_itr = stats_idx.lower_bound( true );

   while( stats_itr != stats_idx.end() )
   {
      const account_statistics_object& acc_stat = *stats_itr;
      const account_object& acc_obj = acc_stat.owner( *this );
      ++stats_itr;

      if( acc_stat.has_some_core_voting() )
         tally_helper( acc_obj, acc_stat );

      if( acc_stat.has_pending_fees() )
         acc_stat.process_fees( acc_obj, *this );
   }

}

/// @brief A visitor for @ref benefactor_type which calls pay_benefactor on the benefactor within
struct benefactor_pay_visitor
{
   private:
      share_type pay;
      database& db;

   public:
      benefactor_pay_visitor(share_type pay, database& db)
         : pay(pay), db(db) {}

      typedef void result_type;
      template<typename W>
      void operator()(W& benefactor)const
      {
         benefactor.pay_benefactor(pay, db);
      }
};

void database::update_benefactor_votes()
{
   const auto& idx = get_index_type<benefactor_index>().indices().get<by_account>();
   auto itr = idx.begin();
   auto itr_end = idx.end();
   bool allow_negative_votes = (head_block_time() < HARDFORK_607_TIME);
   while( itr != itr_end )
   {
      modify( *itr, [this,allow_negative_votes]( benefactor_object& obj )
      {
         obj.total_votes_for = _vote_tally_buffer[obj.vote_for];
         obj.total_votes_against = allow_negative_votes ? _vote_tally_buffer[obj.vote_against] : 0;
      });
      ++itr;
   }
}

void database::pay_benefactors( share_type& budget )
{
   const auto head_time = head_block_time();
//   ilog("Processing payroll! Available budget is ${b}", ("b", budget));
   vector<std::reference_wrapper<const benefactor_object>> active_benefactors;
   // TODO optimization: add by_expiration index to avoid iterating through all objects
   get_index_type<benefactor_index>().inspect_all_objects([head_time, &active_benefactors](const object& o) {
      const benefactor_object& w = static_cast<const benefactor_object&>(o);
      if( w.is_active(head_time) && w.approving_stake() > 0 )
         active_benefactors.emplace_back(w);
   });

   // benefactor with more votes is preferred
   // if two benefactors. exactly tie for votes, benefactor with lower ID is preferred
   std::sort(active_benefactors.begin(), active_benefactors.end(), [](const benefactor_object& wa, const benefactor_object& wb) {
      share_type wa_vote = wa.approving_stake();
      share_type wb_vote = wb.approving_stake();
      if( wa_vote != wb_vote )
         return wa_vote > wb_vote;
      return wa.id < wb.id;
   });

   const auto last_budget_time = get_dynamic_global_properties().last_budget_time;
   const auto passed_time_ms = head_time - last_budget_time;
   const auto passed_time_count = passed_time_ms.count();
   const auto day_count = fc::days(1).count();
   for( uint32_t i = 0; i < active_benefactors.size() && budget > 0; ++i )
   {
      const benefactor_object& active_benefactor = active_benefactors[i];
      share_type requested_pay = active_benefactor.daily_pay;

      // Note: if there is a good chance that passed_time_count == day_count,
      //       for better performance, can avoid the 128 bit calculation by adding a check.
      //       Since it's not the case on Dxperts mainnet, we're not using a check here.
      fc::uint128_t pay = requested_pay.value;
      pay *= passed_time_count;
      pay /= day_count;
      requested_pay = static_cast<uint64_t>(pay);

      share_type actual_pay = std::min(budget, requested_pay);
      //ilog(" ==> Paying ${a} to benefactor ${w}", ("w", active_benefactor.id)("a", actual_pay));
      modify(active_benefactor, [&](benefactor_object& w) {
         w.benefactor.visit(benefactor_pay_visitor(actual_pay, *this));
      });

      budget -= actual_pay;
   }
}

void database::update_active_blockproducers()
{ try {
   assert( _blockproducer_count_histogram_buffer.size() > 0 );
   share_type stake_target = (_total_voting_stake[1]-_blockproducer_count_histogram_buffer[0]) / 2;

   /// accounts that vote for 0 or 1 blockproducer do not get to express an opinion on
   /// the number of blockproducers to have (they abstain and are non-voting accounts)

   share_type stake_tally = 0; 

   size_t blockproducer_count = 0;
   if( stake_target > 0 )
   {
      while( (blockproducer_count < _blockproducer_count_histogram_buffer.size() - 1)
             && (stake_tally <= stake_target) )
      {
         stake_tally += _blockproducer_count_histogram_buffer[++blockproducer_count];
      }
   }

   const chain_property_object& cpo = get_chain_properties();

   blockproducer_count = std::max( blockproducer_count*2+1, (size_t)cpo.immutable_parameters.min_blockproducer_count );
   auto wits = sort_votable_objects<blockproducer_index>( blockproducer_count );

   const global_property_object& gpo = get_global_properties();

   auto update_blockproducer_total_votes = [this]( const blockproducer_object& wit ) {
      modify( wit, [this]( blockproducer_object& obj )
      {
         obj.total_votes = _vote_tally_buffer[obj.vote_id];
      });
   };

   if( _track_standby_votes )
   {
      const auto& all_blockproducers = get_index_type<blockproducer_index>().indices();
      for( const blockproducer_object& wit : all_blockproducers )
      {
         update_blockproducer_total_votes( wit );
      }
   }
   else
   {
      for( const blockproducer_object& wit : wits )
      {
         update_blockproducer_total_votes( wit );
      }
   }

   // Update blockproducer authority
   modify( get(GRAPHENE_BLOCKPRODUCER_ACCOUNT), [this,&wits]( account_object& a )
   {
      if( head_block_time() < HARDFORK_533_TIME )
      {
         uint64_t total_votes = 0;
         map<account_id_type, uint64_t> weights;
         a.active.weight_threshold = 0;
         a.active.clear();

         for( const blockproducer_object& wit : wits )
         {
            weights.emplace(wit.blockproducer_account, _vote_tally_buffer[wit.vote_id]);
            total_votes += _vote_tally_buffer[wit.vote_id];
         }

         // total_votes is 64 bits. Subtract the number of leading low bits from 64 to get the number of useful bits,
         // then I want to keep the most significant 16 bits of what's left.
         int8_t bits_to_drop = std::max(int(boost::multiprecision::detail::find_msb(total_votes)) - 15, 0);
         for( const auto& weight : weights )
         {
            // Ensure that everyone has at least one vote. Zero weights aren't allowed.
            uint16_t votes = std::max((weight.second >> bits_to_drop), uint64_t(1) );
            a.active.account_auths[weight.first] += votes;
            a.active.weight_threshold += votes;
         }

         a.active.weight_threshold /= 2;
         a.active.weight_threshold += 1;
      }
      else
      {
         vote_counter vc;
         for( const blockproducer_object& wit : wits )
            vc.add( wit.blockproducer_account, _vote_tally_buffer[wit.vote_id] );
         vc.finish( a.active );
      }
   } );

   modify( gpo, [&wits]( global_property_object& gp )
   {
      gp.active_blockproducers.clear();
      gp.active_blockproducers.reserve(wits.size());
      std::transform(wits.begin(), wits.end(),
                     std::inserter(gp.active_blockproducers, gp.active_blockproducers.end()),
                     [](const blockproducer_object& w) {
         return w.id;
      });
   });

} FC_CAPTURE_AND_RETHROW() }

void database::update_active_dxpcore_members()
{ try {
   assert( _dxpcore_count_histogram_buffer.size() > 0 );
   share_type stake_target = (_total_voting_stake[0]-_dxpcore_count_histogram_buffer[0]) / 2;

   /// accounts that vote for 0 or 1 dxpcore member do not get to express an opinion on
   /// the number of dxpcore members to have (they abstain and are non-voting accounts)
   share_type stake_tally = 0;
   size_t dxpcore_member_count = 0;
   if( stake_target > 0 )
   {
      while( (dxpcore_member_count < _dxpcore_count_histogram_buffer.size() - 1)
             && (stake_tally <= stake_target.value) )
      {
         stake_tally += _dxpcore_count_histogram_buffer[++dxpcore_member_count];
      }
   }

   const chain_property_object& cpo = get_chain_properties();

   dxpcore_member_count = std::max( dxpcore_member_count*2+1, (size_t)cpo.immutable_parameters.min_dxpcore_member_count );
   auto dxpcore_members = sort_votable_objects<dxpcore_member_index>( dxpcore_member_count );

   auto update_dxpcore_member_total_votes = [this]( const dxpcore_member_object& cm ) {
      modify( cm, [this]( dxpcore_member_object& obj )
      {
         obj.total_votes = _vote_tally_buffer[obj.vote_id];
      });
   };

   if( _track_standby_votes )
   {
      const auto& all_dxpcore_members = get_index_type<dxpcore_member_index>().indices();
      for( const dxpcore_member_object& cm : all_dxpcore_members )
      {
         update_dxpcore_member_total_votes( cm );
      }
   }
   else
   {
      for( const dxpcore_member_object& cm : dxpcore_members )
      {
         update_dxpcore_member_total_votes( cm );
      }
   }

   // Update dxpcore authorities
   if( !dxpcore_members.empty() )
   {
      const account_object& dxpcore_account = get(GRAPHENE_DXPCORE_ACCOUNT);
      modify( dxpcore_account, [this,&dxpcore_members](account_object& a)
      {
         if( head_block_time() < HARDFORK_533_TIME )
         {
            uint64_t total_votes = 0;
            map<account_id_type, uint64_t> weights;
            a.active.weight_threshold = 0;
            a.active.clear();

            for( const dxpcore_member_object& cm : dxpcore_members )
            {
               weights.emplace( cm.dxpcore_member_account, _vote_tally_buffer[cm.vote_id] );
               total_votes += _vote_tally_buffer[cm.vote_id];
            }

            // total_votes is 64 bits. Subtract the number of leading low bits from 64 to get the number of useful bits,
            // then I want to keep the most significant 16 bits of what's left.
            int8_t bits_to_drop = std::max(int(boost::multiprecision::detail::find_msb(total_votes)) - 15, 0);
            for( const auto& weight : weights )
            {
               // Ensure that everyone has at least one vote. Zero weights aren't allowed.
               uint16_t votes = std::max((weight.second >> bits_to_drop), uint64_t(1) );
               a.active.account_auths[weight.first] += votes;
               a.active.weight_threshold += votes;
            }

            a.active.weight_threshold /= 2;
            a.active.weight_threshold += 1;
         }
         else
         {
            vote_counter vc;
            for( const dxpcore_member_object& cm : dxpcore_members )
               vc.add( cm.dxpcore_member_account, _vote_tally_buffer[cm.vote_id] );
            vc.finish( a.active );
         }
      });
      modify( get(GRAPHENE_RELAXED_DXPCORE_ACCOUNT), [&dxpcore_account](account_object& a)
      {
         a.active = dxpcore_account.active;
      });
   }
   modify( get_global_properties(), [&dxpcore_members](global_property_object& gp)
   {
      gp.active_dxpcore_members.clear();
      std::transform(dxpcore_members.begin(), dxpcore_members.end(),
                     std::inserter(gp.active_dxpcore_members, gp.active_dxpcore_members.begin()),
                     [](const dxpcore_member_object& d) { return d.id; });
   });
} FC_CAPTURE_AND_RETHROW() }

void database::initialize_budget_record( fc::time_point_sec now, budget_record& rec )const
{
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();
   const asset_object& core = get_core_asset();
   const asset_dynamic_data_object& core_dd = get_core_dynamic_data();

   rec.from_initial_reserve = core.reserved(*this);
   rec.from_accumulated_fees = core_dd.accumulated_fees;
   rec.from_unused_blockproducer_budget = dpo.blockproducer_budget;
   rec.max_supply = core.options.max_supply;

   if(    (dpo.last_budget_time == fc::time_point_sec())
       || (now <= dpo.last_budget_time) )
   {
      rec.time_since_last_budget = 0;
      return;
   }

   int64_t dt = (now - dpo.last_budget_time).to_seconds();
   rec.time_since_last_budget = uint64_t( dt );

   // We'll consider accumulated_fees to be reserved at the BEGINNING
   // of the maintenance interval.  However, for speed we only
   // call modify() on the asset_dynamic_data_object once at the
   // end of the maintenance interval.  Thus the accumulated_fees
   // are available for the budget at this point, but not included
   // in core.reserved().
   share_type reserve = rec.from_initial_reserve + core_dd.accumulated_fees;
   // Similarly, we consider leftover blockproducer_budget to be burned
   // at the BEGINNING of the maintenance interval.
   reserve += dpo.blockproducer_budget;

   fc::uint128_t budget_u128 = reserve.value;
   budget_u128 *= uint64_t(dt);
   budget_u128 *= GRAPHENE_CORE_ASSET_CYCLE_RATE;
   //round up to the nearest satoshi -- this is necessary to ensure
   //   there isn't an "untouchable" reserve, and we will eventually
   //   be able to use the entire reserve
   budget_u128 += ((uint64_t(1) << GRAPHENE_CORE_ASSET_CYCLE_RATE_BITS) - 1);
   budget_u128 >>= GRAPHENE_CORE_ASSET_CYCLE_RATE_BITS;
   if( budget_u128 < static_cast<fc::uint128_t>(reserve.value) )
      rec.total_budget = share_type(static_cast<uint64_t>(budget_u128));
   else
      rec.total_budget = reserve;

   return;
}

/**
 * Update the budget for blockproducers and benefactors.
 */
void database::process_budget()
{
   try
   {
      const global_property_object& gpo = get_global_properties();
      const dynamic_global_property_object& dpo = get_dynamic_global_properties();
      const asset_dynamic_data_object& core = get_core_dynamic_data();
      fc::time_point_sec now = head_block_time();

      int64_t time_to_maint = (dpo.next_maintenance_time - now).to_seconds();
      //
      // The code that generates the next maintenance time should
      //    only produce a result in the future.  If this assert
      //    fails, then the next maintenance time algorithm is buggy.
      //
      assert( time_to_maint > 0 );
      //
      // Code for setting chain parameters should validate
      //    block_interval > 0 (as well as the humans proposing /
      //    voting on changes to block interval).
      //
      assert( gpo.parameters.block_interval > 0 );
      uint64_t blocks_to_maint = (uint64_t(time_to_maint) + gpo.parameters.block_interval - 1) / gpo.parameters.block_interval;

      // blocks_to_maint > 0 because time_to_maint > 0,
      // which means numerator is at least equal to block_interval

      budget_record rec;
      initialize_budget_record( now, rec );
      share_type available_funds = rec.total_budget;

      share_type blockproducer_budget = gpo.parameters.blockproducer_pay_per_block.value * blocks_to_maint;
      rec.requested_blockproducer_budget = blockproducer_budget;
      blockproducer_budget = std::min(blockproducer_budget, available_funds);
      rec.blockproducer_budget = blockproducer_budget;
      available_funds -= blockproducer_budget;

      fc::uint128_t benefactor_budget_u128 = gpo.parameters.benefactor_budget_per_day.value;
      benefactor_budget_u128 *= uint64_t(time_to_maint);
      benefactor_budget_u128 /= 60*60*24;

      share_type benefactor_budget;
      if( benefactor_budget_u128 >= static_cast<fc::uint128_t>(available_funds.value) )
         benefactor_budget = available_funds;
      else
         benefactor_budget = static_cast<uint64_t>(benefactor_budget_u128);
      rec.benefactor_budget = benefactor_budget;
      available_funds -= benefactor_budget;

      share_type leftover_benefactor_funds = benefactor_budget;
      pay_benefactors(leftover_benefactor_funds);
      rec.leftover_benefactor_funds = leftover_benefactor_funds;
      available_funds += leftover_benefactor_funds;

      rec.supply_delta = rec.blockproducer_budget
         + rec.benefactor_budget
         - rec.leftover_benefactor_funds
         - rec.from_accumulated_fees
         - rec.from_unused_blockproducer_budget;

      modify(core, [&]( asset_dynamic_data_object& _core )
      {
         _core.current_supply = (_core.current_supply + rec.supply_delta );

         assert( rec.supply_delta ==
                                   blockproducer_budget
                                 + benefactor_budget
                                 - leftover_benefactor_funds
                                 - _core.accumulated_fees
                                 - dpo.blockproducer_budget
                                );
         _core.accumulated_fees = 0;
      });

      modify(dpo, [&]( dynamic_global_property_object& _dpo )
      {
         // Since initial blockproducer_budget was rolled into
         // available_funds, we replace it with blockproducer_budget
         // instead of adding it.
         _dpo.blockproducer_budget = blockproducer_budget;
         _dpo.last_budget_time = now;
      });

      rec.current_supply = core.current_supply;
      create< budget_record_object >( [&]( budget_record_object& _rec )
      {
         _rec.time = head_block_time();
         _rec.record = rec;
      });

      // available_funds is money we could spend, but don't want to.
      // we simply let it evaporate back into the reserve.
   }
   FC_CAPTURE_AND_RETHROW()
}

template< typename Visitor >
void visit_special_authorities( const database& db, Visitor visit )
{
   const auto& sa_idx = db.get_index_type< special_authority_index >().indices().get<by_id>();

   for( const special_authority_object& sao : sa_idx )
   {
      const account_object& acct = sao.account(db);
      if( !acct.owner_special_authority.is_type< no_special_authority >() )
      {
         visit( acct, true, acct.owner_special_authority );
      }
      if( !acct.active_special_authority.is_type< no_special_authority >() )
      {
         visit( acct, false, acct.active_special_authority );
      }
   }
}

void update_top_n_authorities( database& db )
{
   visit_special_authorities( db,
   [&]( const account_object& acct, bool is_owner, const special_authority& auth )
   {
      if( auth.is_type< top_holders_special_authority >() )
      {
         // use index to grab the top N holders of the asset and vote_counter to obtain the weights

         const top_holders_special_authority& tha = auth.get< top_holders_special_authority >();
         vote_counter vc;
         const auto& bal_idx = db.get_index_type< account_balance_index >().indices().get< by_asset_balance >();
         uint8_t num_needed = tha.num_top_holders;
         if( num_needed == 0 )
            return;

         // find accounts
         const auto range = bal_idx.equal_range( boost::make_tuple( tha.asset ) );
         for( const account_balance_object& bal : boost::make_iterator_range( range.first, range.second ) )
         {
             assert( bal.asset_type == tha.asset );
             if( bal.owner == acct.id )
                continue;
             vc.add( bal.owner, bal.balance.value );
             --num_needed;
             if( num_needed == 0 )
                break;
         }

         db.modify( acct, [&]( account_object& a )
         {
            vc.finish( is_owner ? a.owner : a.active );
            if( !vc.is_empty() )
               a.top_n_control_flags |= (is_owner ? account_object::top_n_control_owner : account_object::top_n_control_active);
         } );
      }
   } );
}

void split_fba_balance(
   database& db,
   uint64_t fba_id,
   uint16_t network_pct,
   uint16_t designated_asset_buyback_pct,
   uint16_t designated_asset_issuer_pct
)
{
   FC_ASSERT( uint32_t(network_pct) + uint32_t(designated_asset_buyback_pct) + uint32_t(designated_asset_issuer_pct) == GRAPHENE_100_PERCENT );
   const fba_accumulator_object& fba = fba_accumulator_id_type( fba_id )(db);
   if( fba.accumulated_fba_fees == 0 )
      return;

   const asset_dynamic_data_object& core_dd = db.get_core_dynamic_data();

   if( !fba.is_configured(db) )
   {
      ilog( "${n} core given to network at block ${b} due to non-configured FBA", ("n", fba.accumulated_fba_fees)("b", db.head_block_time()) );
      db.modify( core_dd, [&]( asset_dynamic_data_object& _core_dd )
      {
         _core_dd.current_supply -= fba.accumulated_fba_fees;
      } );
      db.modify( fba, [&]( fba_accumulator_object& _fba )
      {
         _fba.accumulated_fba_fees = 0;
      } );
      return;
   }

   fc::uint128_t buyback_amount_128 = fba.accumulated_fba_fees.value;
   buyback_amount_128 *= designated_asset_buyback_pct;
   buyback_amount_128 /= GRAPHENE_100_PERCENT;
   share_type buyback_amount = static_cast<uint64_t>(buyback_amount_128);

   fc::uint128_t issuer_amount_128 = fba.accumulated_fba_fees.value;
   issuer_amount_128 *= designated_asset_issuer_pct;
   issuer_amount_128 /= GRAPHENE_100_PERCENT;
   share_type issuer_amount = static_cast<uint64_t>(issuer_amount_128);

   // this assert should never fail
   FC_ASSERT( buyback_amount + issuer_amount <= fba.accumulated_fba_fees );

   share_type network_amount = fba.accumulated_fba_fees - (buyback_amount + issuer_amount);

   const asset_object& designated_asset = (*fba.designated_asset)(db);

   if( network_amount != 0 )
   {
      db.modify( core_dd, [&]( asset_dynamic_data_object& _core_dd )
      {
         _core_dd.current_supply -= network_amount;
      } );
   }

   fba_distribute_operation vop;
   vop.account_id = *designated_asset.buyback_account;
   vop.fba_id = fba.id;
   vop.amount = buyback_amount;
   if( vop.amount != 0 )
   {
      db.adjust_balance( *designated_asset.buyback_account, asset(buyback_amount) );
      db.push_applied_operation(vop);
   }

   vop.account_id = designated_asset.issuer;
   vop.fba_id = fba.id;
   vop.amount = issuer_amount;
   if( vop.amount != 0 )
   {
      db.adjust_balance( designated_asset.issuer, asset(issuer_amount) );
      db.push_applied_operation(vop);
   }

   db.modify( fba, [&]( fba_accumulator_object& _fba )
   {
      _fba.accumulated_fba_fees = 0;
   } );
}

void distribute_fba_balances( database& db )
{
   split_fba_balance( db, fba_accumulator_id_transfer_to_blind  , 20*GRAPHENE_1_PERCENT, 60*GRAPHENE_1_PERCENT, 20*GRAPHENE_1_PERCENT );
   split_fba_balance( db, fba_accumulator_id_blind_transfer     , 20*GRAPHENE_1_PERCENT, 60*GRAPHENE_1_PERCENT, 20*GRAPHENE_1_PERCENT );
   split_fba_balance( db, fba_accumulator_id_transfer_from_blind, 20*GRAPHENE_1_PERCENT, 60*GRAPHENE_1_PERCENT, 20*GRAPHENE_1_PERCENT );
}

void create_buyback_orders( database& db )
{
   const auto& bbo_idx = db.get_index_type< buyback_index >().indices().get<by_id>();
   const auto& bal_idx = db.get_index_type< primary_index< account_balance_index > >().get_secondary_index< balances_by_account_index >();

   for( const buyback_object& bbo : bbo_idx )
   {
      const asset_object& asset_to_buy = bbo.asset_to_buy(db);
      assert( asset_to_buy.buyback_account.valid() );

      const account_object& buyback_account = (*(asset_to_buy.buyback_account))(db);

      if( !buyback_account.allowed_assets.valid() )
      {
         wlog( "skipping buyback account ${b} at block ${n} because allowed_assets does not exist", ("b", buyback_account)("n", db.head_block_num()) );
         continue;
      }

      for( const auto& entry : bal_idx.get_account_balances( buyback_account.id ) )
      {
         const auto* it = entry.second;
         asset_id_type asset_to_sell = it->asset_type;
         share_type amount_to_sell = it->balance;
         if( asset_to_sell == asset_to_buy.id )
            continue;
         if( amount_to_sell == 0 )
            continue;
         if( buyback_account.allowed_assets->find( asset_to_sell ) == buyback_account.allowed_assets->end() )
         {
            wlog( "buyback account ${b} not selling disallowed holdings of asset ${a} at block ${n}", ("b", buyback_account)("a", asset_to_sell)("n", db.head_block_num()) );
            continue;
         }

         try
         {
            transaction_evaluation_state buyback_context(&db);
            buyback_context.skip_fee_schedule_check = true;

            limit_order_create_operation create_vop;
            create_vop.fee = asset( 0, asset_id_type() );
            create_vop.seller = buyback_account.id;
            create_vop.amount_to_sell = asset( amount_to_sell, asset_to_sell );
            create_vop.min_to_receive = asset( 1, asset_to_buy.id );
            create_vop.expiration = time_point_sec::maximum();
            create_vop.fill_or_kill = false;

            limit_order_id_type order_id = db.apply_operation( buyback_context, create_vop ).get< object_id_type >();

            if( db.find( order_id ) != nullptr )
            {
               limit_order_cancel_operation cancel_vop;
               cancel_vop.fee = asset( 0, asset_id_type() );
               cancel_vop.order = order_id;
               cancel_vop.fee_paying_account = buyback_account.id;

               db.apply_operation( buyback_context, cancel_vop );
            }
         }
         catch( const fc::exception& e )
         {
            // we can in fact get here, e.g. if asset issuer of buy/sell asset blacklists/whitelists the buyback account
            wlog( "Skipping buyback processing selling ${as} for ${ab} for buyback account ${b} at block ${n}; exception was ${e}",
                  ("as", asset_to_sell)("ab", asset_to_buy)("b", buyback_account)("n", db.head_block_num())("e", e.to_detail_string()) );
            continue;
         }
      }
   }
   return;
}

void deprecate_annual_members( database& db )
{
   const auto& account_idx = db.get_index_type<account_index>().indices().get<by_id>();
   fc::time_point_sec now = db.head_block_time();
   for( const account_object& acct : account_idx )
   {
      try
      {
         transaction_evaluation_state upgrade_context(&db);
         upgrade_context.skip_fee_schedule_check = true;

         if( acct.is_annual_member( now ) )
         {
            account_upgrade_operation upgrade_vop;
            upgrade_vop.fee = asset( 0, asset_id_type() );
            upgrade_vop.account_to_upgrade = acct.id;
            upgrade_vop.upgrade_to_lifetime_member = true;
            db.apply_operation( upgrade_context, upgrade_vop );
         }
      }
      catch( const fc::exception& e )
      {
         // we can in fact get here, e.g. if asset issuer of buy/sell asset blacklists/whitelists the buyback account
         wlog( "Skipping annual member deprecate processing for account ${a} (${an}) at block ${n}; exception was ${e}",
               ("a", acct.id)("an", acct.name)("n", db.head_block_num())("e", e.to_detail_string()) );
         continue;
      }
   }
   return;
}

void database::process_bids( const asset_smarttoken_data_object& bad )
{
   if( bad.is_prediction_market ) return;
   if( bad.current_feed.settlement_price.is_null() ) return;

   asset_id_type to_revive_id = (asset( 0, bad.options.short_backing_asset ) * bad.settlement_price).asset_id;
   const asset_object& to_revive = to_revive_id( *this );
   const asset_dynamic_data_object& bdd = to_revive.dynamic_data( *this );

   const auto& bid_idx = get_index_type< collateral_bid_index >().indices().get<by_price>();
   const auto start = bid_idx.lower_bound( boost::make_tuple( to_revive_id, price::max( bad.options.short_backing_asset, to_revive_id ), collateral_bid_id_type() ) );

   share_type covered = 0;
   auto itr = start;
   while( covered < bdd.current_supply && itr != bid_idx.end() && itr->inv_swan_price.quote.asset_id == to_revive_id )
   {
      const collateral_bid_object& bid = *itr;
      asset debt_in_bid = bid.inv_swan_price.quote;
      if( debt_in_bid.amount > bdd.current_supply )
         debt_in_bid.amount = bdd.current_supply;
      asset total_collateral = debt_in_bid * bad.settlement_price;
      total_collateral += bid.inv_swan_price.base;
      price call_price = price::call_price( debt_in_bid, total_collateral, bad.current_feed.maintenance_collateral_ratio );
      if( ~call_price >= bad.current_feed.settlement_price ) break;
      covered += debt_in_bid.amount;
      ++itr;
   }
   if( covered < bdd.current_supply ) return;

   const auto end = itr;
   share_type to_cover = bdd.current_supply;
   share_type remaining_fund = bad.settlement_fund;
   for( itr = start; itr != end; )
   {
      const collateral_bid_object& bid = *itr;
      ++itr;
      asset debt_in_bid = bid.inv_swan_price.quote;
      if( debt_in_bid.amount > bdd.current_supply )
         debt_in_bid.amount = bdd.current_supply;
      share_type debt = debt_in_bid.amount;
      share_type collateral = (debt_in_bid * bad.settlement_price).amount;
      if( debt >= to_cover )
      {
         debt = to_cover;
         collateral = remaining_fund;
      }
      to_cover -= debt;
      remaining_fund -= collateral;
      execute_bid( bid, debt, collateral, bad.current_feed );
   }
   FC_ASSERT( remaining_fund == 0 );
   FC_ASSERT( to_cover == 0 );

   _cancel_bids_and_revive_mpa( to_revive, bad );
}

/// Reset call_price of all call orders according to their remaining collateral and debt.
/// Do not update orders of prediction markets because we're sure they're up to date.
void update_call_orders_hf_343( database& db )
{
   // Update call_price
   wlog( "Updating all call orders for hardfork core-343 at block ${n}", ("n",db.head_block_num()) );
   asset_id_type current_asset;
   const asset_smarttoken_data_object* abd = nullptr;
   // by_collateral index won't change after call_price updated, so it's safe to iterate
   for( const auto& call_obj : db.get_index_type<call_order_index>().indices().get<by_collateral>() )
   {
      if( current_asset != call_obj.debt_type() ) // debt type won't be asset_id_type(), abd will always get initialized
      {
         current_asset = call_obj.debt_type();
         abd = &current_asset(db).smarttoken_data(db);
      }
      if( !abd || abd->is_prediction_market ) // nothing to do with PM's; check !abd just to be safe
         continue;
      db.modify( call_obj, [abd]( call_order_object& call ) {
         call.call_price  =  price::call_price( call.get_debt(), call.get_collateral(),
                                                abd->current_feed.maintenance_collateral_ratio );
      });
   }
   wlog( "Done updating all call orders for hardfork core-343 at block ${n}", ("n",db.head_block_num()) );
}

/// Reset call_price of all call orders to (1,1) since it won't be used in the future.
/// Update PMs as well.
void update_call_orders_hf_1270( database& db )
{
   // Update call_price
   for( const auto& call_obj : db.get_index_type<call_order_index>().indices().get<by_id>() )
   {
      db.modify( call_obj, []( call_order_object& call ) {
         call.call_price.base.amount = 1;
         call.call_price.quote.amount = 1;
      });
   }
}

/// Match call orders for all smartTokens, including PMs.
void match_call_orders( database& db )
{
   // Match call orders
   wlog( "Matching call orders at block ${n}", ("n",db.head_block_num()) );
   const auto& asset_idx = db.get_index_type<asset_index>().indices().get<by_type>();
   auto itr = asset_idx.lower_bound( true /** market issued */ );
   while( itr != asset_idx.end() )
   {
      const asset_object& a = *itr;
      ++itr;
      // be here, next_maintenance_time should have been updated already
      db.check_call_orders( a, true, false ); // allow black swan, and call orders are taker
   }
   wlog( "Done matching call orders at block ${n}", ("n",db.head_block_num()) );
}

void database::process_smarttokens()
{
   time_point_sec head_time = head_block_time();
   uint32_t head_epoch_seconds = head_time.sec_since_epoch();
   bool after_hf_core_518 = ( head_time >= HARDFORK_CORE_518_TIME ); // clear expired feeds

   const auto update_smarttoken = [this,head_time,head_epoch_seconds,after_hf_core_518]( asset_smarttoken_data_object &o )
   {
      o.force_settled_volume = 0; // Reset all SmartToken force settlement volumes to zero

      // clear expired feeds
      if( after_hf_core_518 )
      {
         const auto &asset = get( o.asset_id );
         auto flags = asset.options.flags;
         if ( ( flags & ( blockproducer_fed_asset | dxpcore_fed_asset ) ) &&
              o.options.feed_lifetime_sec < head_epoch_seconds ) // if smartcoin && check overflow
         {
            fc::time_point_sec calculated = head_time - o.options.feed_lifetime_sec;
            for( auto itr = o.feeds.rbegin(); itr != o.feeds.rend(); ) // loop feeds
            {
               auto feed_time = itr->second.first;
               std::advance( itr, 1 );
               if( feed_time < calculated )
                  o.feeds.erase( itr.base() ); // delete expired feed
            }
         }
      }
   };

   for( const auto& d : get_index_type<asset_smarttoken_data_index>().indices() )
   {
      modify( d, update_smarttoken );
      if( d.has_settlement() )
         process_bids(d);
   }
}

/****
 * @brief a one-time data process to correct max_supply
 * 
 * NOTE: while exceeding max_supply happened in mainnet, it seemed to have corrected
 * itself before HF 1465. But this method must remain to correct some assets in testnet
 */
void process_hf_1465( database& db )
{
   // for each market issued asset
   const auto& asset_idx = db.get_index_type<asset_index>().indices().get<by_type>();
   for( auto asset_itr = asset_idx.lower_bound(true); asset_itr != asset_idx.end(); ++asset_itr )
   {
      const auto& current_asset = *asset_itr;
      graphene::chain::share_type current_supply = current_asset.dynamic_data(db).current_supply;
      graphene::chain::share_type max_supply = current_asset.options.max_supply;
      if (current_supply > max_supply && max_supply != GRAPHENE_MAX_SHARE_SUPPLY)
      {
         wlog( "Adjusting max_supply of ${asset} because current_supply (${current_supply}) is greater than ${old}.", 
               ("asset", current_asset.symbol) 
               ("current_supply", current_supply.value)
               ("old", max_supply));
         db.modify<asset_object>( current_asset, [current_supply](asset_object& obj) {
            obj.options.max_supply = graphene::chain::share_type(std::min(current_supply.value, GRAPHENE_MAX_SHARE_SUPPLY));
         });
      }
   }
}

/****
 * @brief a one-time data process to correct current_supply of DXP token in the Dxperts mainnet
 */
void process_hf_2103( database& db )
{
   const balance_object* bal = db.find( balance_id_type( HARDFORK_CORE_2103_BALANCE_ID ) );
   if( bal != nullptr && bal->balance.amount < 0 )
   {
      const asset_dynamic_data_object& ddo = bal->balance.asset_id(db).dynamic_data(db);
      db.modify<asset_dynamic_data_object>( ddo, [bal](asset_dynamic_data_object& obj) {
         obj.current_supply -= bal->balance.amount;
      });
      db.remove( *bal );
   }
}

void update_median_feeds(database& db)
{
   time_point_sec head_time = db.head_block_time();
   time_point_sec next_maint_time = db.get_dynamic_global_properties().next_maintenance_time;

   const auto update_smarttoken = [head_time, next_maint_time]( asset_smarttoken_data_object &o )
   {
      o.update_median_feeds( head_time, next_maint_time );
   };

   for( const auto& d : db.get_index_type<asset_smarttoken_data_index>().indices() )
   {
      db.modify( d, update_smarttoken );
   }
}

/******
 * @brief one-time data process for hard fork core-868-890
 *
 * Prior to hardfork 868, switching a smarttoken's shorting asset would not reset its
 * feeds. This method will run at the hardfork time, and erase (or nullify) feeds
 * that have incorrect backing assets.
 * https://gitlab.com/dxperts/dxperts-core/issues/868
 *
 * Prior to hardfork 890, changing a smarttoken's feed expiration time would not
 * trigger a median feed update. This method will run at the hardfork time, and
 * correct all median feed data.
 * https://gitlab.com/dxperts/dxperts-core/issues/890
 *
 * @param db the database
 * @param skip_check_call_orders true if check_call_orders() should not be called
 */
// NOTE: Unable to remove this function for testnet nor mainnet. Unfortunately, bad
//       feeds were found.
void process_hf_868_890( database& db, bool skip_check_call_orders )
{
   const auto next_maint_time = db.get_dynamic_global_properties().next_maintenance_time;
   const auto head_time = db.head_block_time();
   // for each market issued asset
   const auto& asset_idx = db.get_index_type<asset_index>().indices().get<by_type>();
   for( auto asset_itr = asset_idx.lower_bound(true); asset_itr != asset_idx.end(); ++asset_itr )
   {
      const auto& current_asset = *asset_itr;
      // Incorrect blockproducer & dxpcore feeds can simply be removed.
      // For non-blockproducer-fed and non-dxpcore-fed assets, set incorrect
      // feeds to price(), since we can't simply remove them. For more information:
      // https://gitlab.com/dxperts/dxperts-core/pull/832#issuecomment-384112633
      bool is_blockproducer_or_dxpcore_fed = false;
      if ( current_asset.options.flags & ( blockproducer_fed_asset | dxpcore_fed_asset ) )
         is_blockproducer_or_dxpcore_fed = true;

      // for each feed
      const asset_smarttoken_data_object& smarttoken_data = current_asset.smarttoken_data(db);
      auto itr = smarttoken_data.feeds.begin();
      while( itr != smarttoken_data.feeds.end() )
      {
         // If the feed is invalid
         if ( itr->second.second.settlement_price.quote.asset_id != smarttoken_data.options.short_backing_asset
               && ( is_blockproducer_or_dxpcore_fed || itr->second.second.settlement_price != price() ) )
         {
            db.modify( smarttoken_data, [&itr, is_blockproducer_or_dxpcore_fed]( asset_smarttoken_data_object& obj )
            {
               if( is_blockproducer_or_dxpcore_fed )
               {
                  // erase the invalid feed
                  itr = obj.feeds.erase(itr);
               }
               else
               {
                  // nullify the invalid feed
                  obj.feeds[itr->first].second.settlement_price = price();
                  ++itr;
               }
            });
         }
         else
         {
            // Feed is valid. Skip it.
            ++itr;
         }
      } // end loop of each feed

      // always update the median feed due to https://gitlab.com/dxperts/dxperts-core/issues/890
      db.modify( smarttoken_data, [head_time,next_maint_time]( asset_smarttoken_data_object &obj ) {
         obj.update_median_feeds( head_time, next_maint_time );
         // NOTE: Normally we should call check_call_orders() after called update_median_feeds(), but for
         // mainnet actually check_call_orders() would do nothing, so we skipped it for better performance.
      });

   } // for each market issued asset
}


/**
 * @brief Remove any custom active authorities whose expiration dates are in the past
 * @param db A mutable database reference
 */
void delete_expired_custom_authorities( database& db )
{
   const auto& index = db.get_index_type<custom_authority_index>().indices().get<by_expiration>();
   while (!index.empty() && index.begin()->valid_to < db.head_block_time())
      db.remove(*index.begin());
}

/// A one-time data process to set values of existing liquid tickets to zero.
void process_hf_2262( database& db )
{
   for( const auto& ticket_obj : db.get_index_type<ticket_index>().indices().get<by_id>() )
   {
      if( ticket_obj.current_type != liquid ) // only update liquid tickets
         continue;
      db.modify( db.get_account_stats_by_owner( ticket_obj.account ), [&ticket_obj](account_statistics_object& aso) {
         aso.total_pol_value -= ticket_obj.value;
      });
      db.modify( ticket_obj, []( ticket_object& t ) {
         t.value = 0;
      });
   }
}

namespace detail {

   struct vote_recalc_times
   {
      time_point_sec full_power_time;
      time_point_sec zero_power_time;
   };

   struct vote_recalc_options
   {
      vote_recalc_options( uint32_t f, uint32_t d, uint32_t s )
      : full_power_seconds(f), recalc_steps(d), seconds_per_step(s)
      {
         total_recalc_seconds = ( recalc_steps - 1 ) * seconds_per_step; // should not overflow
         power_percents_to_subtract.reserve( recalc_steps - 1 );
         for( uint32_t i = 1; i < recalc_steps; ++i )
            power_percents_to_subtract.push_back( GRAPHENE_100_PERCENT * i / recalc_steps ); // should not overflow
      }

      vote_recalc_times get_vote_recalc_times( const time_point_sec now ) const
      {
         return { now - full_power_seconds, now - full_power_seconds - total_recalc_seconds };
      }

      uint32_t full_power_seconds;
      uint32_t recalc_steps; // >= 1
      uint32_t seconds_per_step;
      uint32_t total_recalc_seconds;
      vector<uint16_t> power_percents_to_subtract;

      static const vote_recalc_options blockproducer();
      static const vote_recalc_options dxpcore();
      static const vote_recalc_options benefactor();
      static const vote_recalc_options delegator();

      // return the stake that is "recalced to X"
      uint64_t get_recalced_voting_stake( const uint64_t stake, const time_point_sec last_vote_time,
                                         const vote_recalc_times& recalc_times ) const
      {
         if( last_vote_time > recalc_times.full_power_time )
            return stake;
         if( last_vote_time <= recalc_times.zero_power_time )
            return 0;
         uint32_t diff = recalc_times.full_power_time.sec_since_epoch() - last_vote_time.sec_since_epoch();
         uint32_t steps_to_subtract_minus_1 = diff / seconds_per_step;
         fc::uint128_t stake_to_subtract( stake );
         stake_to_subtract *= power_percents_to_subtract[steps_to_subtract_minus_1];
         stake_to_subtract /= GRAPHENE_100_PERCENT;
         return stake - static_cast<uint64_t>(stake_to_subtract);
      }
   };

   const vote_recalc_options vote_recalc_options::blockproducer()
   {
      static const vote_recalc_options o( 360*86400, 8, 45*86400 );
      return o;
   }
   const vote_recalc_options vote_recalc_options::dxpcore()
   {
      static const vote_recalc_options o( 360*86400, 8, 45*86400 );
      return o;
   }
   const vote_recalc_options vote_recalc_options::benefactor()
   {
      static const vote_recalc_options o( 360*86400, 8, 45*86400 );
      return o;
   }
   const vote_recalc_options vote_recalc_options::delegator()
   {
      static const vote_recalc_options o( 360*86400, 8, 45*86400 );
      return o;
   }
}

void database::perform_chain_maintenance(const signed_block& next_block, const global_property_object& global_props)
{
   const auto& gpo = get_global_properties();
   const auto& dgpo = get_dynamic_global_properties();
   auto last_vote_tally_time = head_block_time();

   distribute_fba_balances(*this);
   create_buyback_orders(*this);

   struct vote_tally_helper {
      database& d;
      const global_property_object& props;
      const dynamic_global_property_object& dprops;
      const time_point_sec now;
      const bool hf2103_passed;
      const bool hf2262_passed;
      const bool pob_activated;

      optional<detail::vote_recalc_times> blockproducer_recalc_times;
      optional<detail::vote_recalc_times> dxpcore_recalc_times;
      optional<detail::vote_recalc_times> benefactor_recalc_times;
      optional<detail::vote_recalc_times> delegator_recalc_times;

      vote_tally_helper( database& db )
         : d(db), props( d.get_global_properties() ), dprops( d.get_dynamic_global_properties() ), 
           now( d.head_block_time() ), hf2103_passed( HARDFORK_CORE_2103_PASSED( now ) ),
           hf2262_passed( HARDFORK_CORE_2262_PASSED( now ) ),
           pob_activated( dprops.total_pob > 0 || dprops.total_inactive > 0 )
      {
         d._vote_tally_buffer.resize( props.next_available_vote_id, 0 );
         d._blockproducer_count_histogram_buffer.resize( props.parameters.maximum_blockproducer_count / 2 + 1, 0 );
         d._dxpcore_count_histogram_buffer.resize( props.parameters.maximum_dxpcore_count / 2 + 1, 0 );
         d._total_voting_stake[0] = 0;
         d._total_voting_stake[1] = 0;
         if( hf2103_passed )
         {
            blockproducer_recalc_times   = detail::vote_recalc_options::blockproducer().get_vote_recalc_times( now );
            dxpcore_recalc_times = detail::vote_recalc_options::dxpcore().get_vote_recalc_times( now );
            benefactor_recalc_times    = detail::vote_recalc_options::benefactor().get_vote_recalc_times( now );
            delegator_recalc_times = detail::vote_recalc_options::delegator().get_vote_recalc_times( now );
         }
      }

      void operator()( const account_object& stake_account, const account_statistics_object& stats )
      {
         // PoB activation
         if( pob_activated && stats.total_core_pob == 0 && stats.total_core_inactive == 0 )
            return;

         if( props.parameters.count_non_member_votes || stake_account.is_member( now ) )
         {
            // There may be a difference between the account whose stake is voting and the one specifying opinions.
            // Usually they're the same, but if the stake account has specified a voting_account, that account is the
            // one specifying the opinions.
            bool directly_voting = ( stake_account.options.voting_account == GRAPHENE_PROXY_TO_SELF_ACCOUNT );
            const account_object& opinion_account = ( directly_voting ? stake_account
                                                      : d.get(stake_account.options.voting_account) );

            uint64_t voting_stake[3]; // 0=dxpcore, 1=blockproducer, 2=benefactor, as in vote_id_type::vote_type
            uint64_t num_dxpcore_voting_stake; // number of dxpcore members
            voting_stake[2] = ( pob_activated ? 0 : stats.total_core_in_orders.value )
                  + ( ( !hf2262_passed && stake_account.cashback_vb.valid() ) ?
                           (*stake_account.cashback_vb)(d).balance.amount.value : 0 )
                  + ( hf2262_passed ? 0 : stats.core_in_balance.value );

            // voting power stats
            uint64_t vp_all = 0;       ///<  all voting power.
            uint64_t vp_active = 0;    ///<  the voting power of the proxy, if there is no attenuation, it is equal to vp_all.
            uint64_t vp_dxpcore = 0; ///<  the final voting power for the dxpcores.
            uint64_t vp_blockproducer = 0;   ///<  the final voting power for the blockproducers.
            uint64_t vp_benefactor = 0;    ///<  the final voting power for the benefactors.

            //PoB
            const uint64_t pol_amount = stats.total_core_pol.value;
            const uint64_t pol_value = stats.total_pol_value.value;
            const uint64_t pob_amount = stats.total_core_pob.value;
            const uint64_t pob_value = stats.total_pob_value.value;
            if( pob_amount == 0 )
            {
               voting_stake[2] += pol_value;
            }
            else if( pol_amount == 0 ) // and pob_amount > 0
            {
               if( pob_amount <= voting_stake[2] )
               {
                  voting_stake[2] += ( pob_value - pob_amount );
               }
               else
               {
                  auto base_value = static_cast<fc::uint128_t>( voting_stake[2] ) * pob_value / pob_amount;
                  voting_stake[2] = static_cast<uint64_t>( base_value );
               }
            }
            else if( pob_amount <= pol_amount ) // pob_amount > 0 && pol_amount > 0
            {
               auto base_value = static_cast<fc::uint128_t>( pob_value ) * pol_value / pol_amount;
               auto diff_value = static_cast<fc::uint128_t>( pob_amount ) * pol_value / pol_amount;
               base_value += ( pol_value - diff_value );
               voting_stake[2] += static_cast<uint64_t>( base_value );
            }
            else // pob_amount > pol_amount > 0
            {
               auto base_value = static_cast<fc::uint128_t>( pol_value ) * pob_value / pob_amount;
               fc::uint128_t diff_amount = pob_amount - pol_amount;
               if( diff_amount <= voting_stake[2] )
               {
                  auto diff_value = static_cast<fc::uint128_t>( pol_amount ) * pob_value / pob_amount;
                  base_value += ( pob_value - diff_value );
                  voting_stake[2] += static_cast<uint64_t>( base_value - diff_amount );
               }
               else // diff_amount > voting_stake[2]
               {
                  base_value += static_cast<fc::uint128_t>( voting_stake[2] ) * pob_value / pob_amount;
                  voting_stake[2] = static_cast<uint64_t>( base_value );
               }
            }

            // Shortcut
            if( voting_stake[2] == 0 )
               return;

            const account_statistics_object& opinion_account_stats = ( directly_voting ? stats : opinion_account.statistics( d ) );

            // Recalculate votes
            if( !hf2103_passed )
            {
               voting_stake[0] = voting_stake[2];
               voting_stake[1] = voting_stake[2];
               num_dxpcore_voting_stake = voting_stake[2];
               vp_all = vp_active = vp_dxpcore = vp_blockproducer = vp_benefactor = voting_stake[2];
            }
            else
            {
               vp_all = vp_active = voting_stake[2];
               if( !directly_voting )
               {
                  vp_active = voting_stake[2] = detail::vote_recalc_options::delegator().get_recalced_voting_stake( 
                     voting_stake[2], stats.last_vote_time, *delegator_recalc_times );
               }
               vp_blockproducer = voting_stake[1] = detail::vote_recalc_options::blockproducer().get_recalced_voting_stake( 
                  voting_stake[2], opinion_account_stats.last_vote_time, *blockproducer_recalc_times );
               vp_dxpcore = voting_stake[0] = detail::vote_recalc_options::dxpcore().get_recalced_voting_stake( 
                  voting_stake[2], opinion_account_stats.last_vote_time, *dxpcore_recalc_times );
               num_dxpcore_voting_stake = voting_stake[0];
               if( opinion_account.num_dxpcore_voted > 1 )
                  voting_stake[0] /= opinion_account.num_dxpcore_voted;
               vp_benefactor = voting_stake[2] = detail::vote_recalc_options::benefactor().get_recalced_voting_stake( 
                  voting_stake[2], opinion_account_stats.last_vote_time, *benefactor_recalc_times );
            }

            // update voting power
            d.modify( opinion_account_stats, [=]( account_statistics_object& update_stats ) {
               if (update_stats.vote_tally_time != now)
               {
                  update_stats.vp_all = vp_all;
                  update_stats.vp_active = vp_active;
                  update_stats.vp_dxpcore = vp_dxpcore;
                  update_stats.vp_blockproducer = vp_blockproducer;
                  update_stats.vp_benefactor = vp_benefactor;
                  update_stats.vote_tally_time = now;
               }
               else
               {
                  update_stats.vp_all += vp_all;
                  update_stats.vp_active += vp_active;
                  update_stats.vp_dxpcore += vp_dxpcore;
                  update_stats.vp_blockproducer += vp_blockproducer;
                  update_stats.vp_benefactor += vp_benefactor;
                  // update_stats.vote_tally_time = now; 
               }
            });

            for( vote_id_type id : opinion_account.options.votes )
            {
               uint32_t offset = id.instance();
               uint32_t type = std::min( id.type(), vote_id_type::vote_type::benefactor ); // cap the data
               // if they somehow managed to specify an illegal offset, ignore it.
               if( offset < d._vote_tally_buffer.size() )
                  d._vote_tally_buffer[offset] += voting_stake[type];
            }

            // votes for a number greater than maximum_blockproducer_count are skipped here
            if( voting_stake[1] > 0
                  && opinion_account.options.num_blockproducer <= props.parameters.maximum_blockproducer_count )
            {
               uint16_t offset = opinion_account.options.num_blockproducer / 2;
               d._blockproducer_count_histogram_buffer[offset] += voting_stake[1];
            }
            // votes for a number greater than maximum_dxpcore_count are skipped here
            if( num_dxpcore_voting_stake > 0
                  && opinion_account.options.num_dxpcore <= props.parameters.maximum_dxpcore_count )
            {
               uint16_t offset = opinion_account.options.num_dxpcore / 2;
               d._dxpcore_count_histogram_buffer[offset] += num_dxpcore_voting_stake;
            }

            d._total_voting_stake[0] += num_dxpcore_voting_stake;
            d._total_voting_stake[1] += voting_stake[1];
         }
      }
   } tally_helper(*this);

   perform_account_maintenance( tally_helper );
   
   struct clear_canary {
      clear_canary(vector<uint64_t>& target): target(target){}
      ~clear_canary() { target.clear(); }
   private:
      vector<uint64_t>& target;
   };
   clear_canary a(_blockproducer_count_histogram_buffer),
                b(_dxpcore_count_histogram_buffer),
                c(_vote_tally_buffer);

   update_top_n_authorities(*this);
   update_active_blockproducers();
   update_active_dxpcore_members();
   update_benefactor_votes();

   modify(gpo, [&dgpo](global_property_object& p) {
      // Remove scaling of account registration fee
      p.parameters.get_mutable_fees().get<account_create_operation>().basic_fee >>= p.parameters.account_fee_scale_bitshifts *
            (dgpo.accounts_registered_this_interval / p.parameters.accounts_per_fee_scale);

      if( p.pending_parameters )
      {
         p.parameters = std::move(*p.pending_parameters);
         p.pending_parameters.reset();
      }
   });

   auto next_maintenance_time = dgpo.next_maintenance_time;
   auto maintenance_interval = gpo.parameters.maintenance_interval;

   if( next_maintenance_time <= next_block.timestamp )
   {
      if( next_block.block_num() == 1 )
         next_maintenance_time = time_point_sec() +
               (((next_block.timestamp.sec_since_epoch() / maintenance_interval) + 1) * maintenance_interval);
      else
      {
         // We want to find the smallest k such that next_maintenance_time + k * maintenance_interval > head_block_time()
         //  This implies k > ( head_block_time() - next_maintenance_time ) / maintenance_interval
         //
         // Let y be the right-hand side of this inequality, i.e.
         // y = ( head_block_time() - next_maintenance_time ) / maintenance_interval
         //
         // and let the fractional part f be y-floor(y).  Clearly 0 <= f < 1.
         // We can rewrite f = y-floor(y) as floor(y) = y-f.
         //
         // Clearly k = floor(y)+1 has k > y as desired.  Now we must
         // show that this is the least such k, i.e. k-1 <= y.
         //
         // But k-1 = floor(y)+1-1 = floor(y) = y-f <= y.
         // So this k suffices.
         //
         auto y = (head_block_time() - next_maintenance_time).to_seconds() / maintenance_interval;
         next_maintenance_time += (y+1) * maintenance_interval;
      }
   }

   if( (dgpo.next_maintenance_time < HARDFORK_613_TIME) && (next_maintenance_time >= HARDFORK_613_TIME) )
      deprecate_annual_members(*this);

   // To reset call_price of all call orders, then match by new rule, for hard fork core-343
   bool to_update_and_match_call_orders_for_hf_343 = false;
   if( (dgpo.next_maintenance_time <= HARDFORK_CORE_343_TIME) && (next_maintenance_time > HARDFORK_CORE_343_TIME) )
      to_update_and_match_call_orders_for_hf_343 = true;

   // Process inconsistent price feeds
   if( (dgpo.next_maintenance_time <= HARDFORK_CORE_868_890_TIME) && (next_maintenance_time > HARDFORK_CORE_868_890_TIME) )
      process_hf_868_890( *this, to_update_and_match_call_orders_for_hf_343 );

   // To reset call_price of all call orders, then match by new rule, for hard fork core-1270
   bool to_update_and_match_call_orders_for_hf_1270 = false;
   if( (dgpo.next_maintenance_time <= HARDFORK_CORE_1270_TIME) && (next_maintenance_time > HARDFORK_CORE_1270_TIME) )
      to_update_and_match_call_orders_for_hf_1270 = true;

   // make sure current_supply is less than or equal to max_supply
   if ( dgpo.next_maintenance_time <= HARDFORK_CORE_1465_TIME && next_maintenance_time > HARDFORK_CORE_1465_TIME )
      process_hf_1465(*this);

   // Fix supply issue
   if ( dgpo.next_maintenance_time <= HARDFORK_CORE_2103_TIME && next_maintenance_time > HARDFORK_CORE_2103_TIME )
      process_hf_2103(*this);

   // Update tickets. Note: the new values will take effect only on the next maintenance interval
   if ( dgpo.next_maintenance_time <= HARDFORK_CORE_2262_TIME && next_maintenance_time > HARDFORK_CORE_2262_TIME )
      process_hf_2262(*this);

   modify(dgpo, [last_vote_tally_time, next_maintenance_time](dynamic_global_property_object& d) {
      d.next_maintenance_time = next_maintenance_time;
      d.last_vote_tally_time = last_vote_tally_time;
      d.accounts_registered_this_interval = 0;
   });

   // We need to do it after updated next_maintenance_time, to apply new rules here, for hard fork core-343
   if( to_update_and_match_call_orders_for_hf_343 )
   {
      update_call_orders_hf_343(*this);
      match_call_orders(*this);
   }

   // We need to do it after updated next_maintenance_time, to apply new rules here, for hard fork core-1270.
   if( to_update_and_match_call_orders_for_hf_1270 )
   {
      update_call_orders_hf_1270(*this);
      update_median_feeds(*this);
      match_call_orders(*this);
   }

   process_smarttokens();
   delete_expired_custom_authorities(*this);

   // process_budget needs to run at the bottom because
   //   it needs to know the next_maintenance_time
   process_budget();
}

} }
