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
      void active();

      [[eosio::action]]
      void deactive();

      [[eosio::action]]
      void regtoken(const name      &token_contract,
                    const symbol    &token_symbol,
                    const asset     &max_accept,
                    const asset     &deposit_min_once,
                    const asset     &deposit_max_once,
                    const asset     &deposit_max_daily,
                    bool            active);

      [[eosio::action]]
      void activetk(const name &token_contract, const symbol &token_symbol);

      [[eosio::action]]
      void deactivetk(const name &token_contract, const symbol &token_symbol);

      [[eosio::action]]
      void setdeposittk(const name      &token_contract,
                        const symbol    &token_symbol,
                        const asset     &deposit_min_once,
                        const asset     &deposit_max_once,
                        const asset     &deposit_max_daily);

      using withdraw_action = eosio::action_wrapper<"withdraw"_n, &bridge::withdraw>;

   private:
      static constexpr eosio::name active_permission{"active"_n};
      static constexpr eosio::name eosio_token_contract{"eosio.token"_n};
      static constexpr eosio::name transfer_action{"transfer"_n};

      struct [[eosio::table]] globalstate {
         globalstate() {}

         uint64_t deposit_id = 0;
         bool active = true;

         EOSLIB_SERIALIZE( globalstate, (deposit_id)(active) )
      };

      struct [[eosio::table]] deposit {
         uint64_t id;
         name     contract;
         name     from;
         asset    quantity;
         string   memo;
         uint8_t  status;

         EOSLIB_SERIALIZE( deposit, (id)(contract)(from)(quantity)(memo)(status) )

         uint64_t primary_key() const { return id; }
      };

      struct [[eosio::table]] token_register {
         symbol      token_symbol;
         asset       accept;
         asset       max_accept;
         asset       deposit_min_once;
         asset       deposit_max_once;
         asset       deposit_max_daily;
         asset       deposit_total;
         uint64_t    deposit_total_times;
         asset       withdraw_total;
         uint64_t    withdraw_total_times;
         bool        active;

         uint64_t  primary_key()const { return token_symbol.code().raw(); }
      };

      eosio::singleton<"globalstate"_n, globalstate> _global_state;
      globalstate _gstate;

      typedef eosio::multi_index<"deposits"_n, deposit> deposits;

      typedef eosio::multi_index< "tokens"_n, token_register> tokens;
   };

}
