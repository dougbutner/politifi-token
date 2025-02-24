#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

using namespace eosio;
using namespace std;

CONTRACT deer : public contract {
public:
    using contract::contract;

    [[eosio::on_notify("*::transfer")]]
    void handle_transfer(name from, name to, asset quantity, string memo) {
        // Only process incoming transfers to this contract
        if (to != get_self()) return;
        
        // Check memo starts with "Col" (case sensitive)
        if (memo.length() < 3 || memo.substr(0, 3) != "Col") return;
        
        // Check token is not XPR (assuming XPR uses symbol code XPR with precision 4)
        if (quantity.symbol == symbol("XPR", 4)) return;

        // Calculate 50% of received amount (rounding down)
        asset half_quantity = quantity;
        half_quantity.amount /= 2;

        // Send 50% to fees account
        action(
            permission_level{get_self(), "active"_n},
            get_first_receiver(), // Use original token contract
            "transfer"_n,
            std::make_tuple(get_self(), "nyra"_n, half_quantity, 
                           std::string("Fee gift: "))
        ).send();
    }
}; 