#include "arcanemaker.hpp"

// === Main Contract Actions and Methods === //
ACTION arcanemaker::worship(bool notithe = 0) {
    if(notithe){
        require_auth(get_self()); // -- Only contract account can call this action with no fee
    }

    // --- Check WAX and ARK balances before placing orders --- //
    asset usdc_balance = get_balance("xtokens"_n, get_self(), symbol_code("XUSDC"));
    asset ark_balance = get_balance("arcana"_n, get_self(), symbol_code("ARK"));

    // -- Send if there's over 1 USDC or over 1 ARK held by the contract -- //
    if (usdc_balance.amount >= 1000000 ) {
        // --- Place spot orders --- //
        place_spot_order(usdc_balance, notithe);
    }

    if (ark_balance.amount >= 100000000 && !notithe) {
        // Convert ARK balance to string and chop off the last 3 characters
        string ark_str = ark_balance.to_string();
        string ark_str_chopped = ark_str.substr(0, ark_str.size() - 6);

        // Create memo with the modified ARK string
        string realmemo = ark_str_chopped + " XUSDC@xtokens";

                //check(false, ark_str_chopped + " NEXT " + realmemo);
        // --- Place spot orders --- //
        send_ark(ark_balance, "alcor"_n, realmemo);
    }

    if (usdc_balance.amount <= 100000 && ark_balance.amount <= 100000000) {
        check(false, "There's no offerings.");
    }
}

// --- Place spot orders on Alcor DEX --- //
void arcanemaker::place_spot_order( asset& usdc_quantity, bool notithe = 0 ) {


    // --- Adjust usdc_quantity --- //
    if (!notithe){ 
        usdc_quantity = usdc_quantity - (usdc_quantity * (618 / 1000));  // Adjusting for the 61.8% that will not be placed into these orders
    }

    // Split the USDC into different parts as per the strategy and place orders on Alcor DEX
    asset order1 = asset(usdc_quantity.amount * 11 / 100, usdc_quantity.symbol);  // 6% for 0.9764 USD
    asset order2 = asset(usdc_quantity.amount * 7 / 100, usdc_quantity.symbol);  // 7% for 0.9618 USD
    asset order3 = asset(usdc_quantity.amount * 10 / 100, usdc_quantity.symbol);  // 10% for 0.95 USD
    asset order4 = asset(usdc_quantity.amount * 9 / 100, usdc_quantity.symbol);  // 9% for 0.9382 USD
    asset order5 = asset(usdc_quantity.amount / 100, usdc_quantity.symbol);  // 6% for 0.9214 USD

    // Calculate ARK amounts for each USDC order
    // Assuming that the ARK amounts should be calculated based on price levels corresponding to each order
    asset ark_amount1 = asset(order1.amount * 10000 / 98, symbol("ARK", 8)); // Corresponding to 0.9764 USD
    asset ark_amount2 = asset(order2.amount * 10000 / 96, symbol("ARK", 8)); // Corresponding to 0.9618 USD
    asset ark_amount3 = asset(order3.amount * 10000 / 95, symbol("ARK", 8));    // Corresponding to 0.95 USD
    asset ark_amount4 = asset(order4.amount * 10000 / 94, symbol("ARK", 8)); // Corresponding to 0.9382 USD
    asset ark_amount5 = asset(order5.amount * 10000 / 66, symbol("ARK", 8)); // Corresponding to 0.9214 USD


    // Memo for each order
    string memo1 = ark_amount1.to_string() + "@arcana";
    string memo2 = ark_amount2.to_string() + "@arcana";
    string memo3 = ark_amount3.to_string() + "@arcana";
    string memo4 = ark_amount4.to_string() + "@arcana";
    string memo5 = ark_amount5.to_string() + "@arcana";


    // Placing the orders
    send_usdc(order1, "alcor"_n, memo1);
    send_usdc(order2, "alcor"_n, memo2);
    send_usdc(order3, "alcor"_n, memo3);
    send_usdc(order4, "alcor"_n, memo4);
    send_usdc(order5, "alcor"_n, memo5);

    // Send Profits back to 
    if(!notithe){
        asset donation = usdc_quantity * 618 / 1000;
        send_usdc(donation, "lyran"_n, "⛵️ Donation in honor of the Sea ⛵️");
    }
}

// --- Inline action to send USDC tokens --- //
void arcanemaker::send_usdc(const asset& quantity, const name& recipient, const string& memo) {
    action(
        permission_level{get_self(), "active"_n},
        "xtokens"_n,
        "transfer"_n,
        std::make_tuple(get_self(), recipient, quantity, memo)
    ).send();
}

// --- Inline action to send ARK tokens --- //
void arcanemaker::send_ark(const asset& quantity, const name& recipient, const string& memo) {
    action(
        permission_level{get_self(), "active"_n},
        "arcana"_n,
        "transfer"_n,
        std::make_tuple(get_self(), recipient, quantity, memo)
    ).send();
}

//END of contract actions and methods
