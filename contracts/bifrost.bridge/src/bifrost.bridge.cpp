#include <bifrost.bridge/bifrost.bridge.hpp>
#include <bifrost.bridge/utils.hpp>

namespace bifrost {

   bridge::bridge(name s, name code, datastream<const char *> ds) :
           contract(s, code, ds),
           _global_state(get_self(), get_self().value) {
      _gstate = _global_state.exists() ? _global_state.get() : globalstate{};
   }

   bridge::~bridge() {
      _global_state.set(_gstate, get_self());
   }

   void bridge::deposit_notify(const name &from,
                               const name &to,
                               const asset &quantity,
                               const string &memo) {
      if (from == get_self() || to != get_self()) {
         return;
      }

      // check contract active
      check(_gstate.active, "contract not active");

      // parse and check memo
      memo_info_type memo_info = get_memo_info(memo);
//      check(""_n == memo_info.receiver, "receiver: invalid bifrost address format");
      check(memo_info.peerchain == "bifrost"_n, "peerchain: invalid peerchain");

      // record to database
      deposits deposit_table(get_self(), from.value);
      deposit_table.emplace(get_self(), [&](auto &dt) {
         dt.id = ++_gstate.deposit_id;
         dt.contract = eosio_token_account;
         dt.from = from;
         dt.quantity = quantity;
         dt.memo = memo;
         dt.status = 0;
      });
   }

   /**
    * deposit confirm
    */
   void bridge::depositcnfm(uint64_t deposit_id) {
      require_auth(get_self());

      // lookup deposit info
      deposits deposit_table(get_self(), get_self().value);
      auto dt = deposit_table.find(deposit_id);
      check(dt != deposit_table.end(), "deposit id does not exists in table");
      check(dt->status == 0, "deposit status must be 0");

      // modify deposit status
      deposit_table.modify(dt, same_payer, [&](auto &dt) {
         dt.status = 1;
      });
   }

   /**
    * deposit rollback
    */
   void bridge::depositrlbk(uint64_t deposit_id) {
      require_auth(get_self());

      // lookup deposit info
      deposits deposit_table(get_self(), get_self().value);
      auto dt = deposit_table.find(deposit_id);
      check(dt != deposit_table.end(), "deposit id does not exists in table");
      check(dt->status == 0, "deposit status must be 0");

      // refund asset to original account
      name contract = dt->contract;
      name from = get_self();
      name to = dt->from;
      asset quantity = dt->quantity;
      string memo = "Deposit rollback";
      action{
              permission_level{get_self(), active_permission},
              contract,
              transfer_action,
              std::make_tuple(from, to, quantity, memo)
      }.send();

      // modify deposit status
      deposit_table.modify(dt, same_payer, [&](auto &dt) {
         dt.status = 2;
      });
   }

   void bridge::withdraw(const name &contract,
                         const name &to,
                         const asset &quantity,
                         const string &memo) {
      require_auth(get_self());

      // check active
      check(_gstate.active, "contract not active");

      // check contract

      auto sym = quantity.symbol;
      check(sym.is_valid(), "invalid symbol name");
      check(memo.size() <= 256, "memo has more than 256 bytes");

      action{
              permission_level{get_self(), active_permission},
              contract,
              transfer_action,
              std::make_tuple(get_self(), to, quantity, memo)
      }.send();
   }

   void bridge::activate() {
      require_auth(get_self());

      _gstate.active = true;
   }

   void bridge::deactivate() {
      require_auth(get_self());

      _gstate.active = false;
   }

} /// namespace bifrost
