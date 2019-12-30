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
      static constexpr eosio::name eosio_token_account{"eosio.token"_n};
      static constexpr eosio::name transfer_action{"transfer"_n};

      struct [[eosio::table]] global_state {
         global_state() {}

         uint64_t deposit_id = 1;
         bool active = true;
      };

      struct [[eosio::table]] deposit {
         uint64_t id;
         name contract;
         name from;
         asset quantity;
         string memo;
         uint8_t status;

         uint64_t primary_key() const { return id; }
      };

      eosio::singleton<"globals"_n, global_state> _global_state;
      global_state _gstate;

      typedef eosio::multi_index<"deposits"_n, deposit> deposits;
   };

}
