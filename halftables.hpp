#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

namespace alcor_contract {

  struct[[eosio::table]] incentives {
    uint64_t                  id;
    eosio::name               creator;
    uint64_t                  poolId;
    eosio::extended_asset     reward;
    uint32_t                  periodFinish;
    uint32_t                  rewardsDuration;
    uint128_t                 rewardRateE18;
    uint128_t                 rewardPerTokenStored;
    uint64_t                  totalStakingWeight;
    uint32_t                  lastUpdateTime;
    uint32_t                  numberOfStakes;

    uint64_t  primary_key()const { return id; }
  };

  typedef eosio::multi_index< "incentives"_n, incentives> incentives_table;
  
}

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] incentive_ids {
  uint64_t  pool_id;
  uint64_t  incentive_id;                

  uint64_t primary_key() const { return incentive_id; }
};
using incentive_ids_table = eosio::multi_index<"incentiveids"_n, incentive_ids>;