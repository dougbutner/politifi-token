#include "xparty.hpp"
#include <eosio/system.hpp> 

using namespace eosio;
using namespace std;

// === Politifi Token by XParty xparty.win === //

// --- Create a new token --- //
void xparty::create( const name& issuer, const asset& maximum_supply ) {
    require_auth( get_self() );

    auto sym = maximum_supply.symbol;
    check( maximum_supply.is_valid(), "🗳 Supply is not valid." );
    check( maximum_supply.amount > 0, "🗳 Max-supply must be positive." );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "🗳 A token with symbol already exists." );

    statstable.emplace( get_self(), [&]( auto& s ) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply = maximum_supply;
        s.issuer = issuer;
    });
}

// --- Mint tokens to an account --- //
void xparty::mint( const name& to, const asset& quantity, const string& memo ) {
    auto sym = quantity.symbol;
    check( sym.is_valid(), "🗳 Invalid symbol name" );
    check( memo.size() <= 256, "🗳 Memo is too long" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "🗳 A token with symbol does not exist, create token before issue" );
    const auto& st = *existing;
    check( to == st.issuer, "🗳 Tokens can only be issued to issuer account" );

    require_auth( st.issuer );
    check( quantity.is_valid(), "🗳  Invalid quantity" );
    check( quantity.amount > 0, "🗳 You must issue positive quantity" );

    check( quantity.symbol == st.supply.symbol, "🗳 Symbol precision mismatch" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "🗳 Quantity exceeds available supply" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
        s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );
}

// --- vote tokens from an account --- //
void xparty::vote( const name& voter, const asset& quantity, const string& memo, const name& state_postal, uint64_t zip_code ) {
    require_auth( voter );
    require_recipient("xparty"_n);

    auto sym = quantity.symbol;
    check( sym.is_valid(), "🗳 Invalid token." );
    check( memo.size() <= 500, "🗳  Say less. Memo is too long." );

    // Check state_postal length and validity
    const set<string> valid_states = {"al", "ak", "az", "ar", "ca", "co", "ct", "de", "fl", "ga", "hi", "id", "il", "in", "ia", "ks", "ky", "la", "me", "md", "ma", "mi", "mn", "ms", "mo", "mt", "ne", "nv", "nh", "nj", "nm", "ny", "nc", "nd", "oh", "ok", "or", "pa", "ri", "sc", "sd", "tn", "tx", "ut", "vt", "va", "wa", "wv", "wi", "wy"};
    string state_postal_str = state_postal.to_string();
    check(state_postal_str.size() == 2, "🗳 State postal must be lower-case and exactly 2 characters");
    check(valid_states.find(state_postal_str) != valid_states.end(), "🗳 Invalid US state postal abbreviation. Should be 2-letters, lowercase.");

    // Check zip_code validity
    check(zip_code < 100000, "🗳 Zip code is outside of valid range. Should be 5 numbers, like 21520.");


    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "🗳 Token with symbol does not exist" );
    const auto& st = *existing;

    check( quantity.is_valid(), "🗳 Invalid quantity. Should be like 100 X." );
    check( quantity.amount > 0, "🗳 Must vote positive quantity" );
    check( quantity.symbol == st.supply.symbol, "🗳 Symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
        s.supply -= quantity;
    });

    sub_balance( voter, quantity );


    voters votetable( get_self(), get_self().value );
    bool new_voter = false;
    auto it = votetable.find( voter.value );
    if (it == votetable.end()) {
        votetable.emplace( voter, [&]( auto& b ) {
            b.voter = voter;
            b.total_voted = quantity;
            b.last_memo = memo;
            b.state_postal = state_postal;
            b.zip_code = zip_code;
        });
        new_voter = true;
    } else {
        votetable.modify( it, same_payer, [&]( auto& b ) {
            b.total_voted += quantity;
            b.last_memo = memo;
        });
    }

    // Update state statistics
    statestats statstable_state( get_self(), get_self().value );
    auto state_it = statstable_state.find( state_postal.value );
    if (state_it == statstable_state.end()) {
        statstable_state.emplace( get_self(), [&]( auto& s ) {
            s.state_postal = state_postal;
            s.total_votes = quantity.amount;
            s.total_voters = 1;
        });
    } else {
        statstable_state.modify( state_it, same_payer, [&]( auto& s ) {
            s.total_votes += quantity.amount;
            if (new_voter) {
                s.total_voters += 1;
            }
        });
    }
}

