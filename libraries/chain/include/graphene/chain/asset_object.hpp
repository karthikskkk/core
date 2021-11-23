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
#include <graphene/protocol/asset_ops.hpp>

#include <boost/multi_index/composite_key.hpp>

/**
 * @defgroup prediction_market Prediction Market
 *
 * A prediction market is a specialized SmartToken such that total debt and total collateral are always equal amounts
 * (although asset IDs differ). No margin calls or force settlements may be performed on a prediction market asset. A
 * prediction market is globally settled by the issuer after the event being predicted resolves, thus a prediction
 * market must always have the @c global_settle permission enabled. The maximum price for global settlement or short
 * sale of a prediction market asset is 1-to-1.
 */

namespace graphene { namespace chain {
   class asset_smarttoken_data_object;
   class database;
   using namespace graphene::db;

   /**
    *  @brief tracks the asset information that changes frequently
    *  @ingroup object
    *  @ingroup implementation
    *
    *  Because the asset_object is very large it doesn't make sense to save an undo state
    *  for all of the parameters that never change.   This object factors out the parameters
    *  of an asset that change in almost every transaction that involves the asset.
    *
    *  This object exists as an implementation detail and its ID should never be referenced by
    *  a blockchain operation.
    */
   class asset_dynamic_data_object : public abstract_object<asset_dynamic_data_object>
   {
      public:
         static constexpr uint8_t space_id = implementation_ids;
         static constexpr uint8_t type_id  = impl_asset_dynamic_data_object_type;

         /// The number of shares currently in existence
         share_type current_supply;
         share_type confidential_supply; ///< total asset held in confidential balances
         share_type accumulated_fees; ///< fees accumulate to be paid out over time
         share_type accumulated_collateral_fees; ///< accumulated collateral-denominated fees (for smarttokens)
         share_type fee_pool;         ///< in core asset
   };

   /**
    *  @brief tracks the parameters of an asset
    *  @ingroup object
    *
    *  All assets have a globally unique symbol name that controls how they are traded and an issuer who
    *  has authority over the parameters of the asset.
    */
   class asset_object : public graphene::db::abstract_object<asset_object>
   {
      public:
         static constexpr uint8_t space_id = protocol_ids;
         static constexpr uint8_t type_id  = asset_object_type;

         /// This function does not check if any registered asset has this symbol or not; it simply checks whether the
         /// symbol would be valid.
         /// @return true if symbol is a valid ticker symbol; false otherwise.
         static bool is_valid_symbol( const string& symbol );

         /// @return true if this is a market-issued asset; false otherwise.
         bool is_market_issued()const { return smarttoken_data_id.valid(); }
         /// @return true if this is a share asset of a liquidity pool; false otherwise.
         bool is_liquidity_pool_share_asset()const { return for_liquidity_pool.valid(); }
         /// @return true if users may request force-settlement of this market-issued asset; false otherwise
         bool can_force_settle()const { return !(options.flags & disable_force_settle); }
         /// @return true if the issuer of this market-issued asset may globally settle the asset; false otherwise
         bool can_global_settle()const { return options.issuer_permissions & global_settle; }
         /// @return true if this asset charges a fee for the issuer on market operations; false otherwise
         bool charges_market_fees()const { return options.flags & charge_market_fee; }
         /// @return true if this asset may only be transferred to/from the issuer or market orders
         bool is_transfer_restricted()const { return options.flags & transfer_restricted; }
         bool can_override()const { return options.flags & override_authority; }
         bool allow_confidential()const { return !(options.flags & asset_issuer_permission_flags::disable_confidential); }
         /// @return true if max supply of the asset can be updated
         bool can_update_max_supply()const { return !(options.flags & lock_max_supply); }
         /// @return true if can create new supply for the asset
         bool can_create_new_supply()const { return !(options.flags & disable_new_supply); }
         /// @return true if the asset owner can update MCR directly
         bool can_owner_update_mcr()const { return !(options.issuer_permissions & disable_mcr_update); }
         /// @return true if the asset owner can update ICR directly
         bool can_owner_update_icr()const { return !(options.issuer_permissions & disable_icr_update); }
         /// @return true if the asset owner can update MSSR directly
         bool can_owner_update_mssr()const { return !(options.issuer_permissions & disable_mssr_update); }

