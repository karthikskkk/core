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

#include <graphene/chain/database.hpp>
#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/blockproducer_object.hpp>
#include <graphene/chain/blockproducer_schedule_object.hpp>

#include <fc/popcount.hpp>

namespace graphene { namespace chain {

using boost::container::flat_set;

blockproducer_id_type database::get_scheduled_blockproducer( uint32_t slot_num )const
{
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();
   const blockproducer_schedule_object& wso = get_blockproducer_schedule_object();
   uint64_t current_aslot = dpo.current_aslot + slot_num;
   return wso.current_shuffled_blockproducers[ current_aslot % wso.current_shuffled_blockproducers.size() ];
}

fc::time_point_sec database::get_slot_time(uint32_t slot_num)const
{
   if( slot_num == 0 )
      return fc::time_point_sec();

   auto interval = block_interval();
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();

   if( head_block_num() == 0 )
   {
      // n.b. first block is at genesis_time plus one block interval
      fc::time_point_sec genesis_time = dpo.time;
      return genesis_time + slot_num * interval;
   }

   int64_t head_block_abs_slot = head_block_time().sec_since_epoch() / interval;
   fc::time_point_sec head_slot_time(head_block_abs_slot * interval);

   const global_property_object& gpo = get_global_properties();

   if( dpo.dynamic_flags & dynamic_global_property_object::maintenance_flag )
      slot_num += gpo.parameters.maintenance_skip_slots;

   // "slot 0" is head_slot_time
   // "slot 1" is head_slot_time,
   //   plus maint interval if head block is a maint block
   //   plus block interval if head block is not a maint block
   return head_slot_time + (slot_num * interval);
}

uint32_t database::get_slot_at_time(fc::time_point_sec when)const
{
   fc::time_point_sec first_slot_time = get_slot_time( 1 );
   if( when < first_slot_time )
      return 0;
   return (when - first_slot_time).to_seconds() / block_interval() + 1;
}

uint32_t database::update_blockproducer_missed_blocks( const signed_block& b )
{
   uint32_t missed_blocks = get_slot_at_time( b.timestamp );
   FC_ASSERT( missed_blocks != 0, "Trying to push double-produced block onto current block?!" );
   missed_blocks--;
   const auto& blockproducers = blockproducer_schedule_id_type()(*this).current_shuffled_blockproducers;
   if( missed_blocks < blockproducers.size() )
      for( uint32_t i = 0; i < missed_blocks; ++i ) {
         const auto& blockproducer_missed = get_scheduled_blockproducer( i+1 )(*this);
         modify( blockproducer_missed, []( blockproducer_object& w ) {
            w.total_missed++;
         });
      }
   return missed_blocks;
}

uint32_t database::blockproducer_participation_rate()const
{
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();
   return uint64_t(GRAPHENE_100_PERCENT) * fc::popcount(dpo.recent_slots_filled) / 128;
}

void database::update_blockproducer_schedule()
{
   const blockproducer_schedule_object& wso = get_blockproducer_schedule_object();
   const global_property_object& gpo = get_global_properties();

   if( head_block_num() % gpo.active_blockproducers.size() == 0 )
   {
      modify( wso, [&]( blockproducer_schedule_object& _wso )
      {
         _wso.current_shuffled_blockproducers.clear();
         _wso.current_shuffled_blockproducers.reserve( gpo.active_blockproducers.size() );

         for( const blockproducer_id_type& w : gpo.active_blockproducers )
            _wso.current_shuffled_blockproducers.push_back( w );

         auto now_hi = uint64_t(head_block_time().sec_since_epoch()) << 32;
         for( uint32_t i = 0; i < _wso.current_shuffled_blockproducers.size(); ++i )
         {
            /// High performance random generator
            /// http://xorshift.di.unimi.it/
            uint64_t k = now_hi + uint64_t(i)*2685821657736338717ULL;
            k ^= (k >> 12);
            k ^= (k << 25);
            k ^= (k >> 27);
            k *= 2685821657736338717ULL;

            uint32_t jmax = _wso.current_shuffled_blockproducers.size() - i;
            uint32_t j = i + k%jmax;
            std::swap( _wso.current_shuffled_blockproducers[i],
                       _wso.current_shuffled_blockproducers[j] );
         }
      });
   }
}

} }