// --- Transfer tokens between accounts --- //
void xparty::transfer( const name& from, const name& to, const asset& quantity, const string& memo ) {
    check( from != to, "🗳 Cannot transfer to self" );
    require_auth( from );
    check( is_account( to ), "🗳 To account does not exist" );
    auto sym = quantity.symbol.code();
    stats statstable( get_self(), sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "🗳 Invalid quantity" );
    check( quantity.amount > 0, "🗳 Must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "🗳 Symbol precision mismatch" );
    check( memo.size() <= 256, "🗳 Memo is too long" );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

// --- Vest tokens --- //
void xparty::vest( const name& to, const asset& quantity, uint64_t vest_seconds, const string& memo ) {
    require_auth( get_self() );
    check( is_account( to ), "🗳 To account does not exist" );
    check( vest_seconds > 0, "🗳 Must vest for positive time" );
    check( vest_seconds <= 10*365*24*60*60, "🗳 Must vest for no longer than 10 years" );

    auto sym = quantity.symbol.code();
    stats statstable( get_self(), sym.raw() );
    const auto& st = statstable.get( sym.raw(), "🗳 Token with symbol does not exist" );

    check( quantity.is_valid(), "🗳 Invalid quantity" );
    check( quantity.amount > 0, "🗳 Must vest positive quantity" );
    check( quantity.symbol == st.supply.symbol, "🗳 Symbol precision mismatch" );
    check( memo.size() <= 256, "🗳 Memo is too long" );

    sub_balance( get_self(), quantity );
    add_vested_balance( to, quantity, vest_seconds, get_self() );
}

// --- Claim vested tokens --- //
void xparty::claimvest( uint64_t id, const asset& quantity ) {
    check( quantity.is_valid(), "🗳 Invalid quantity" );
    check( quantity.amount > 0, "🗳 Must claim positive quantity" );

    vests vestings( get_self(), get_self().value );
    auto vest = vestings.get( id, "🗳 Vest not found" );
    auto receiver = vest.receiver;

    require_auth( receiver );

    check( quantity.symbol == vest.vested_balance.symbol, "🗳 Symbol precision mismatch" );
    check( vest.vested_balance.amount >= quantity.amount, "🗳 Overdrawn balance" );
    check( current_time_point().sec_since_epoch() >= vest.vested_until, "🗳 Vesting period not over" );

    if( vest.vested_balance.amount == quantity.amount ) {
        vestings.erase( vest );
    } else {
        vestings.modify( vest, receiver, [&]( auto& v ) {
            v.vested_balance -= quantity;
        });
    }

    add_balance( receiver, quantity, get_self() );
}

// --- Subtract balance from an account --- //
void xparty::sub_balance( const name& owner, const asset& value ) {
    accounts from_acnts( get_self(), owner.value );

    const auto& from = from_acnts.get( value.symbol.code().raw(), "🗳 You don't have the token. No balance object found." );
    check( from.balance.amount >= value.amount, "🗳 Overdrawn balance." );

    from_acnts.modify( from, owner, [&]( auto& a ) {
        a.balance -= value;
    });
}

// --- Add balance to an account --- //
void xparty::add_balance( const name& owner, const asset& value, const name& ram_payer ) {
    accounts to_acnts( get_self(), owner.value );
    auto to = to_acnts.find( value.symbol.code().raw() );
    if( to == to_acnts.end() ) {
        to_acnts.emplace( ram_payer, [&]( auto& a ){
            a.balance = value;
        });
    } else {
        to_acnts.modify( to, same_payer, [&]( auto& a ) {
            a.balance += value;
        });
    }
}

// --- Add vested balance to an account --- //
void xparty::add_vested_balance( const name& owner, const asset& value, uint64_t vest_seconds, const name& ram_payer ) {
    vests vestings( get_self(), get_self().value );
    vestings.emplace( ram_payer, [&]( auto& v ) {
        v.id = vestings.available_primary_key();
        v.vested_balance = value;
        v.receiver = owner;
        v.vested_until = current_time_point().sec_since_epoch() + vest_seconds;
    });
}

// --- Open a token account --- //
void xparty::open( const name& owner, const symbol& symbol, const name& ram_payer ) {
    require_auth( ram_payer );

    check( is_account( owner ), "🗳 Owner account does not exist" );

    auto sym_code_raw = symbol.code().raw();
    stats statstable( get_self(), sym_code_raw );
    const auto& st = statstable.get( sym_code_raw, "🗳 Symbol does not exist" );
    check( st.supply.symbol == symbol, "🗳 Symbol precision mismatch" );

    accounts acnts( get_self(), owner.value );
    auto it = acnts.find( sym_code_raw );
    if( it == acnts.end() ) {
        acnts.emplace( ram_payer, [&]( auto& a ){
            a.balance = asset{0, symbol};
        });
    }
}

// --- Close a token account --- //
void xparty::close( const name& owner, const symbol& symbol ) {
    require_auth( owner );

    accounts acnts( get_self(), owner.value );
    auto it = acnts.find( symbol.code().raw() );
    check( it != acnts.end(), "🗳 Balance row already deleted or never existed. Action won't have any effect." );
    check( it->balance.amount == 0, "🗳 Cannot close because the balance is above zero." );
    acnts.erase( it );
}