         /// Helper function to get an asset object with the given amount in this asset's type
         asset amount(share_type a)const { return asset(a, id); }
         /// Convert a string amount (i.e. "123.45") to an asset object with this asset's type
         /// The string may have a decimal and/or a negative sign.
         asset amount_from_string(string amount_string)const;
         /// Convert an asset to a textual representation, i.e. "123.45"
         string amount_to_string(share_type amount)const;
         /// Convert an asset to a textual representation, i.e. "123.45"
         string amount_to_string(const asset& amount)const
         { FC_ASSERT(amount.asset_id == get_id()); return amount_to_string(amount.amount); }
         /// Convert an asset to a textual representation with symbol, i.e. "123.45 USD"
         string amount_to_pretty_string(share_type amount)const
         { return amount_to_string(amount) + " " + symbol; }
         /// Convert an asset to a textual representation with symbol, i.e. "123.45 USD"
         string amount_to_pretty_string(const asset &amount)const
         { FC_ASSERT(amount.asset_id == get_id()); return amount_to_pretty_string(amount.amount); }

         /// Ticker symbol for this asset, i.e. "USD"
         string symbol;
         /// Maximum number of digits after the decimal point (must be <= 12)
         uint8_t precision = 0;
         /// ID of the account which issued this asset.
         account_id_type issuer;

         asset_options options;


         /// Current supply, fee pool, and collected fees are stored in a separate object as they change frequently.
         asset_dynamic_data_id_type  dynamic_asset_data_id;
         /// Extra data associated with SmartTokens. This field is non-null if and only if is_market_issued() returns true
         optional<asset_smarttoken_data_id_type> smarttoken_data_id;

         optional<account_id_type> buyback_account;

         /// The ID of the liquidity pool if the asset is the share asset of a liquidity pool
         optional<liquidity_pool_id_type> for_liquidity_pool;

         asset_id_type get_id()const { return id; }

         void validate()const
         {
            // UIAs may not be prediction markets, have force settlement, or global settlements
            if( !is_market_issued() )
            {
               FC_ASSERT(!(options.flags & disable_force_settle || options.flags & global_settle));
               FC_ASSERT(!(options.issuer_permissions & disable_force_settle || options.issuer_permissions & global_settle));
            }
         }

         template<class DB>
         const asset_smarttoken_data_object& smarttoken_data(const DB& db)const
         {
            FC_ASSERT( smarttoken_data_id.valid(),
                       "Asset ${a} (${id}) is not a market issued asset.",
                       ("a",this->symbol)("id",this->id) );
            return db.get( *smarttoken_data_id );
         }

         template<class DB>
         const asset_dynamic_data_object& dynamic_data(const DB& db)const
         { return db.get(dynamic_asset_data_id); }

         /**
          *  The total amount of an asset that is reserved for future issuance. 
          */
         template<class DB>
         share_type reserved( const DB& db )const
         { return options.max_supply - dynamic_data(db).current_supply; }

         /// @return true if asset can accumulate fees in the given denomination
         template<class DB>
         bool can_accumulate_fee(const DB& db, const asset& fee) const {
            return (( fee.asset_id == get_id() ) ||
                    ( is_market_issued() && fee.asset_id == smarttoken_data(db).options.short_backing_asset ));
         }

         /***
          * @brief receive a fee asset to accrue in dynamic_data object
          *
          * Asset owners define various fees (market fees, force-settle fees, etc.) to be
          * collected for the asset owners. These fees are typically denominated in the asset
          * itself, but for smarttokens some of the fees are denominated in the collateral
          * asset. This will place the fee in the right container.
          */
         template<class DB>
         void accumulate_fee(DB& db, const asset& fee) const
         {
            if (fee.amount == 0) return;
            FC_ASSERT( fee.amount >= 0, "Fee amount must be non-negative." );
            const auto& dyn_data = dynamic_asset_data_id(db);
            if (fee.asset_id == get_id()) { // fee same as asset
               db.modify( dyn_data, [&fee]( asset_dynamic_data_object& obj ){
                  obj.accumulated_fees += fee.amount;
               });
            } else { // fee different asset; perhaps collateral-denominated fee
               FC_ASSERT( is_market_issued(),
                          "Asset ${a} (${id}) cannot accept fee of asset (${fid}).",
                          ("a",this->symbol)("id",this->id)("fid",fee.asset_id) );
               const auto & bad = smarttoken_data(db);
               FC_ASSERT( fee.asset_id == bad.options.short_backing_asset,
                          "Asset ${a} (${id}) cannot accept fee of asset (${fid}).",
                          ("a",this->symbol)("id",this->id)("fid",fee.asset_id) );
               db.modify( dyn_data, [&fee]( asset_dynamic_data_object& obj ){
                  obj.accumulated_collateral_fees += fee.amount;
               });
            }
         }

   };

