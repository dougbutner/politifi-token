#include "partymaker.hpp"

// === Main Contract Actions and Methods === //

ACTION partymaker::partytime(const name& from, const asset& wax_quantity) {
    require_auth(get_self()); // -- Only contract account can call this action

    // --- Check WAX and XPX balances before placing orders --- //
    asset wax_balance = get_balance("eosio.token"_n, get_self(), symbol_code("WAX"));
    asset xpx_balance = get_balance("xparty"_n, get_self(), symbol_code("XPX"));

    // -- Send if there's over 20 WAX or over 1 XPX held by the contract -- //
    if (wax_balance.amount <= 2000000000 ) {
        // --- Place spot orders --- //
        place_spot_order(wax_quantity);
    }

    if (xpx_balance.amount <= 100000000) {
        string realmemo = to_string(xpx_amount.amount / 100000000.0) + " WAX@eosio.token";
        // --- Place spot orders --- //
        send_xpx(xpx_balance, "swap.alcor"_n, realmemo);
    }



}

// --- Place spot orders on Alcor DEX --- //
void partymaker::place_spot_order(const asset& wax_quantity) {
    // Split the WAX into different parts as per the strategy and place orders on Alcor DEX
    asset order1 = wax_quantity * 68 / 100;
    asset order2 = wax_quantity * 10 / 100;
    asset order3 = wax_quantity * 5 / 100;
    asset order4 = wax_quantity * 5 / 100;
    asset order5 = wax_quantity * 12 / 100;

   // Calculate XPX amounts for each WAX order
    asset xpx_amount1 = asset(order1 / 98, symbol("XPX", 8));
    asset xpx_amount2 = asset(order2 / 96, symbol("XPX", 8));
    asset xpx_amount3 = asset(order3 / 92, symbol("XPX", 8));
    asset xpx_amount4 = asset(order4 / 88, symbol("XPX", 8));
    asset xpx_amount5 = asset(order5 / 68, symbol("XPX", 8));

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
    asset fee1 = wax_quantity * 22 / 1000;
    send_wax(fee1, "klmay.wam"_n, "Fee transfer");

    // Send 4.4% to tetra
    asset fee2 = wax_quantity * 44 / 1000;
    send_wax(fee2, "tetra"_n, "Fee transfer");
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
