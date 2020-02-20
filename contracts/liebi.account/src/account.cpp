#include "liebi/account.hpp"
#include "liebi/utils.hpp"
#include "eosio/native.hpp"

namespace liebi {

    void account::transfer_receipt(const name from, const name to, const asset quantity, const string memo) {
        if (from == _self || to != _self) {
            return;
        }

        check(quantity.is_valid(), "invalid transfer amount.");
        check(quantity.amount > 0, "transfer amount not positive");

        check(quantity.symbol == EOS_SYMBOL, "invalid symbol!");

        // 解析 memo:
        //      新建账户   memo: account,新建账户名称,owner_public_key[,active_public_key][:第三方memo]
        vector <string> pieces1;
        split(memo, ":", pieces1);
        uint8_t memo_size1 = pieces1.size();
        if (memo_size1 != 1 && memo_size1 != 2) {
           // 参数不正确，直接返回
           return;
        }
        vector <string> pieces2;
        split(pieces1[0], ",", pieces2);
        uint8_t memo_size2 = pieces2.size();
        if (memo_size2 != 3 && memo_size2 != 4) {
            // 参数不正确，直接返回
            return;
        }

        if (pieces2[0] == "account") {
            assert_active();

            // get account name
            check(pieces2[1].size() == 12, "invalid account size");
            name account = name(pieces2[1]);

            // get owner key and active key
            string owner_key_str = pieces2[2];
            string active_key_str = owner_key_str;
            if (memo_size2 == 4 && !pieces2[3].empty()) {
                active_key_str = pieces2[3];
            }

            asset surplus_quantity = create_account(account, quantity, owner_key_str, active_key_str);
            if (surplus_quantity.amount > 0) {
               action(
                       permission_level{_self, "active"_n},
                       EOS_TOKEN_CONTRACT,
                       "transfer"_n,
                       std::make_tuple(_self, from, surplus_quantity,
                                       string("surplus quantity of new account"))
               ).send();
            }
         }
    }

    asset account::create_account(const name account, const asset quantity,
                               const string owner_key_str, const string active_key_str) {

        // see https://eosio.stackexchange.com/questions/847/how-to-get-current-last-ram-price
        // calculate 4k ram price
        symbol ramcore_symbol = symbol(symbol_code("RAMCORE"), 4);
        rammarket_index rammarket(EOS_SYSTEM_CONTRACT, EOS_SYSTEM_CONTRACT.value);
        const auto &st = rammarket.get(ramcore_symbol.raw(), "ram market does not exist");
        double quote_balance = (double) st.quote.balance.amount / 10000;
        double base_balance = (double) st.base.balance.amount;
        double ram_price_per_byte = quote_balance / base_balance;
        double ram_price_per_kbyte = ram_price_per_byte * 1024;
        uint64_t ram_4k_amount = (uint64_t)((ram_price_per_kbyte * 4 * 10000) + 0.5);
        uint64_t ram_256byte_amount = (uint64_t)((ram_price_per_byte * 256 * 10000) + 0.5);
        asset ram_4k_quantity = asset(ram_4k_amount, EOS_SYMBOL);
        asset ram_256byte_quantity = asset(ram_256byte_amount, EOS_SYMBOL);

        // get net and cpu quantity
        asset stake_net_quantity = asset(get_dict_val(ID_STAKE_NET_QUANTITY), EOS_SYMBOL);
        asset stake_cpu_quantity = asset(get_dict_val(ID_STAKE_CPU_QUANTITY), EOS_SYMBOL);
        check(stake_net_quantity.amount > 0, "not enough net quantity");
        check(stake_cpu_quantity.amount > 0, "not enough cpu quantity");

        // set fee quantity
        asset fee_quantity = asset(get_dict_val(ID_NEW_ACCOUNT_FEE_QUANTITY), EOS_SYMBOL);
        check(fee_quantity.amount >= 0, "invalid fee quantity");

        // new account need at least 0.0001 EOS
        asset new_account_quantity = asset(1, EOS_SYMBOL);

        // calculate surplus quantity
        asset surplus_quantity = quantity - fee_quantity - ram_4k_quantity - ram_256byte_quantity
                                 - stake_net_quantity - stake_cpu_quantity - new_account_quantity;
        check(surplus_quantity.amount >= 0, "not enough quantity");

        // init authority
        public_key owner_key = str_to_pub(owner_key_str);
        public_key active_key = str_to_pub(active_key_str);
        eosiosystem::authority owner_authority{1, {{owner_key, 1}}, {}, {}};
        eosiosystem::authority active_authority{1, {{active_key, 1}}, {}, {}};

        // get create account
        name creator = _self;

        // new account
        action(
                permission_level{creator, "active"_n},
                EOS_SYSTEM_CONTRACT,
                "newaccount"_n,
                std::make_tuple(creator, account, owner_authority, active_authority)
        ).send();

        // buy ram for account
        name payer = _self;
        name receiver = account;
        uint64_t bytes = 1024 * 4;
        action(
                permission_level{creator, "active"_n},
                EOS_SYSTEM_CONTRACT,
                "buyrambytes"_n,
                std::make_tuple(payer, receiver, bytes)
        ).send();
        action(
                permission_level{creator, "active"_n},
                EOS_SYSTEM_CONTRACT,
                "buyrambytes"_n,
                std::make_tuple(payer, receiver, 256)
        ).send();

        // delegate net and cpu for account
        name from = _self;
        bool transfer = true;
        action(
                permission_level{creator, "active"_n},
                EOS_SYSTEM_CONTRACT,
                "delegatebw"_n,
                std::make_tuple(from, receiver, stake_net_quantity, stake_cpu_quantity, transfer)
        ).send();

        if (fee_quantity.amount > 0) {
            name to_fee_account = LIEBI_NEW_ACCT_FEE_ACCOUNT;
            action(
                    permission_level{creator, "active"_n},
                    EOS_TOKEN_CONTRACT,
                    "transfer"_n,
                    std::make_tuple(from, to_fee_account, fee_quantity, string("new account fee"))
            ).send();
        }

        action(
                permission_level{creator, "active"_n},
                EOS_TOKEN_CONTRACT,
                "transfer"_n,
                std::make_tuple(from, receiver, new_account_quantity,
                                string("Thank you for using, liebi.com is a professional blockchain service provider."))
        ).send();

        if (surplus_quantity.amount > 0 && quantity.amount > surplus_quantity.amount) {
            return surplus_quantity;
        }

        return asset(0, EOS_SYMBOL);
    }