   /**
    *  @brief defines market parameters for margin positions, extended with an initial_collateral_ratio field
    */
   struct price_feed_with_icr : public price_feed
   {
      /// After BSIP77, when creating a new debt position or updating an existing position,
      /// the position will be checked against this parameter.
      /// Fixed point between 1.000 and 10.000, implied fixed point denominator is GRAPHENE_COLLATERAL_RATIO_DENOM
      uint16_t initial_collateral_ratio = GRAPHENE_DEFAULT_MAINTENANCE_COLLATERAL_RATIO;

      price_feed_with_icr()
      : price_feed(), initial_collateral_ratio( maintenance_collateral_ratio )
      {}

      price_feed_with_icr( const price_feed& pf, const optional<uint16_t>& icr = {} )
      : price_feed( pf ), initial_collateral_ratio( icr.valid() ? *icr : pf.maintenance_collateral_ratio )
      {}

      /// The result will be used to check new debt positions and position updates.
      /// Calculation: ~settlement_price * initial_collateral_ratio / GRAPHENE_COLLATERAL_RATIO_DENOM
      price calculate_initial_collateralization()const;
   };

   /**
    *  @brief contains properties that only apply to smarttokens (market issued assets)
    *
    *  @ingroup object
    *  @ingroup implementation
    */
   class asset_smarttoken_data_object : public abstract_object<asset_smarttoken_data_object>
   {
      public:
         static constexpr uint8_t space_id = implementation_ids;
         static constexpr uint8_t type_id  = impl_asset_smarttoken_data_object_type;

         /// The asset this object belong to
         asset_id_type asset_id;

         /// The tunable options for SmartTokens are stored in this field.
         smarttoken_options options;

         /// Feeds published for this asset. If issuer is not dxpcore, the keys in this map are the feed publishing
         /// accounts; otherwise, the feed publishers are the currently active dxpcore_members and blockproducers and this map
         /// should be treated as an implementation detail. The timestamp on each feed is the time it was published.
         flat_map<account_id_type, pair<time_point_sec,price_feed_with_icr>> feeds;
         /// This is the currently active price feed, calculated as the median of values from the currently active
         /// feeds.
         price_feed_with_icr current_feed;
         /// This is the publication time of the oldest feed which was factored into current_feed.
         time_point_sec current_feed_publication_time;

         /// Call orders with collateralization (aka collateral/debt) not greater than this value are in margin
         /// call territory.
         /// This value is derived from @ref current_feed for better performance and should be kept consistent.
         price current_maintenance_collateralization;
         /// After BSIP77, when creating a new debt position or updating an existing position, the position
         /// will be checked against the `initial_collateral_ratio` (ICR) parameter in the smarttoken options.
         /// This value is derived from @ref current_feed (which includes `ICR`) for better performance and
         /// should be kept consistent.
         price current_initial_collateralization;

         /// Derive @ref current_maintenance_collateralization and @ref current_initial_collateralization from
         /// other member variables.
         void refresh_cache();

         /// True if this asset implements a @ref prediction_market
         bool is_prediction_market = false;

         /// This is the volume of this asset which has been force-settled this maintanence interval
         share_type force_settled_volume;
         /// Calculate the maximum force settlement volume per maintenance interval, given the current share supply
         share_type max_force_settlement_volume(share_type current_supply)const;

         /** return true if there has been a black swan, false otherwise */
         bool has_settlement()const { return !settlement_price.is_null(); }

         /**
          *  In the event of a black swan, the swan price is saved in the settlement price, and all margin positions
          *  are settled at the same price with the siezed collateral being moved into the settlement fund. From this
          *  point on no further updates to the asset are permitted (no feeds, etc) and forced settlement occurs
          *  immediately when requested, using the settlement price and fund.
          */
         ///@{
         /// Price at which force settlements of a black swanned asset will occur
         price settlement_price;
         /// Amount of collateral which is available for force settlement
         share_type settlement_fund;
         ///@}

         /// Track whether core_exchange_rate in corresponding asset_object has updated
         bool asset_cer_updated = false;

         /// Track whether core exchange rate in current feed has updated
         bool feed_cer_updated = false;

         /// Whether need to update core_exchange_rate in asset_object
         bool need_to_update_cer() const
         {
            return ( ( feed_cer_updated || asset_cer_updated ) && !current_feed.core_exchange_rate.is_null() );
         }

