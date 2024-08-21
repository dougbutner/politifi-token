#include "partymaker.hpp"

// === Main Contract Actions and Methods === //


ACTION partymaker::partytime(bool freecover = 0) {
    if(freecover){
        require_auth(get_self()); // -- Only contract account can call this action with no fee
    }

    // --- Check WAX and XPX balances before placing orders --- //
    asset wax_balance = get_balance("eosio.token"_n, get_self(), symbol_code("WAX"));
    asset xpx_balance = get_balance("xparty"_n, get_self(), symbol_code("XPX"));

    // -- Send if there's over 20 WAX or over 1 XPX held by the contract -- //
    if (wax_balance.amount >= 2000000000 ) {
        // --- Place spot orders --- //
        place_spot_order(wax_balance, freecover);
    }

    if (xpx_balance.amount >= 100000000) {
        // Convert XPX balance to string and chop off the last 3 characters
        string xpx_str = xpx_balance.to_string();
        string xpx_str_chopped = xpx_str.substr(0, xpx_str.size() - 3);

        // Create memo with the modified XPX string
        string realmemo = xpx_str_chopped + " WAX@eosio.token";
        // --- Place spot orders --- //
        send_xpx(xpx_balance, "alcordexmain"_n, realmemo);
    }

    if (wax_balance.amount <= 2000000000 && xpx_balance.amount <= 100000000) {
        check(false, "There's no party supplies.");
    }
}

// --- Place spot orders on Alcor DEX --- //
void partymaker::place_spot_order( asset& wax_quantity, bool freecover = 0 ) {

    // Adjust wax_quantity
    wax_quantity = wax_quantity - (wax_quantity * 66 / 1000);

    // Split the WAX into different parts as per the strategy and place orders on Alcor DEX
    asset order1 = asset(wax_quantity.amount * 68 / 100, wax_quantity.symbol);
    asset order2 = asset(wax_quantity.amount * 10 / 100, wax_quantity.symbol);
    asset order3 = asset(wax_quantity.amount * 5 / 100, wax_quantity.symbol);
    asset order4 = asset(wax_quantity.amount * 5 / 100, wax_quantity.symbol);
    asset order5 = asset(wax_quantity.amount * 12 / 100, wax_quantity.symbol);

    // Calculate XPX amounts for each WAX order
    asset xpx_amount1 = asset(order1.amount * 100 / 98, symbol("XPX", 8));
    asset xpx_amount2 = asset(order2.amount * 100 / 96, symbol("XPX", 8));
    asset xpx_amount3 = asset(order3.amount * 100 / 92, symbol("XPX", 8));
    asset xpx_amount4 = asset(order4.amount * 100 / 88, symbol("XPX", 8));
    asset xpx_amount5 = asset(order5.amount * 100 / 68, symbol("XPX", 8));

    // Memo for each order
    string memo1 = xpx_amount1.to_string() + "@xparty";
    string memo2 = xpx_amount2.to_string() + "@xparty";
    string memo3 = xpx_amount3.to_string() + "@xparty";
    string memo4 = xpx_amount4.to_string() + "@xparty";
    string memo5 = xpx_amount5.to_string() + "@xparty";


    // Placing the orders
    send_wax(order1, "alcordexmain"_n, memo1);
    send_wax(order2, "alcordexmain"_n, memo2);
    send_wax(order3, "alcordexmain"_n, memo3);
    send_wax(order4, "alcordexmain"_n, memo4);
    send_wax(order5, "alcordexmain"_n, memo5);

    // Send 2.2% to klmay.wam
    if(!freecover){
        asset fee1 = wax_quantity * 22 / 1000;
        send_wax(fee1, "klmay.wam"_n, "💸 Fee transfer");
    }

    // Send 4.4% to tetra
    if(!freecover){
        asset fee2 = wax_quantity * 44 / 1000;
        send_wax(fee2, "tetra"_n, "💸 Fee transfer");
    }

}

// --- Inline action to send WAX tokens --- //
void partymaker::send_wax(const asset& quantity, const name& recipient, const string& memo) {
    action(
        permission_level{get_self(), "active"_n},
        "eosio.token"_n,
        "transfer"_n,
        std::make_tuple(get_self(), recipient, quantity, memo)
    ).send();
}

// --- Inline action to send XPX tokens --- //
void partymaker::send_xpx(const asset& quantity, const name& recipient, const string& memo) {
    action(
        permission_level{get_self(), "active"_n},
        "xparty"_n,
        "transfer"_n,
        std::make_tuple(get_self(), recipient, quantity, memo)
    ).send();
}

//END of contract actions and methods
