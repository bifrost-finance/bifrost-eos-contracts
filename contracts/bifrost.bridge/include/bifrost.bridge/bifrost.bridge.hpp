#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

#include <string>

namespace bifrost {

   using std::string;
   using namespace eosio;

   class [[eosio::contract("bifrost.bridge")]] bridge : public contract {
   public:
      using contract::contract;

      [[eosio::on_notify("eosio.token::transfer")]]
      void deposit_notify(const name &from,
                          const name &to,
                          const asset &quantity,
                          const string &memo);

      [[eosio::action]]
      void withdraw(const name &to,
                    const asset &quantity,
                    const string &memo);

      using withdraw_action = eosio::action_wrapper<"withdraw"_n, &bridge::withdraw>;

   private:
      static constexpr eosio::name active_permission{"active"_n};
      static constexpr eosio::name eosio_token_account{"eosio.token"_n};
      static constexpr eosio::name eosio_token_transfer_action{"transfer"_n};
   };

}
