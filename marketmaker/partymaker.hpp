#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;

CONTRACT partymaker : public contract {
public:
    using contract::contract;

    ACTION partytime(const name& from, const asset& wax_quantity);

private:
    struct [[eosio::table]] account {
        asset    balance;
        uint64_t primary_key()const { return balance.symbol.code().raw(); }
    };

    using accounts = eosio::multi_index<"accounts"_n, account>;

    static asset get_balance(name token_contract_account, name owner, symbol_code sym_code) {
        accounts accountstable(token_contract_account, owner.value);
        const auto& ac = accountstable.get(sym_code.raw());
        return ac.balance;
    }

    void place_spot_order(const asset& wax_quantity);
    void send_wax(const asset& quantity, const name& recipient, const string& memo);
    void send_xpx(const asset& quantity, const name& recipient, const string& memo);
};