    void account::init() {
        require_auth(_self);

        init_dict(ID_ACTIVE, VAL_ACTIVE);

        init_dict(ID_NEW_ACCOUNT_FEE_QUANTITY, VAL_NEW_ACCOUNT_FEE_QUANTITY);
        init_dict(ID_STAKE_NET_QUANTITY, VAL_STAKE_NET_QUANTITY);
        init_dict(ID_STAKE_CPU_QUANTITY, VAL_STAKE_CPU_QUANTITY);
    }

    void account::setdict(const uint64_t id, const uint64_t value) {
        require_auth(_self);

        upsert_dict_val(id, value);
    }

    void account::active() {
        require_auth(_self);

        upsert_dict_val(ID_ACTIVE, true);
    }

    void account::deactive() {
        require_auth(_self);

        upsert_dict_val(ID_ACTIVE, false);
    }

    void account::init_dict(uint64_t id, uint64_t val) {
        dict_index dicts(_self, _self.value);
        auto pos = dicts.find(id);
        if (pos == dicts.end()) {
            dicts.emplace(_self, [&](auto &a) {
                a.id = id;
                a.val = val;
            });
        }
    }

    // 新增或更新字典
    void account::upsert_dict_val(uint64_t id, uint64_t val) {
        dict_index dicts(_self, _self.value);
        auto pos = dicts.find(id);
        if (pos == dicts.end()) {
            dicts.emplace(_self, [&](auto &a) {
                a.id = id;
                a.val = val;
            });
        } else {
            dicts.modify(pos, same_payer, [&](auto &a) {
                a.val = val;
            });
        }
    }

    uint64_t account::get_dict_val(uint64_t id) {
        dict_index dicts(_self, _self.value);
        auto dict_iter = dicts.find(id);
        check(dict_iter != dicts.end(), "invalid dict id");

        return dict_iter->val;
    }

    void account::assert_active() {
        dict_index dicts(_self, _self.value);
        auto pos = dicts.find(ID_ACTIVE);
        check(pos != dicts.end() && (pos->val == 1), "maintaining ...");
    }

    void account::split(const std::string &s, const std::string &seperator, std::vector <std::string> &v) {
        std::string::size_type pos1, pos2;
        pos2 = s.find(seperator);
        pos1 = 0;
        while (std::string::npos != pos2) {
            v.push_back(s.substr(pos1, pos2 - pos1));

            pos1 = pos2 + seperator.size();
            pos2 = s.find(seperator, pos1);
        }
        if (pos1 != s.length()) v.push_back(s.substr(pos1));
    }
} /// namespace liebi
