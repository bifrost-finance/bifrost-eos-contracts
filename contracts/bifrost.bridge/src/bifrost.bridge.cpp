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

      // check token active, quantity
      tokens _tokens(get_self(), eosio_token_contract.value);
      auto idx = _tokens.get_index<"tokensym"_n>();
      auto token = idx.get(quantity.symbol.code().raw(),"token with symbol does not support" );
      check(token.active, "token not active");
      check(quantity < token.deposit_min_once, "quantity lower than deposit_min_once");
      check(quantity > token.deposit_max_once, "quantity greater than deposit_max_once");

      // parse and check memo
      memo_info_type memo_info = get_memo_info(memo);
      check(memo_info.peerchain == "bifrost"_n, "peerchain: invalid peerchain");

      // record to database
      deposits _deposit(get_self(), from.value);
      _deposit.emplace(get_self(), [&](auto &dt) {
         dt.id = ++_gstate.deposit_id;
         dt.contract = eosio_token_contract;
         dt.from = from;
         dt.quantity = quantity;
         dt.memo = memo;
         dt.status = 0;
      });

      _tokens.modify(token, same_payer, [&](auto &t) {
         t.deposit_total += quantity;
         t.deposit_total_times += 1;
      });
   }

   /**
    * deposit confirm
    */
   void bridge::depositcnfm(uint64_t deposit_id) {
      require_auth(get_self());

      // lookup deposit info
      deposits _deposit(get_self(), get_self().value);
      auto dt = _deposit.find(deposit_id);
      check(dt != _deposit.end(), "deposit id does not exists in table");
      check(dt->status == 0, "deposit status must be 0");

      // modify deposit status
      _deposit.modify(dt, same_payer, [&](auto &dt) {
         dt.status = 1;
      });
   }

   /**
    * deposit rollback
    */
   void bridge::depositrlbk(uint64_t deposit_id) {
      require_auth(get_self());

      // lookup deposit info
      deposits _deposit(get_self(), get_self().value);
      auto dt = _deposit.find(deposit_id);
      check(dt != _deposit.end(), "deposit id does not exists in table");
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
      _deposit.modify(dt, same_payer, [&](auto &dt) {
         dt.status = 2;
      });

      tokens _tokens(get_self(), eosio_token_contract.value);
      auto idx = _tokens.get_index<"tokensym"_n>();
      auto token = idx.get(quantity.symbol.code().raw(),"token with symbol does not support" );
      _tokens.modify(token, same_payer, [&](auto &t) {
         t.deposit_total -= quantity;
         t.deposit_total_times -= 1;
      });
   }

   void bridge::withdraw(const name &contract,
                         const name &to,
                         const asset &quantity,
                         const string &memo) {
      require_auth(get_self());

      auto sym = quantity.symbol;
      check(sym.is_valid(), "invalid symbol name");
      check(memo.size() <= 256, "memo has more than 256 bytes");

      action{
              permission_level{get_self(), active_permission},
              contract,
              transfer_action,
              std::make_tuple(get_self(), to, quantity, memo)
      }.send();

      tokens _tokens(get_self(), eosio_token_contract.value);
      auto idx = _tokens.get_index<"tokensym"_n>();
      auto token = idx.get(quantity.symbol.code().raw(),"token with symbol does not support" );
      _tokens.modify(token, same_payer, [&](auto &t) {
         t.withdraw_total += quantity;
         t.withdraw_total_times += 1;
      });
   }

   void bridge::active() {
      require_auth(get_self());

      _gstate.active = true;
   }

   void bridge::deactive() {
      require_auth(get_self());

      _gstate.active = false;
   }

   void bridge::regtoken(const name      &token_contract,
                         const symbol    &token_symbol,
                         const asset     &max_accept,
                         const asset     &deposit_min_once,
                         const asset     &deposit_max_once,
                         const asset     &deposit_max_daily,
                         bool            active) {
      require_auth(get_self());

      check(is_account(token_contract), "token_contract account does not exist");
      check(token_symbol.is_valid(), "token_symbol invalid");
      check(max_accept.is_valid(), "invalid max_accept");
      check(max_accept.amount > 0, "max_accept must be positive");
      check(max_accept.symbol == token_symbol &&
            deposit_min_once.symbol == token_symbol &&
            deposit_max_once.symbol == token_symbol &&
            deposit_max_daily.symbol == token_symbol &&
            deposit_min_once.amount > 0 &&
            deposit_max_once.amount > deposit_min_once.amount &&
            deposit_max_daily.amount > deposit_max_once.amount, "invalid asset");

      tokens _tokens(get_self(), token_contract.value);
      auto existing = _tokens.find(token_contract.value);
      check(existing == _tokens.end(), "token contract already exist");
      _tokens.emplace(get_self(), [&](auto &r) {
         r.token_contract = token_contract;
         r.accept = asset{0, token_symbol};
         r.max_accept = max_accept;
         r.deposit_min_once = deposit_min_once;
         r.deposit_max_once = deposit_max_once;
         r.deposit_max_daily = deposit_max_daily;
         r.deposit_total = asset{0, token_symbol};
         r.deposit_total_times = 0;
         r.withdraw_total = asset{0, token_symbol};
         r.withdraw_total_times = 0;
         r.active = active;
      });
   }

   void bridge::activetk(const name &token_contract, const symbol &token_symbol) {
      require_auth(get_self());

      tokens _tokens(get_self(), eosio_token_contract.value);
      auto idx = _tokens.get_index<"tokensym"_n>();
      auto token = idx.get(token_symbol.code().raw(),"token with symbol does not support" );
      _tokens.modify(token, same_payer, [&](auto &t) {
         t.active = true;
      });
   }

   void bridge::deactivetk(const name &token_contract, const symbol &token_symbol) {
      require_auth(get_self());

      tokens _tokens(get_self(), eosio_token_contract.value);
      auto idx = _tokens.get_index<"tokensym"_n>();
      auto token = idx.get(token_symbol.code().raw(),"token with symbol does not support" );
      _tokens.modify(token, same_payer, [&](auto &t) {
         t.active = false;
      });
   }

   void bridge::setdeposittk(const name      &token_contract,
                             const symbol    &token_symbol,
                             const asset     &deposit_min_once,
                             const asset     &deposit_max_once,
                             const asset     &deposit_max_daily) {
      require_auth(get_self());

      tokens _tokens(get_self(), eosio_token_contract.value);
      auto idx = _tokens.get_index<"tokensym"_n>();
      auto token = idx.get(token_symbol.code().raw(),"token with symbol does not support" );
      _tokens.modify(token, same_payer, [&](auto &t) {
         t.deposit_min_once = deposit_min_once;
         t.deposit_max_once = deposit_max_once;
         t.deposit_max_daily = deposit_max_daily;
      });
   }

} /// namespace bifrost
