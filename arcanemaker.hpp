#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;

CONTRACT arcanemaker : public contract {
public:
    using contract::contract;

    ACTION worship( bool notithe);

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

    void place_spot_order( asset& wax_quantity, bool notithe);
    void send_usdc(const asset& quantity, const name& recipient, const string& memo);
    void send_ark(const asset& quantity, const name& recipient, const string& memo);
};