         /// The time when @ref current_feed would expire
         time_point_sec feed_expiration_time()const
         {
            uint32_t current_feed_seconds = current_feed_publication_time.sec_since_epoch();
            if( std::numeric_limits<uint32_t>::max() - current_feed_seconds <= options.feed_lifetime_sec )
               return time_point_sec::maximum();
            else
               return current_feed_publication_time + options.feed_lifetime_sec;
         }
         bool feed_is_expired_before_hardfork_615(time_point_sec current_time)const
         { return feed_expiration_time() >= current_time; }
         bool feed_is_expired(time_point_sec current_time)const
         { return feed_expiration_time() <= current_time; }

         /******
          * @brief calculate the median feed
          *
          * This calculates the median feed from @ref feeds, feed_lifetime_sec
          * in @ref options, and the given parameters.
          * It may update the current_feed_publication_time, current_feed and
          * current_maintenance_collateralization member variables.
          *
          * @param current_time the current time to use in the calculations
          * @param next_maintenance_time the next chain maintenance time
          */
         void update_median_feeds(time_point_sec current_time, time_point_sec next_maintenance_time);
   };

   // key extractor for short backing asset
   struct smarttoken_short_backing_asset_extractor
   {
      typedef asset_id_type result_type;
      result_type operator() (const asset_smarttoken_data_object& obj) const
      {
         return obj.options.short_backing_asset;
      }
   };

   struct by_short_backing_asset;
   struct by_feed_expiration;
   struct by_cer_update;

   typedef multi_index_container<
      asset_smarttoken_data_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
         ordered_non_unique< tag<by_short_backing_asset>, smarttoken_short_backing_asset_extractor >,
         ordered_unique< tag<by_feed_expiration>,
            composite_key< asset_smarttoken_data_object,
               const_mem_fun< asset_smarttoken_data_object, time_point_sec, &asset_smarttoken_data_object::feed_expiration_time >,
               member< asset_smarttoken_data_object, asset_id_type, &asset_smarttoken_data_object::asset_id >
            >
         >,
         ordered_non_unique< tag<by_cer_update>,
                             const_mem_fun< asset_smarttoken_data_object, bool, &asset_smarttoken_data_object::need_to_update_cer >
         >
      >
   > asset_smarttoken_data_object_multi_index_type;
   typedef generic_index<asset_smarttoken_data_object, asset_smarttoken_data_object_multi_index_type> asset_smarttoken_data_index;

   struct by_symbol;
   struct by_type;
   struct by_issuer;
   typedef multi_index_container<
      asset_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
         ordered_unique< tag<by_symbol>, member<asset_object, string, &asset_object::symbol> >,
         ordered_unique< tag<by_type>,
            composite_key< asset_object,
                const_mem_fun<asset_object, bool, &asset_object::is_market_issued>,
                member< object, object_id_type, &object::id >
            >
         >,
         ordered_unique< tag<by_issuer>,
            composite_key< asset_object,
                member< asset_object, account_id_type, &asset_object::issuer >,
                member< object, object_id_type, &object::id >
            >
         >
      >
   > asset_object_multi_index_type;
   typedef generic_index<asset_object, asset_object_multi_index_type> asset_index;

} } // graphene::chain

MAP_OBJECT_ID_TO_TYPE(graphene::chain::asset_object)
MAP_OBJECT_ID_TO_TYPE(graphene::chain::asset_dynamic_data_object)
MAP_OBJECT_ID_TO_TYPE(graphene::chain::asset_smarttoken_data_object)

FC_REFLECT_DERIVED( graphene::chain::price_feed_with_icr, (graphene::protocol::price_feed),
                    (initial_collateral_ratio) )

FC_REFLECT_DERIVED( graphene::chain::asset_object, (graphene::db::object),
                    (symbol)
                    (precision)
                    (issuer)
                    (options)
                    (dynamic_asset_data_id)
                    (smarttoken_data_id)
                    (buyback_account)
                    (for_liquidity_pool)
                  )

FC_REFLECT_TYPENAME( graphene::chain::asset_smarttoken_data_object )
FC_REFLECT_TYPENAME( graphene::chain::asset_dynamic_data_object )

GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::chain::price_feed_with_icr )

GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::chain::asset_object )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::chain::asset_smarttoken_data_object )
GRAPHENE_DECLARE_EXTERNAL_SERIALIZATION( graphene::chain::asset_dynamic_data_object )