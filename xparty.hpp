#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <string>

// === Politifi Token by X Party xparty.win === //

using namespace eosio;

class [[eosio::contract]] xparty : public contract {
   public:
      using contract::contract;

      // --- Action to create a new token --- //
      ACTION create( const name&   issuer,
                     const asset&  maximum_supply);

      // --- Mint tokens to an account --- //
      ACTION mint( const name& to, const asset& quantity, const std::string& memo );

      // --- Burnvote tokens from an account --- //
      ACTION vote( const name& voter, const asset& quantity, const std::string& memo );

      // --- Transfer tokens between accounts --- //
      ACTION transfer( const name&    from,
                       const name&    to,
                       const asset&   quantity,
                       const std::string&  memo );

      // --- Open a token account --- //
      ACTION open( const name& owner, const symbol& symbol, const name& ram_payer );

      // --- Close a token account --- //
      ACTION close( const name& owner, const symbol& symbol );

      // --- Vest tokens --- //
      ACTION vest( const name& to,
                   const asset& quantity,
                   uint64_t vest_seconds,
                   const std::string& memo );

      // --- Claim vested tokens --- //
      ACTION claimvest( uint64_t id,
                        const asset& quantity );

      // --- Get the supply of a token --- //
      static asset get_supply( const name& token_contract_account, const symbol_code& sym_code ) {
         stats statstable( token_contract_account, sym_code.raw() );
         const auto& st = statstable.get( sym_code.raw(), "🜛 Invalid supply symbol code" );
         return st.supply;
      }

      // --- Get the balance of an account --- //
      static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code ) {
         accounts accountstable( token_contract_account, owner.value );
         const auto& ac = accountstable.get( sym_code.raw(), "🜛 No balance with specified symbol" );
         return ac.balance;
      }

   private:
      // --- Store account balances --- //
      TABLE account {
         asset    balance;

         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };

      // --- Store token statistics --- //
      TABLE currency_stats {
         asset    supply;
         asset    max_supply;
         name     issuer;

         uint64_t primary_key()const { return supply.symbol.code().raw(); }
      };



      // --- Store vesting records --- //
      TABLE vest_record {
         uint64_t       id;
         asset          vested_balance;
         name           receiver;
         uint64_t       vested_until;

         uint64_t primary_key()const { return id; }
      };

      // --- Store vote records --- //
      TABLE voter_record {
         name           voter;
         asset          total_voted;
         std::string    last_memo;
         std::string    state_postal;
         uint64_t       zip_code;

         uint64_t primary_key()const { return voter.value; }
         uint64_t by_state() const { return eosio::name(state).value; }
         uint64_t by_zip() const { return eosio::name(zip).value; }

      };

      typedef multi_index< "accounts"_n, account > accounts;
      typedef multi_index< "stat"_n, currency_stats > stats;
      typedef multi_index< "vests"_n, vest_record > vests;
      typedef multi_index< "voters"_n, voter_record > voters;

      // --- Subtract balance from an account --- //
      void sub_balance( const name& owner, const asset& value );
      // --- Add balance to an account --- //
      void add_balance( const name& owner, const asset& value, const name& ram_payer );
      // --- Add vested balance to an account --- //
      void add_vested_balance( const name& owner, const asset& value, uint64_t vest_seconds, const name& ram_payer );
}; //END xparty

} // namespace eosio
