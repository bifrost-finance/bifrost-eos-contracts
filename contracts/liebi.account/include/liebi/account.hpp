#pragma

#include <string>
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>

#include "liebi/config.hpp"

namespace eosiosystem {
   class system_contract;
}

using namespace std;
using namespace eosio;

namespace liebi {
    class [[eosio::contract("liebi.account")]] account : contract {
    public:
        using contract::contract;

        [[eosio::on_notify("eosio.token::transfer")]]
        void transfer_receipt(const name from, const name to, const asset quantity, const string memo);

        [[eosio::action]]
        void init();

        [[eosio::action]]
        void active();

        [[eosio::action]]
        void deactive();

        [[eosio::action]]
        void setdict(const uint64_t id, const uint64_t value);

        [[eosio::action]]
        void ramprice() {
            require_auth(_self);

            symbol ramcore_symbol = symbol(symbol_code("RAMCORE"), 4);
            rammarket_index rammarket(EOS_SYSTEM_CONTRACT, EOS_SYSTEM_CONTRACT.value);
            const auto &st = rammarket.get(ramcore_symbol.raw(), "ram market does not exist");

            print(st.supply);
            print(" ");

            print(st.quote.balance);
            print(" ");

            print(st.base.balance);
            print(" ");

            double quote_balance = (double) st.quote.balance.amount / 10000;
            double base_balance = (double) st.base.balance.amount;
            print(quote_balance);
            print(" ");
            print(base_balance);
            print(" ");
            print(((quote_balance / base_balance) * 1024));
            print(" ");

            return;
        }

    private:

        asset create_account(const name account, const asset quantity,
                            const string owner_key_str, const string active_key_str);

        void init_dict(uint64_t id, uint64_t val);

        void upsert_dict_val(uint64_t id, uint64_t val);

        uint64_t get_dict_val(uint64_t id);

        void assert_active();

        void split(const std::string &s, const std::string &seperator, std::vector <std::string> &v);

        struct [[eosio::table]] dict {
            uint64_t id;
            uint64_t val;

            uint64_t primary_key() const { return id; };
            EOSLIB_SERIALIZE(dict, (id)(val)
            );
        };

        struct exchange_state {
            asset supply;

            struct connector {
                asset balance;
                double weight;

                EOSLIB_SERIALIZE( connector, (balance)(weight))
            };

            connector base;
            connector quote;

            uint64_t primary_key() const { return supply.symbol.raw(); }

            EOSLIB_SERIALIZE( exchange_state, (supply)(base)(quote)
            )
        };

        typedef eosio::multi_index<"dicts"_n, dict> dict_index;

        typedef eosio::multi_index<"rammarket"_n, exchange_state> rammarket_index;
    };
} /// namespace liebi
