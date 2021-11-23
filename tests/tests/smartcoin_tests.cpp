/*
 * Copyright (c) 2021 oxarbitrage, and contributors.
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

#include <boost/test/unit_test.hpp>

#include <graphene/app/database_api.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/exceptions.hpp>

#include <iostream>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;


BOOST_FIXTURE_TEST_SUITE(smartcoin_tests, database_fixture)


BOOST_AUTO_TEST_CASE(bsip36)
{
   try
   {
      /* Issue affects only smartcoins(market pegged assets feeded by active blockproducers or dxpcore members).
       * Test case reproduces, advance to hardfork and check if solved after it.
       */

      /* References:
       * BSIP 36: https://gitlab.com/dxperts/bsips/blob/master/bsip-0036.md
       * and the former: CORE Issue 518: https://gitlab.com/dxperts/dxperts-core/issues/518
       */

      // Create 12 accounts to be blockproducers under our control
      ACTORS( (blockproducer0)(blockproducer1)(blockproducer2)(blockproducer3)(blockproducer4)(blockproducer5)
                   (blockproducer6)(blockproducer7)(blockproducer8)(blockproducer9)(blockproducer10)(blockproducer11) );

      // Upgrade all accounts to LTM
      upgrade_to_lifetime_member(blockproducer0_id);
      upgrade_to_lifetime_member(blockproducer1_id);
      upgrade_to_lifetime_member(blockproducer2_id);
      upgrade_to_lifetime_member(blockproducer3_id);
      upgrade_to_lifetime_member(blockproducer4_id);
      upgrade_to_lifetime_member(blockproducer5_id);
      upgrade_to_lifetime_member(blockproducer6_id);
      upgrade_to_lifetime_member(blockproducer7_id);
      upgrade_to_lifetime_member(blockproducer8_id);
      upgrade_to_lifetime_member(blockproducer9_id);
      upgrade_to_lifetime_member(blockproducer10_id);
      upgrade_to_lifetime_member(blockproducer11_id);

      // Create all the blockproducers
      const blockproducer_id_type blockproducer0_blockproducer_id = create_blockproducer(blockproducer0_id, blockproducer0_private_key).id;
      const blockproducer_id_type blockproducer1_blockproducer_id = create_blockproducer(blockproducer1_id, blockproducer1_private_key).id;
      const blockproducer_id_type blockproducer2_blockproducer_id = create_blockproducer(blockproducer2_id, blockproducer2_private_key).id;
      const blockproducer_id_type blockproducer3_blockproducer_id = create_blockproducer(blockproducer3_id, blockproducer3_private_key).id;
      const blockproducer_id_type blockproducer4_blockproducer_id = create_blockproducer(blockproducer4_id, blockproducer4_private_key).id;
      const blockproducer_id_type blockproducer5_blockproducer_id = create_blockproducer(blockproducer5_id, blockproducer5_private_key).id;
      const blockproducer_id_type blockproducer6_blockproducer_id = create_blockproducer(blockproducer6_id, blockproducer6_private_key).id;
      const blockproducer_id_type blockproducer7_blockproducer_id = create_blockproducer(blockproducer7_id, blockproducer7_private_key).id;
      const blockproducer_id_type blockproducer8_blockproducer_id = create_blockproducer(blockproducer8_id, blockproducer8_private_key).id;
      const blockproducer_id_type blockproducer9_blockproducer_id = create_blockproducer(blockproducer9_id, blockproducer9_private_key).id;
      const blockproducer_id_type blockproducer10_blockproducer_id = create_blockproducer(blockproducer10_id, blockproducer10_private_key).id;
      const blockproducer_id_type blockproducer11_blockproducer_id = create_blockproducer(blockproducer11_id, blockproducer11_private_key).id;

      // Create a vector with private key of all blockproducers, will be used to activate 11 blockproducers at a time
      const vector <fc::ecc::private_key> private_keys = {
            blockproducer0_private_key,
            blockproducer1_private_key,
            blockproducer2_private_key,
            blockproducer3_private_key,
            blockproducer4_private_key,
            blockproducer5_private_key,
            blockproducer6_private_key,
            blockproducer7_private_key,
            blockproducer8_private_key,
            blockproducer9_private_key,
            blockproducer10_private_key
      };

      // create a map with account id and blockproducer id of the first 11 blockproducers
      const flat_map <account_id_type, blockproducer_id_type> blockproducer_map = {
         {blockproducer0_id, blockproducer0_blockproducer_id},
         {blockproducer1_id, blockproducer1_blockproducer_id},
         {blockproducer2_id, blockproducer2_blockproducer_id},
         {blockproducer3_id, blockproducer3_blockproducer_id},
         {blockproducer4_id, blockproducer4_blockproducer_id},
         {blockproducer5_id, blockproducer5_blockproducer_id},
         {blockproducer6_id, blockproducer6_blockproducer_id},
         {blockproducer7_id, blockproducer7_blockproducer_id},
         {blockproducer8_id, blockproducer8_blockproducer_id},
         {blockproducer9_id, blockproducer9_blockproducer_id},
         {blockproducer10_id, blockproducer10_blockproducer_id}
      };

      // Create the asset
      const asset_id_type bit_usd_id = create_smarttoken("USDD").id;

      // Update the asset to be fed by system blockproducers
      asset_update_operation op;
      const asset_object &asset_obj = bit_usd_id(db);
      op.asset_to_update = bit_usd_id;
      op.issuer = asset_obj.issuer;
      op.new_options = asset_obj.options;
      op.new_options.flags &= blockproducer_fed_asset;
      op.new_options.issuer_permissions &= blockproducer_fed_asset;
      trx.operations.push_back(op);
      PUSH_TX(db, trx, ~0);
      generate_block();
      trx.clear();

      // Check current default blockproducers, default chain is configured with 10 blockproducers
      auto blockproducers = db.get_global_properties().active_blockproducers;
      BOOST_CHECK_EQUAL(blockproducers.size(), INITIAL_BLOCKPRODUCER_COUNT);
      BOOST_CHECK_EQUAL(blockproducers.begin()[0].instance.value, 1u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[1].instance.value, 2u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[2].instance.value, 3u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[3].instance.value, 4u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[4].instance.value, 5u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[5].instance.value, 6u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[6].instance.value, 7u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[7].instance.value, 8u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[8].instance.value, 9u);

      // We need to activate 11 blockproducers by voting for each of them.
      // Each blockproducer is voted with incremental stake so last blockproducer created will be the ones with more votes

      // by default we have 9 blockproducers, we need to vote for desired blockproducer count (11) to increase them
      vote_for_dxpcore_and_blockproducers(9, 11);

      int c = 0;
      for (auto l : blockproducer_map) {
         // voting stake have step of 100
         // so vote_for_dxpcore_and_blockproducers() with stake=10 does not affect the expected result
         int stake = 100 * (c + 1);
         transfer(dxpcore_account, l.first, asset(stake));
         {
            account_update_operation op;
            op.account = l.first;
            op.new_options = l.first(db).options;
            op.new_options->votes.insert(l.second(db).vote_id);
            op.new_options->num_blockproducer = std::count_if(op.new_options->votes.begin(), op.new_options->votes.end(),
                                                        [](vote_id_type id) {
                                                           return id.type() == vote_id_type::blockproducer;
                                                        });
            trx.operations.push_back(op);
            sign(trx, private_keys.at(c));
            PUSH_TX(db, trx);
            trx.clear();
         }
         ++c;
      }

      // Trigger the new blockproducers
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // Check my blockproducers are now in control of the system
      blockproducers = db.get_global_properties().active_blockproducers;
      BOOST_CHECK_EQUAL(blockproducers.size(), 11u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[0].instance.value, 11u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[1].instance.value, 12u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[2].instance.value, 13u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[3].instance.value, 14u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[4].instance.value, 15u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[5].instance.value, 16u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[6].instance.value, 17u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[7].instance.value, 18u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[8].instance.value, 19u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[9].instance.value, 20u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[10].instance.value, 21u);

      // Adding 2 feeds with blockproducers 0 and 1, checking if they get inserted
      const asset_object &core = asset_id_type()(db);
      price_feed feed;
      feed.settlement_price = bit_usd_id(db).amount(1) / core.amount(5);
      publish_feed(bit_usd_id(db), blockproducer0_id(db), feed);

      asset_smarttoken_data_object smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 1u);
      auto itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 16u);

      feed.settlement_price = bit_usd_id(db).amount(2) / core.amount(5);
      publish_feed(bit_usd_id(db), blockproducer1_id(db), feed);

      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 2u);
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 16u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 17u);

      // Activate blockproducer11 with voting stake, will kick the blockproducer with less votes(blockproducer0) out of the active list
      transfer(dxpcore_account, blockproducer11_id, asset(1200));
      set_expiration(db, trx);
      {
         account_update_operation op;
         op.account = blockproducer11_id;
         op.new_options = blockproducer11_id(db).options;
         op.new_options->votes.insert(blockproducer11_blockproducer_id(db).vote_id);
         op.new_options->num_blockproducer = std::count_if(op.new_options->votes.begin(), op.new_options->votes.end(),
                                                     [](vote_id_type id) {
                                                        return id.type() == vote_id_type::blockproducer;
                                                     });
         trx.operations.push_back(op);
         sign(trx, blockproducer11_private_key);
         PUSH_TX(db, trx);
         trx.clear();
      }

      // Trigger new blockproducer
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // Check active blockproducer list now
      blockproducers = db.get_global_properties().active_blockproducers;
      BOOST_CHECK_EQUAL(blockproducers.begin()[0].instance.value, 12u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[1].instance.value, 13u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[2].instance.value, 14u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[3].instance.value, 15u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[4].instance.value, 16u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[5].instance.value, 17u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[6].instance.value, 18u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[7].instance.value, 19u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[8].instance.value, 20u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[9].instance.value, 21u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[10].instance.value, 22u);

      // blockproducer0 has been removed but it was a feeder before
      // Feed persist in the blockchain, this reproduces the issue
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 2u);
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 16u);

      // Feed persist after expiration
      const auto feed_lifetime = bit_usd_id(db).smarttoken_data(db).options.feed_lifetime_sec;
      generate_blocks(db.head_block_time() + feed_lifetime + 1);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 2u);
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 16u);

      // Other blockproducers add more feeds
      feed.settlement_price = bit_usd_id(db).amount(4) / core.amount(5);
      publish_feed(bit_usd_id(db), blockproducer2_id(db), feed);
      feed.settlement_price = bit_usd_id(db).amount(3) / core.amount(5);
      publish_feed(bit_usd_id(db), blockproducer3_id(db), feed);

      // But the one from blockproducer0 is never removed
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 4u);
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 16u);

      // Feed from blockproducer1 is also expired but never deleted
      // All feeds should be deleted at this point
      const auto minimum_feeds = bit_usd_id(db).smarttoken_data(db).options.minimum_feeds;
      BOOST_CHECK_EQUAL(minimum_feeds, 1u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 17u);

      // Advancing into HF time
      generate_blocks(HARDFORK_CORE_518_TIME);

      // Advancing to next maint
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      //  All expired feeds are deleted
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 0u);

      // blockproducer1 start feed producing again
      feed.settlement_price = bit_usd_id(db).amount(1) / core.amount(5);
      publish_feed(bit_usd_id(db), blockproducer1_id(db), feed);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 1u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 17u);

      // generate some blocks up to expiration but feed will not be deleted yet as need next maint time
      generate_blocks(itr[0].second.first + feed_lifetime + 1);

      // add another feed with blockproducer2
      feed.settlement_price = bit_usd_id(db).amount(1) / core.amount(5);
      publish_feed(bit_usd_id(db), blockproducer2_id(db), feed);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 2u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 17u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 18u);

      // make the first feed expire
      generate_blocks(itr[0].second.first + feed_lifetime + 1);
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // feed from blockproducer0 expires and gets deleted, feed from blockproducer is on time so persist
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 1u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 18u);

      // expire everything
      generate_blocks(itr[0].second.first + feed_lifetime + 1);
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 0u);

      // add new feed with blockproducer1
      feed.settlement_price = bit_usd_id(db).amount(1) / core.amount(5);
      publish_feed(bit_usd_id(db), blockproducer1_id(db), feed);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 1u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 17u);

      // Reactivate blockproducer0
      transfer(dxpcore_account, blockproducer0_id, asset(1000));
      set_expiration(db, trx);
      {
         account_update_operation op;
         op.account = blockproducer0_id;
         op.new_options = blockproducer0_id(db).options;
         op.new_options->votes.insert(blockproducer0_blockproducer_id(db).vote_id);
         op.new_options->num_blockproducer = std::count_if(op.new_options->votes.begin(), op.new_options->votes.end(),
                                                     [](vote_id_type id) {
                                                        return id.type() == vote_id_type::blockproducer;
                                                     });
         trx.operations.push_back(op);
         sign(trx, blockproducer0_private_key);
         PUSH_TX(db, trx);
         trx.clear();
      }

      // This will deactivate blockproducer1 as it is the one with less votes
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // Checking
      blockproducers = db.get_global_properties().active_blockproducers;
      BOOST_CHECK_EQUAL(blockproducers.begin()[0].instance.value, 11u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[1].instance.value, 13u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[2].instance.value, 14u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[3].instance.value, 15u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[4].instance.value, 16u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[5].instance.value, 17u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[6].instance.value, 18u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[7].instance.value, 19u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[8].instance.value, 20u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[9].instance.value, 21u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[10].instance.value, 22u);

      // feed from blockproducer1 is still here as the blockproducer is no longer a producer but the feed is not yet expired
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 1u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 17u);

      // make feed from blockproducer1 expire
      generate_blocks(itr[0].second.first + feed_lifetime + 1);
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 0u);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(bsip36_update_feed_producers)
{
   try
   {
      /* For MPA fed by non blockproducers or non dxpcore mmembers but by feed producers changes should do nothing */
      ACTORS( (sam)(alice)(paul)(bob) );

      // Create the asset
      const asset_id_type bit_usd_id = create_smarttoken("USDD").id;

      // Update asset issuer
      const asset_object &asset_obj = bit_usd_id(db);
      {
         asset_update_operation op;
         op.asset_to_update = bit_usd_id;
         op.issuer = asset_obj.issuer;
         op.new_issuer = bob_id;
         op.new_options = asset_obj.options;
         op.new_options.flags &= ~blockproducer_fed_asset;
         trx.operations.push_back(op);
         PUSH_TX(db, trx, ~0);
         generate_block();
         trx.clear();
      }

      // Add 3 feed producers for asset
      {
         asset_update_feed_producers_operation op;
         op.asset_to_update = bit_usd_id;
         op.issuer = bob_id;
         op.new_feed_producers = {sam_id, alice_id, paul_id};
         trx.operations.push_back(op);
         sign(trx, bob_private_key);
         PUSH_TX(db, trx);
         generate_block();
         trx.clear();
      }

      // Dxperts will create entries in the field feed after feed producers are added
      auto smarttoken_data = bit_usd_id(db).smarttoken_data(db);

      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 3u);
      auto itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 16u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 17u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 18u);

      // Removing a feed producer
      {
         asset_update_feed_producers_operation op;
         op.asset_to_update = bit_usd_id;
         op.issuer = bob_id;
         op.new_feed_producers = {alice_id, paul_id};
         trx.operations.push_back(op);
         sign(trx, bob_private_key);
         PUSH_TX(db, trx);
         generate_block();
         trx.clear();
      }

      // Feed for removed producer is removed
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 2u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 17u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 18u);

      // Feed persist after expiration
      const auto feed_lifetime = bit_usd_id(db).smarttoken_data(db).options.feed_lifetime_sec;
      generate_blocks(db.head_block_time() + feed_lifetime + 1);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 2u);
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 17u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 18u);

      // Advancing into HF time
      generate_blocks(HARDFORK_CORE_518_TIME);

      // Advancing to next maint
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // Expired feeds persist, no changes
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 2u);
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 17u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 18u);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(bsip36_additional)
{
   try
   {
      /* Check impact of bsip36 with multiple feeds */
      INVOKE( bsip36 );

      // get the stuff needed from invoked test
      const asset_id_type bit_usd_id = get_token("USDD").id;
      const asset_id_type core_id = asset_id_type();
      const account_id_type blockproducer5_id= get_account("blockproducer5").id;
      const account_id_type blockproducer6_id= get_account("blockproducer6").id;
      const account_id_type blockproducer7_id= get_account("blockproducer7").id;
      const account_id_type blockproducer8_id= get_account("blockproducer8").id;
      const account_id_type blockproducer9_id= get_account("blockproducer9").id;
      const account_id_type blockproducer10_id= get_account("blockproducer10").id;


      set_expiration( db, trx );

      // changing lifetime feed to 5 days
      // maint interval default is every 1 day
      {
         asset_update_smarttoken_operation op;
         op.new_options.minimum_feeds = 3;
         op.new_options.feed_lifetime_sec = 86400 * 5;
         op.asset_to_update = bit_usd_id;
         op.issuer = bit_usd_id(db).issuer;
         trx.operations.push_back(op);
         PUSH_TX(db, trx, ~0);
         generate_block();
         trx.clear();
      }

      price_feed feed;
      feed.settlement_price = bit_usd_id(db).amount(1) / core_id(db).amount(5);
      publish_feed(bit_usd_id(db), blockproducer5_id(db), feed);
      auto smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 1u);
      auto itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 21u);

      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();

      feed.settlement_price = bit_usd_id(db).amount(1) / core_id(db).amount(5);
      publish_feed(bit_usd_id(db), blockproducer6_id(db), feed);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 2u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 21u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 22u);

      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();

      feed.settlement_price = bit_usd_id(db).amount(1) / core_id(db).amount(5);
      publish_feed(bit_usd_id(db), blockproducer7_id(db), feed);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 3u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 21u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 22u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 23u);

      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();

      feed.settlement_price = bit_usd_id(db).amount(1) / core_id(db).amount(5);
      publish_feed(bit_usd_id(db), blockproducer8_id(db), feed);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 4u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 21u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 22u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 23u);
      BOOST_CHECK_EQUAL(itr[3].first.instance.value, 24u);

      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();

      feed.settlement_price = bit_usd_id(db).amount(1) / core_id(db).amount(5);
      publish_feed(bit_usd_id(db), blockproducer9_id(db), feed);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 5u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 21u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 22u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 23u);
      BOOST_CHECK_EQUAL(itr[3].first.instance.value, 24u);
      BOOST_CHECK_EQUAL(itr[4].first.instance.value, 25u);

      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();

      feed.settlement_price = bit_usd_id(db).amount(1) / core_id(db).amount(5);
      publish_feed(bit_usd_id(db), blockproducer10_id(db), feed);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 6u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 21u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 22u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 23u);
      BOOST_CHECK_EQUAL(itr[3].first.instance.value, 24u);
      BOOST_CHECK_EQUAL(itr[4].first.instance.value, 25u);
      BOOST_CHECK_EQUAL(itr[5].first.instance.value, 26u);

      // make the older feed expire
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();

      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 5u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 22u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 23u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 24u);
      BOOST_CHECK_EQUAL(itr[3].first.instance.value, 25u);
      BOOST_CHECK_EQUAL(itr[4].first.instance.value, 26u);

      // make older 2 feeds expire
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();

      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 3u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 24u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 25u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 26u);

      // blockproducer5 add new feed, feeds are sorted by blockproducer_id not by feed_time
      feed.settlement_price = bit_usd_id(db).amount(1) / core_id(db).amount(5);
      publish_feed(bit_usd_id(db), blockproducer5_id(db), feed);
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 4u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 21u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 24u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 25u);
      BOOST_CHECK_EQUAL(itr[3].first.instance.value, 26u);

      // another feed expires
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 3u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 21u);
      BOOST_CHECK_EQUAL(itr[1].first.instance.value, 25u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 26u);

      // another feed expires
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();
      smarttoken_data = bit_usd_id(db).smarttoken_data(db);
      BOOST_CHECK_EQUAL(smarttoken_data.feeds.size(), 2u);
      itr = smarttoken_data.feeds.begin();
      BOOST_CHECK_EQUAL(itr[0].first.instance.value, 21u);
      BOOST_CHECK_EQUAL(itr[2].first.instance.value, 26u);

      // and so on

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
