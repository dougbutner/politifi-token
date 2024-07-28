#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <string>

// === Politifi Token by X Party xparty.win === //
using namespace std;
using namespace eosio;

class [[eosio::contract]] xparty : public contract {
   public:
      using contract::contract;

      // --- Action to create a new token --- //
      ACTION create( const name&   issuer,
                     const asset&  maximum_supply);

      // --- Mint tokens to an account --- //
      ACTION mint( const name& to, const asset& quantity, const string& memo );

      // --- Burnvote tokens from an account --- //
      ACTION vote( const name& voter, const asset& quantity, const string& memo, const name& state_postal, uint64_t zip_code );

      // --- Transfer tokens between accounts --- //
      ACTION transfer( const name&    from,
                       const name&    to,
                       const asset&   quantity,
                       const string&  memo );

      // --- Open a token account --- //
      ACTION open( const name& owner, const symbol& symbol, const name& ram_payer );

      // --- Close a token account --- //
      ACTION close( const name& owner, const symbol& symbol );

      // --- Vest tokens --- //
      ACTION vest( const name& to,
                   const asset& quantity,
                   uint64_t vest_seconds,
                   const string& memo );

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


            // --- Store state vote statistics --- //
      TABLE state_stat {
         name           state_postal;
         uint64_t       total_votes;
         uint64_t       total_voters;

         uint64_t primary_key()const { return state_postal.value; }
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
         string         last_memo;
         name           state_postal;
         uint64_t       zip_code;

         uint64_t primary_key()const { return voter.value; }
         uint64_t by_state() const { return state_postal.value; }
         uint64_t by_zip() const { return zip_code; }

      };


      using accounts = multi_index<"accounts"_n, account>;
      using stats = multi_index<"stat"_n, currency_stats>;
      using vests = multi_index<"vests"_n, vest_record>;
      //using voters = multi_index<"voters"_n, voter_record>;
      using statestats = multi_index<"statestats"_n, state_stat>;


      using voters = multi_index<name("voters"), voter_record,
         indexed_by<"bystate"_n, const_mem_fun<voter_record, uint64_t, &voter_record::by_state>>,
         indexed_by<"byzip"_n, const_mem_fun<voter_record, uint64_t, &voter_record::by_zip>>
      >;



      // --- Subtract balance from an account --- //
      void sub_balance( const name& owner, const asset& value );
      // --- Add balance to an account --- //
      void add_balance( const name& owner, const asset& value, const name& ram_payer );
      // --- Add vested balance to an account --- //
      void add_vested_balance( const name& owner, const asset& value, uint64_t vest_seconds, const name& ram_payer );
}; //END xparty

