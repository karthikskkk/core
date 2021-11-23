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
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>

#include <iostream>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE(voting_tests, database_fixture)

BOOST_FIXTURE_TEST_CASE(dxpcore_account_initialization_test, database_fixture)
{
   try
   {
      // Check current default dxpcore
      // By default chain is configured with INITIAL_DXPCORE_MEMBER_COUNT=9 members
      const auto &dxpcore_members = db.get_global_properties().active_dxpcore_members;
      const auto &dxpcore = dxpcore_account(db);

      BOOST_CHECK_EQUAL(dxpcore_members.size(), INITIAL_DXPCORE_MEMBER_COUNT);
      BOOST_CHECK_EQUAL(dxpcore.active.num_auths(), INITIAL_DXPCORE_MEMBER_COUNT);

      generate_blocks(HARDFORK_533_TIME);
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();
      set_expiration(db, trx);

      // Check that dxpcore not changed after 533 hardfork
      // vote counting method changed, but any votes are absent
      const auto &dxpcore_members_after_hf533 = db.get_global_properties().active_dxpcore_members;
      const auto &dxpcore_after_hf533 = dxpcore_account(db);
      BOOST_CHECK_EQUAL(dxpcore_members_after_hf533.size(), INITIAL_DXPCORE_MEMBER_COUNT);
      BOOST_CHECK_EQUAL(dxpcore_after_hf533.active.num_auths(), INITIAL_DXPCORE_MEMBER_COUNT);

      // You can't use uninitialized dxpcore after 533 hardfork
      // when any user with stake created (create_account method automatically set up votes for dxpcore)
      // dxpcore is incomplete and consist of random active members
      ACTOR(alice);
      fund(alice);
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      const auto &dxpcore_after_hf533_with_stake = dxpcore_account(db);
      BOOST_CHECK_LT(dxpcore_after_hf533_with_stake.active.num_auths(), INITIAL_DXPCORE_MEMBER_COUNT);

      // Initialize dxpcore by voting for each memeber and for desired count
      vote_for_dxpcore_and_blockproducers(INITIAL_DXPCORE_MEMBER_COUNT, INITIAL_BLOCKPRODUCER_COUNT);
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      const auto &dxpcore_members_after_hf533_and_init = db.get_global_properties().active_dxpcore_members;
      const auto &dxpcore_after_hf533_and_init = dxpcore_account(db);
      BOOST_CHECK_EQUAL(dxpcore_members_after_hf533_and_init.size(), INITIAL_DXPCORE_MEMBER_COUNT);
      BOOST_CHECK_EQUAL(dxpcore_after_hf533_and_init.active.num_auths(), INITIAL_DXPCORE_MEMBER_COUNT);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(put_my_blockproducers)
{
   try
   {
      ACTORS((blockproducer0)(blockproducer1)(blockproducer2)(blockproducer3)(blockproducer4)(blockproducer5)(blockproducer6)(blockproducer7)(blockproducer8)(blockproducer9)(blockproducer10)(blockproducer11)(blockproducer12)(blockproducer13));

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
      upgrade_to_lifetime_member(blockproducer12_id);
      upgrade_to_lifetime_member(blockproducer13_id);

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
      const blockproducer_id_type blockproducer12_blockproducer_id = create_blockproducer(blockproducer12_id, blockproducer12_private_key).id;
      const blockproducer_id_type blockproducer13_blockproducer_id = create_blockproducer(blockproducer13_id, blockproducer13_private_key).id;

      // Create a vector with private key of all blockproducers, will be used to activate 9 blockproducers at a time
      const vector<fc::ecc::private_key> private_keys = {
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
          blockproducer10_private_key,
          blockproducer11_private_key,
          blockproducer12_private_key,
          blockproducer13_private_key

      };

      // create a map with account id and blockproducer id
      const flat_map<account_id_type, blockproducer_id_type> blockproducer_map = {
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
          {blockproducer10_id, blockproducer10_blockproducer_id},
          {blockproducer11_id, blockproducer11_blockproducer_id},
          {blockproducer12_id, blockproducer12_blockproducer_id},
          {blockproducer13_id, blockproducer13_blockproducer_id}};

      // Check current default blockproducers, default chain is configured with 9 blockproducers
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

      // Activate all blockproducers
      // Each blockproducer is voted with incremental stake so last blockproducer created will be the ones with more votes
      int c = 0;
      for (auto l : blockproducer_map)
      {
         int stake = 100 + c + 10;
         transfer(dxpcore_account, l.first, asset(stake));
         {
            set_expiration(db, trx);
            account_update_operation op;
            op.account = l.first;
            op.new_options = l.first(db).options;
            op.new_options->votes.insert(l.second(db).vote_id);

            trx.operations.push_back(op);
            sign(trx, private_keys.at(c));
            PUSH_TX(db, trx);
            trx.clear();
         }
         ++c;
      }

      // Trigger the new blockproducers
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();

      // Check my blockproducers are now in control of the system
      blockproducers = db.get_global_properties().active_blockproducers;
      BOOST_CHECK_EQUAL(blockproducers.size(), INITIAL_BLOCKPRODUCER_COUNT);
      BOOST_CHECK_EQUAL(blockproducers.begin()[0].instance.value, 16u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[1].instance.value, 17u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[2].instance.value, 18u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[3].instance.value, 19u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[4].instance.value, 20u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[5].instance.value, 21u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[6].instance.value, 22u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[7].instance.value, 23u);
      BOOST_CHECK_EQUAL(blockproducers.begin()[8].instance.value, 24u);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(track_votes_blockproducers_enabled)
{
   try
   {
      graphene::app::database_api db_api1(db);

      INVOKE(put_my_blockproducers);

      const account_id_type blockproducer1_id = get_account("blockproducer1").id;
      auto blockproducer1_object = db_api1.get_blockproducer_by_account(blockproducer1_id(db).name);
      BOOST_CHECK_EQUAL(blockproducer1_object->total_votes, 111u);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(track_votes_blockproducers_disabled)
{
   try
   {
      graphene::app::database_api db_api1(db);

      INVOKE(put_my_blockproducers);

      const account_id_type blockproducer1_id = get_account("blockproducer1").id;
      auto blockproducer1_object = db_api1.get_blockproducer_by_account(blockproducer1_id(db).name);
      BOOST_CHECK_EQUAL(blockproducer1_object->total_votes, 0u);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(put_my_dxpcore_members)
{
   try
   {
      ACTORS((dxpcore0)(dxpcore1)(dxpcore2)(dxpcore3)(dxpcore4)(dxpcore5)(dxpcore6)(dxpcore7)(dxpcore8)(dxpcore9)(dxpcore10)(dxpcore11)(dxpcore12)(dxpcore13));

      // Upgrade all accounts to LTM
      upgrade_to_lifetime_member(dxpcore0_id);
      upgrade_to_lifetime_member(dxpcore1_id);
      upgrade_to_lifetime_member(dxpcore2_id);
      upgrade_to_lifetime_member(dxpcore3_id);
      upgrade_to_lifetime_member(dxpcore4_id);
      upgrade_to_lifetime_member(dxpcore5_id);
      upgrade_to_lifetime_member(dxpcore6_id);
      upgrade_to_lifetime_member(dxpcore7_id);
      upgrade_to_lifetime_member(dxpcore8_id);
      upgrade_to_lifetime_member(dxpcore9_id);
      upgrade_to_lifetime_member(dxpcore10_id);
      upgrade_to_lifetime_member(dxpcore11_id);
      upgrade_to_lifetime_member(dxpcore12_id);
      upgrade_to_lifetime_member(dxpcore13_id);

      // Create all the dxpcore
      const dxpcore_member_id_type dxpcore0_dxpcore_id = create_dxpcore_member(dxpcore0_id(db)).id;
      const dxpcore_member_id_type dxpcore1_dxpcore_id = create_dxpcore_member(dxpcore1_id(db)).id;
      const dxpcore_member_id_type dxpcore2_dxpcore_id = create_dxpcore_member(dxpcore2_id(db)).id;
      const dxpcore_member_id_type dxpcore3_dxpcore_id = create_dxpcore_member(dxpcore3_id(db)).id;
      const dxpcore_member_id_type dxpcore4_dxpcore_id = create_dxpcore_member(dxpcore4_id(db)).id;
      const dxpcore_member_id_type dxpcore5_dxpcore_id = create_dxpcore_member(dxpcore5_id(db)).id;
      const dxpcore_member_id_type dxpcore6_dxpcore_id = create_dxpcore_member(dxpcore6_id(db)).id;
      const dxpcore_member_id_type dxpcore7_dxpcore_id = create_dxpcore_member(dxpcore7_id(db)).id;
      const dxpcore_member_id_type dxpcore8_dxpcore_id = create_dxpcore_member(dxpcore8_id(db)).id;
      const dxpcore_member_id_type dxpcore9_dxpcore_id = create_dxpcore_member(dxpcore9_id(db)).id;
      const dxpcore_member_id_type dxpcore10_dxpcore_id = create_dxpcore_member(dxpcore10_id(db)).id;
      const dxpcore_member_id_type dxpcore11_dxpcore_id = create_dxpcore_member(dxpcore11_id(db)).id;
      const dxpcore_member_id_type dxpcore12_dxpcore_id = create_dxpcore_member(dxpcore12_id(db)).id;
      const dxpcore_member_id_type dxpcore13_dxpcore_id = create_dxpcore_member(dxpcore13_id(db)).id;

      // Create a vector with private key of all dxpcore members, will be used to activate 9 members at a time
      const vector<fc::ecc::private_key> private_keys = {
          dxpcore0_private_key,
          dxpcore1_private_key,
          dxpcore2_private_key,
          dxpcore3_private_key,
          dxpcore4_private_key,
          dxpcore5_private_key,
          dxpcore6_private_key,
          dxpcore7_private_key,
          dxpcore8_private_key,
          dxpcore9_private_key,
          dxpcore10_private_key,
          dxpcore11_private_key,
          dxpcore12_private_key,
          dxpcore13_private_key};

      // create a map with account id and dxpcore member id
      const flat_map<account_id_type, dxpcore_member_id_type> dxpcore_map = {
          {dxpcore0_id, dxpcore0_dxpcore_id},
          {dxpcore1_id, dxpcore1_dxpcore_id},
          {dxpcore2_id, dxpcore2_dxpcore_id},
          {dxpcore3_id, dxpcore3_dxpcore_id},
          {dxpcore4_id, dxpcore4_dxpcore_id},
          {dxpcore5_id, dxpcore5_dxpcore_id},
          {dxpcore6_id, dxpcore6_dxpcore_id},
          {dxpcore7_id, dxpcore7_dxpcore_id},
          {dxpcore8_id, dxpcore8_dxpcore_id},
          {dxpcore9_id, dxpcore9_dxpcore_id},
          {dxpcore10_id, dxpcore10_dxpcore_id},
          {dxpcore11_id, dxpcore11_dxpcore_id},
          {dxpcore12_id, dxpcore12_dxpcore_id},
          {dxpcore13_id, dxpcore13_dxpcore_id}};

      // Check current default dxpcore, default chain is configured with 9 dxpcore members
      auto dxpcore_members = db.get_global_properties().active_dxpcore_members;

      BOOST_CHECK_EQUAL(dxpcore_members.size(), INITIAL_DXPCORE_MEMBER_COUNT);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[0].instance.value, 0u);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[1].instance.value, 1u);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[2].instance.value, 2u);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[3].instance.value, 3u);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[4].instance.value, 4u);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[5].instance.value, 5u);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[6].instance.value, 6u);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[7].instance.value, 7u);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[8].instance.value, 8u);

      // Activate all dxpcore
      // Each dxpcore is voted with incremental stake so last member created will be the ones with more votes
      int c = 0;
      for (auto dxpcore : dxpcore_map)
      {
         int stake = 100 + c + 10;
         transfer(dxpcore_account, dxpcore.first, asset(stake));
         {
            set_expiration(db, trx);
            account_update_operation op;
            op.account = dxpcore.first;
            op.new_options = dxpcore.first(db).options;

            op.new_options->votes.clear();
            op.new_options->votes.insert(dxpcore.second(db).vote_id);
            op.new_options->num_dxpcore = 1;

            trx.operations.push_back(op);
            sign(trx, private_keys.at(c));
            PUSH_TX(db, trx);
            trx.clear();
         }
         ++c;
      }

      // Trigger the new dxpcore
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      generate_block();

      // Check my blockproducers are now in control of the system
      dxpcore_members = db.get_global_properties().active_dxpcore_members;
      std::sort(dxpcore_members.begin(), dxpcore_members.end());

      BOOST_CHECK_EQUAL(dxpcore_members.size(), INITIAL_DXPCORE_MEMBER_COUNT);

      // Check my dxpcore members are now in control of the system
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[0].instance.value, 15);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[1].instance.value, 16);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[2].instance.value, 17);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[3].instance.value, 18);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[4].instance.value, 19);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[5].instance.value, 20);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[6].instance.value, 21);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[7].instance.value, 22);
      BOOST_CHECK_EQUAL(dxpcore_members.begin()[8].instance.value, 23);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(track_votes_dxpcore_enabled)
{
   try
   {
      graphene::app::database_api db_api1(db);

      INVOKE(put_my_dxpcore_members);

      const account_id_type dxpcore1_id = get_account("dxpcore1").id;
      auto dxpcore1_object = db_api1.get_dxpcore_member_by_account(dxpcore1_id(db).name);
      BOOST_CHECK_EQUAL(dxpcore1_object->total_votes, 111u);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(track_votes_dxpcore_disabled)
{
   try
   {
      graphene::app::database_api db_api1(db);

      INVOKE(put_my_dxpcore_members);

      const account_id_type dxpcore1_id = get_account("dxpcore1").id;
      auto dxpcore1_object = db_api1.get_dxpcore_member_by_account(dxpcore1_id(db).name);
      BOOST_CHECK_EQUAL(dxpcore1_object->total_votes, 0u);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(invalid_voting_account)
{
   try
   {
      ACTORS((alice));

      account_id_type invalid_account_id((uint64_t)999999);

      BOOST_CHECK(!db.find(invalid_account_id));

      graphene::chain::account_update_operation op;
      op.account = alice_id;
      op.new_options = alice.options;
      op.new_options->voting_account = invalid_account_id;
      trx.operations.push_back(op);
      sign(trx, alice_private_key);

      GRAPHENE_REQUIRE_THROW(PUSH_TX(db, trx, ~0), fc::exception);
   }
   FC_LOG_AND_RETHROW()
}
BOOST_AUTO_TEST_CASE(last_voting_date)
{
   try
   {
      ACTORS((alice));

      transfer(dxpcore_account, alice_id, asset(100));

      // we are going to vote for this blockproducer
      auto blockproducer1 = blockproducer_id_type(1)(db);

      auto stats_obj = db.get_account_stats_by_owner(alice_id);
      BOOST_CHECK_EQUAL(stats_obj.last_vote_time.sec_since_epoch(), 0u);

      // alice votes
      graphene::chain::account_update_operation op;
      op.account = alice_id;
      op.new_options = alice.options;
      op.new_options->votes.insert(blockproducer1.vote_id);
      trx.operations.push_back(op);
      sign(trx, alice_private_key);
      PUSH_TX(db, trx, ~0);

      auto now = db.head_block_time().sec_since_epoch();

      // last_vote_time is updated for alice
      stats_obj = db.get_account_stats_by_owner(alice_id);
      BOOST_CHECK_EQUAL(stats_obj.last_vote_time.sec_since_epoch(), now);
   }
   FC_LOG_AND_RETHROW()
}
BOOST_AUTO_TEST_CASE(last_voting_date_proxy)
{
   try
   {
      ACTORS((alice)(proxy)(bob));

      transfer(dxpcore_account, alice_id, asset(100));
      transfer(dxpcore_account, bob_id, asset(200));
      transfer(dxpcore_account, proxy_id, asset(300));

      generate_block();

      // blockproducer to vote for
      auto blockproducer1 = blockproducer_id_type(1)(db);

      // round1: alice changes proxy, this is voting activity
      {
         graphene::chain::account_update_operation op;
         op.account = alice_id;
         op.new_options = alice_id(db).options;
         op.new_options->voting_account = proxy_id;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         PUSH_TX(db, trx, ~0);
      }
      // alice last_vote_time is updated
      auto alice_stats_obj = db.get_account_stats_by_owner(alice_id);
      auto round1 = db.head_block_time().sec_since_epoch();
      BOOST_CHECK_EQUAL(alice_stats_obj.last_vote_time.sec_since_epoch(), round1);

      generate_block();

      // round 2: alice update account but no proxy or voting changes are done
      {
         graphene::chain::account_update_operation op;
         op.account = alice_id;
         op.new_options = alice_id(db).options;
         trx.operations.push_back(op);
         sign(trx, alice_private_key);
         set_expiration(db, trx);
         PUSH_TX(db, trx, ~0);
      }
      // last_vote_time is not updated
      alice_stats_obj = db.get_account_stats_by_owner(alice_id);
      BOOST_CHECK_EQUAL(alice_stats_obj.last_vote_time.sec_since_epoch(), round1);

      generate_block();

      // round 3: bob votes
      {
         graphene::chain::account_update_operation op;
         op.account = bob_id;
         op.new_options = bob_id(db).options;
         op.new_options->votes.insert(blockproducer1.vote_id);
         trx.operations.push_back(op);
         sign(trx, bob_private_key);
         set_expiration(db, trx);
         PUSH_TX(db, trx, ~0);
      }

      // last_vote_time for bob is updated as he voted
      auto round3 = db.head_block_time().sec_since_epoch();
      auto bob_stats_obj = db.get_account_stats_by_owner(bob_id);
      BOOST_CHECK_EQUAL(bob_stats_obj.last_vote_time.sec_since_epoch(), round3);

      generate_block();

      // round 4: proxy votes
      {
         graphene::chain::account_update_operation op;
         op.account = proxy_id;
         op.new_options = proxy_id(db).options;
         op.new_options->votes.insert(blockproducer1.vote_id);
         trx.operations.push_back(op);
         sign(trx, proxy_private_key);
         PUSH_TX(db, trx, ~0);
      }

      // proxy just voted so the last_vote_time is updated
      auto round4 = db.head_block_time().sec_since_epoch();
      auto proxy_stats_obj = db.get_account_stats_by_owner(proxy_id);
      BOOST_CHECK_EQUAL(proxy_stats_obj.last_vote_time.sec_since_epoch(), round4);

      // alice haves proxy, proxy votes but last_vote_time is not updated for alice
      alice_stats_obj = db.get_account_stats_by_owner(alice_id);
      BOOST_CHECK_EQUAL(alice_stats_obj.last_vote_time.sec_since_epoch(), round1);

      // bob haves nothing to do with proxy so last_vote_time is not updated
      bob_stats_obj = db.get_account_stats_by_owner(bob_id);
      BOOST_CHECK_EQUAL(bob_stats_obj.last_vote_time.sec_since_epoch(), round3);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(blockproducer_votes_calculation)
{
   try
   {
      auto original_wits = db.get_global_properties().active_blockproducers;

      INVOKE(put_my_blockproducers);

      GET_ACTOR(blockproducer0);
      GET_ACTOR(blockproducer1);
      GET_ACTOR(blockproducer2);
      GET_ACTOR(blockproducer3);
      GET_ACTOR(blockproducer4);
      GET_ACTOR(blockproducer5);
      GET_ACTOR(blockproducer6);
      GET_ACTOR(blockproducer7);
      GET_ACTOR(blockproducer8);
      GET_ACTOR(blockproducer9);
      GET_ACTOR(blockproducer10);
      GET_ACTOR(blockproducer11);
      GET_ACTOR(blockproducer12);
      GET_ACTOR(blockproducer13);

      graphene::app::database_api db_api1(db);

      vector<account_id_type> wit_account_ids = {blockproducer0_id, blockproducer1_id, blockproducer2_id, blockproducer3_id,
                                                 blockproducer4_id, blockproducer5_id, blockproducer6_id, blockproducer7_id,
                                                 blockproducer8_id, blockproducer9_id, blockproducer10_id, blockproducer11_id,
                                                 blockproducer12_id, blockproducer13_id};
      vector<blockproducer_id_type> wit_ids;
      size_t total = wit_account_ids.size();

      for (size_t i = 0; i < total; ++i)
      {
         auto wit_object = db_api1.get_blockproducer_by_account(wit_account_ids[i](db).name);
         BOOST_REQUIRE(wit_object.valid());
         wit_ids.push_back(wit_object->id);
      }

      generate_blocks(HARDFORK_CORE_2103_TIME - 750 * 86400);
      set_expiration(db, trx);

      // refresh last_vote_time
      for (size_t i = 0; i < total; ++i)
      {
         account_id_type voter = wit_account_ids[total - i - 1];

         account_update_operation op;
         op.account = voter;
         op.new_options = op.account(db).options;
         op.new_options->voting_account = account_id_type();

         trx.operations.clear();
         trx.operations.push_back(op);
         PUSH_TX(db, trx, ~0);

         op.new_options->voting_account = GRAPHENE_PROXY_TO_SELF_ACCOUNT;
         trx.operations.clear();
         trx.operations.push_back(op);
         PUSH_TX(db, trx, ~0);

         trx.clear();

         generate_blocks(db.head_block_time() + 45 * 86400);
         set_expiration(db, trx);
      }

      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(wit_ids[i](db).total_votes, 110u + i);
      }

      generate_blocks(HARDFORK_CORE_2103_TIME);
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      set_expiration(db, trx);

      uint64_t expected_votes[14];

      expected_votes[0] = 110;                // 750 - 45 * 13 = 165 days
      expected_votes[1] = 111;                // 210 days
      expected_votes[2] = 112;                // 255 days
      expected_votes[3] = 113;                // 300 days
      expected_votes[4] = 114;                // 345 days
      expected_votes[5] = 115 - 115 / 8;      // 390 days
      expected_votes[6] = 116 - 116 * 2 / 8;  // 435 days
      expected_votes[7] = 117 - 117 * 3 / 8;  // 480 days
      expected_votes[8] = 118 - 118 * 4 / 8;  // 525 days
      expected_votes[9] = 119 - 119 * 5 / 8;  // 570 days
      expected_votes[10] = 120 - 120 * 6 / 8; // 615 days
      expected_votes[11] = 121 - 121 * 7 / 8; // 660 days
      expected_votes[12] = 0;                 // 705 days
      expected_votes[13] = 0;                 // 750 days

      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(wit_ids[i](db).total_votes, expected_votes[i]);
      }

      flat_set<blockproducer_id_type> expected_active_blockproducers = {wit_ids[0], wit_ids[1], wit_ids[2],
                                                                        wit_ids[3], wit_ids[4], wit_ids[5],
                                                                        wit_ids[6], wit_ids[7], wit_ids[8]};
      BOOST_CHECK(db.get_global_properties().active_blockproducers == expected_active_blockproducers);

      // new vote
      {
         account_update_operation op;
         op.account = wit_account_ids[12];
         op.new_options = op.account(db).options;
         op.new_options->votes.insert(wit_ids[8](db).vote_id);

         trx.operations.clear();
         trx.operations.push_back(op);
         PUSH_TX(db, trx, ~0);
      }

      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      expected_votes[8] += 122;
      expected_votes[12] = 122;
      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(wit_ids[i](db).total_votes, expected_votes[i]);
      }

      expected_active_blockproducers = {wit_ids[0], wit_ids[1], wit_ids[2],
                                        wit_ids[3], wit_ids[4], wit_ids[5],
                                        wit_ids[6], wit_ids[8], wit_ids[12]};
      BOOST_CHECK(db.get_global_properties().active_blockproducers == expected_active_blockproducers);

      // create some tickets
      create_ticket(wit_account_ids[4], lock_forever, asset(40));
      create_ticket(wit_account_ids[7], lock_forever, asset(30));
      create_ticket(wit_account_ids[7], lock_720_days, asset(20));

      auto tick_start_time = db.head_block_time();

      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // votes doesn't change
      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(wit_ids[i](db).total_votes, expected_votes[i]);
      }
      BOOST_CHECK(db.get_global_properties().active_blockproducers == expected_active_blockproducers);

      // some days passed
      generate_blocks(tick_start_time + fc::days(15));
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // check votes
      expected_votes[0] = 110;                           // 180 days
      expected_votes[1] = 111;                           // 225 days
      expected_votes[2] = 112;                           // 270 days
      expected_votes[3] = 113;                           // 315 days
      expected_votes[4] = 114 + 40 - (114 + 40) / 8;     // 360 days
      expected_votes[5] = 115 - 115 * 2 / 8;             // 405 days
      expected_votes[6] = 116 - 116 * 3 / 8;             // 450 days, 73
      expected_votes[7] = 117 + 50 - (117 + 50) * 4 / 8; // 495 days, 84
      expected_votes[8] = 118 - 118 * 5 / 8 + 122;       // 540 days
      expected_votes[9] = 119 - 119 * 6 / 8;             // 585 days
      expected_votes[10] = 120 - 120 * 7 / 8;            // 630 days
      expected_votes[11] = 0;                            // 675 days
      expected_votes[12] = 122;                          // 15 days
      expected_votes[13] = 0;                            // 765 days

      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(wit_ids[i](db).total_votes, expected_votes[i]);
      }

      expected_active_blockproducers = {wit_ids[0], wit_ids[1], wit_ids[2],
                                        wit_ids[3], wit_ids[4], wit_ids[5],
                                        wit_ids[7], wit_ids[8], wit_ids[12]};
      BOOST_CHECK(db.get_global_properties().active_blockproducers == expected_active_blockproducers);

      // some days passed
      generate_blocks(tick_start_time + fc::days(30));
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // check votes
      expected_votes[4] = 114 + 40 * 3 - (114 + 40 * 3) / 8;     // 375 days
      expected_votes[7] = 117 + 50 * 3 - (117 + 50 * 3) * 4 / 8; // 510 days
      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(wit_ids[i](db).total_votes, expected_votes[i]);
      }
      BOOST_CHECK(db.get_global_properties().active_blockproducers == expected_active_blockproducers);

      // some days passed
      generate_blocks(tick_start_time + fc::days(45));
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // check votes
      expected_votes[4] = 114 + 40 * 7 - (114 + 40 * 7) / 8;     // 390 days
      expected_votes[7] = 117 + 50 * 7 - (117 + 50 * 7) * 4 / 8; // 525 days
      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(wit_ids[i](db).total_votes, expected_votes[i]);
      }
      BOOST_CHECK(db.get_global_properties().active_blockproducers == expected_active_blockproducers);

      // some days passed
      generate_blocks(tick_start_time + fc::days(60));
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      // pob activated
      bool has_hf_2262 = (HARDFORK_CORE_2262_PASSED(db.get_dynamic_global_properties().next_maintenance_time));
      expected_votes[0] = 0; // 225 days
      expected_votes[1] = 0; // 270 days
      expected_votes[2] = 0; // 315 days
      expected_votes[3] = 0; // 360 days
      int64_t base4 = 40 * 8 + (114 - 40) - 40;
      expected_votes[4] = (has_hf_2262 ? 0 : (base4 - base4 * 2 / 8)); // 405 days
      expected_votes[5] = 0;                                           // 450 days
      expected_votes[6] = 0;                                           // 495 days
      int64_t base7 = 20 * 8 * 8 + (has_hf_2262 ? 0 : ((30 - 20) * 8 + (117 - 30 - 20) - (30 - 20)));
      expected_votes[7] = base7 - base7 * 5 / 8; // 540 days
      expected_votes[8] = 0;                     // 585 days
      expected_votes[9] = 0;                     // 630 days
      expected_votes[10] = 0;                    // 675 days
      expected_votes[11] = 0;                    // 720 days
      expected_votes[12] = 0;                    // 60 days
      expected_votes[13] = 0;                    // 810 days

      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(wit_ids[i](db).total_votes, expected_votes[i]);
      }

      expected_active_blockproducers = original_wits;
      expected_active_blockproducers.erase(*expected_active_blockproducers.rbegin());
      if (!has_hf_2262)
      {
         expected_active_blockproducers.erase(*expected_active_blockproducers.rbegin());
         expected_active_blockproducers.insert(wit_ids[4]);
      }
      expected_active_blockproducers.insert(wit_ids[7]);
      BOOST_CHECK(db.get_global_properties().active_blockproducers == expected_active_blockproducers);

      // some days passed
      generate_blocks(tick_start_time + fc::days(60 + 180));
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

      has_hf_2262 = (HARDFORK_CORE_2262_PASSED(db.get_dynamic_global_properties().next_maintenance_time));
      // check votes
      base4 = 40 * 6 + (114 - 40) - 40;
      expected_votes[4] = (has_hf_2262 ? 0 : (base4 - base4 * 6 / 8)); // 585 days
      base7 = 20 * 8 * 6 + (30 - 20) * 6 + (117 - 30 - 20) - (30 - 20);
      expected_votes[7] = 0; // 720 days

      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(wit_ids[i](db).total_votes, expected_votes[i]);
      }

      expected_active_blockproducers = original_wits;
      if (!has_hf_2262)
      {
         expected_active_blockproducers.erase(*expected_active_blockproducers.rbegin());
         expected_active_blockproducers.insert(wit_ids[4]);
      }
      BOOST_CHECK(db.get_global_properties().active_blockproducers == expected_active_blockproducers);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(dxpcore_votes_calculation)
{
   try
   {
      INVOKE(put_my_dxpcore_members);

      GET_ACTOR(dxpcore0);
      GET_ACTOR(dxpcore1);
      GET_ACTOR(dxpcore2);
      GET_ACTOR(dxpcore3);
      GET_ACTOR(dxpcore4);
      GET_ACTOR(dxpcore5);
      GET_ACTOR(dxpcore6);
      GET_ACTOR(dxpcore7);
      GET_ACTOR(dxpcore8);
      GET_ACTOR(dxpcore9);
      GET_ACTOR(dxpcore10);
      GET_ACTOR(dxpcore11);
      GET_ACTOR(dxpcore12);
      GET_ACTOR(dxpcore13);

      graphene::app::database_api db_api1(db);

      vector<account_id_type> com_account_ids = {dxpcore0_id, dxpcore1_id, dxpcore2_id, dxpcore3_id,
                                                 dxpcore4_id, dxpcore5_id, dxpcore6_id, dxpcore7_id,
                                                 dxpcore8_id, dxpcore9_id, dxpcore10_id, dxpcore11_id,
                                                 dxpcore12_id, dxpcore13_id};
      vector<dxpcore_member_id_type> com_ids;
      size_t total = com_account_ids.size();

      for (size_t i = 0; i < total; ++i)
      {
         auto com_object = db_api1.get_dxpcore_member_by_account(com_account_ids[i](db).name);
         BOOST_REQUIRE(com_object.valid());
         com_ids.push_back(com_object->id);
      }

      generate_blocks(HARDFORK_CORE_2103_TIME - 750 * 86400);
      set_expiration(db, trx);

      // refresh last_vote_time
      for (size_t i = 0; i < total; ++i)
      {
         account_id_type voter = com_account_ids[total - i - 1];

         account_update_operation op;
         op.account = voter;
         op.new_options = op.account(db).options;
         op.new_options->voting_account = account_id_type();

         trx.operations.clear();
         trx.operations.push_back(op);
         PUSH_TX(db, trx, ~0);

         op.new_options->voting_account = GRAPHENE_PROXY_TO_SELF_ACCOUNT;
         trx.operations.clear();
         trx.operations.push_back(op);
         PUSH_TX(db, trx, ~0);

         trx.clear();

         generate_blocks(db.head_block_time() + 45 * 86400);
         set_expiration(db, trx);
      }

      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(com_ids[i](db).total_votes, 110u + i);
      }

      generate_blocks(HARDFORK_CORE_2103_TIME);
      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      set_expiration(db, trx);

      uint64_t expected_votes[14];

      expected_votes[0] = 110;                // 750 - 45 * 13 = 165 days
      expected_votes[1] = 111;                // 210 days
      expected_votes[2] = 112;                // 255 days
      expected_votes[3] = 113;                // 300 days
      expected_votes[4] = 114;                // 345 days
      expected_votes[5] = 115 - 115 / 8;      // 390 days
      expected_votes[6] = 116 - 116 * 2 / 8;  // 435 days
      expected_votes[7] = 117 - 117 * 3 / 8;  // 480 days
      expected_votes[8] = 118 - 118 * 4 / 8;  // 525 days
      expected_votes[9] = 119 - 119 * 5 / 8;  // 570 days
      expected_votes[10] = 120 - 120 * 6 / 8; // 615 days
      expected_votes[11] = 121 - 121 * 7 / 8; // 660 days
      expected_votes[12] = 0;                 // 705 days
      expected_votes[13] = 0;                 // 750 days

      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(com_ids[i](db).total_votes, expected_votes[i]);
      }

      vector<dxpcore_member_id_type> expected_active_dxpcore_members = {
          com_ids[0], com_ids[1], com_ids[2],
          com_ids[3], com_ids[4], com_ids[5],
          com_ids[6], com_ids[7], com_ids[8]};
      auto current_dxpcore_members = db.get_global_properties().active_dxpcore_members;
      sort(current_dxpcore_members.begin(), current_dxpcore_members.end());
      BOOST_CHECK(current_dxpcore_members == expected_active_dxpcore_members);

      // new vote
      {
         account_update_operation op;
         op.account = com_account_ids[12];
         op.new_options = op.account(db).options;
         op.new_options->votes.insert(com_ids[11](db).vote_id);
         op.new_options->votes.insert(com_ids[12](db).vote_id);

         trx.operations.clear();
         trx.operations.push_back(op);
         PUSH_TX(db, trx, ~0);
      }

      generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
      set_expiration(db, trx);

      expected_votes[11] += 122 / 2;
      expected_votes[12] = 122 / 2;
      for (size_t i = 0; i < total; ++i)
      {
         BOOST_CHECK_EQUAL(com_ids[i](db).total_votes, expected_votes[i]);
      }

      expected_active_dxpcore_members = {com_ids[0], com_ids[1], com_ids[2],
                                         com_ids[3], com_ids[4], com_ids[5],
                                         com_ids[6], com_ids[7], com_ids[11]};
      current_dxpcore_members = db.get_global_properties().active_dxpcore_members;
      sort(current_dxpcore_members.begin(), current_dxpcore_members.end());
      BOOST_CHECK(current_dxpcore_members == expected_active_dxpcore_members);
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
