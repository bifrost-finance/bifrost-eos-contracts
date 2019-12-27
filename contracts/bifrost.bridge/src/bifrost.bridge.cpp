#include <bifrost.bridge/bifrost.bridge.hpp>

namespace bifrost {

   void bridge::deposit_notify(const name &from,
                               const name &to,
                               const asset &quantity,
                               const string &memo) {
      if (from == get_self() || to != get_self()) {
         return;
      }

   }

   void bridge::withdraw(const name &to,
                         const asset &quantity,
                         const string &memo) {
      require_auth(get_self());

      action{
              permission_level{get_self(), active_permission},
              eosio_token_account,
              eosio_token_transfer_action,
              std::make_tuple(get_self(), to, quantity, memo)
      }.send();
   }

} /// namespace bifrost
