#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

#include <string>

namespace bifrost {

   using std::string;
   using namespace eosio;

   class [[eosio::contract("bifrost.bridge")]] bridge : public contract {
   public:
      using contract::contract;

      bridge(name s, name code, datastream<const char *> ds);

      ~bridge();

      [[eosio::on_notify("eosio.token::transfer")]]
      void deposit_notify(const name &from,
                          const name &to,
                          const asset &quantity,
                          const string &memo);

      [[eosio::action]]
      void depositcnfm(uint64_t deposit_id);

      [[eosio::action]]
      void depositrlbk(uint64_t deposit_id);

      [[eosio::action]]
      void withdraw(const name &contract,
                    const name &to,
                    const asset &quantity,
                    const string &memo);

      [[eosio::action]]
      void activate();

      [[eosio::action]]
      void deactivate();

      using withdraw_action = eosio::action_wrapper<"withdraw"_n, &bridge::withdraw>;

   private:
      static constexpr eosio::name active_permission{"active"_n};
      static constexpr eosio::name eosio_token_contract{"eosio.token"_n};
      static constexpr eosio::name transfer_action{"transfer"_n};

      struct [[eosio::table]] globalstate {
         globalstate() {}

         uint64_t deposit_id = 1;
         bool active = true;

         EOSLIB_SERIALIZE( globalstate, (deposit_id)(active) )
      };

      struct [[eosio::table]] deposit {
         uint64_t id;
         name contract;
         name from;
         asset quantity;
         string memo;
         uint8_t status;

         EOSLIB_SERIALIZE( deposit, (id)(contract)(from)(quantity)(memo)(status) )

         uint64_t primary_key() const { return id; }
      };

      struct [[eosio::table]] token_register {
         name        token_contract;
         symbol      token_symbol;
         asset       accept;
         asset       max_accept;
         asset       min_once_transfer;
         asset       max_once_transfer;
         asset       max_daily_transfer;
         asset       total_transfer;
         uint64_t    total_transfer_times;
         bool        active;

         uint64_t  primary_key()const { return token_contract.value; }
         uint64_t  by_token_symbol()const { return token_symbol.code().raw(); }
      };

      eosio::singleton<"globalstate"_n, globalstate> _global_state;
      globalstate _gstate;

      typedef eosio::multi_index<"deposits"_n, deposit> deposits;

      typedef eosio::multi_index< "tokens"_n, token_register,
              indexed_by<
                 "tokensym"_n,
                 const_mem_fun<token_register, uint64_t, &token_register::by_token_symbol>
              >
      > tokens;
   };

}
