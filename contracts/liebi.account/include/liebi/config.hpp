#pragma once

#define ID_ACTIVE 100   // 是否激活
#define VAL_ACTIVE true

// new account
#define ID_NEW_ACCOUNT_FEE_QUANTITY 600 // 新建账户费率
#define VAL_NEW_ACCOUNT_FEE_QUANTITY 0
//#define VAL_NEW_ACCOUNT_FEE_QUANTITY 1000     // 0.1000 EOS for fee
#define ID_STAKE_NET_QUANTITY 601       // 新建账户抵押 NET
#define VAL_STAKE_NET_QUANTITY 100              // 0.0100 EOS for NET
#define ID_STAKE_CPU_QUANTITY 602       // 新建账户抵押 CPU
#define VAL_STAKE_CPU_QUANTITY 1900             // 0.1900 EOS for CPU


#define EOS_SYMBOL symbol("EOS", 4)

#define EOS_TOKEN_CONTRACT      name("eosio.token")
#define EOS_SYSTEM_CONTRACT     name("eosio")

#define LIEBI_NEW_ACCT_FEE_ACCOUNT      name("lbstakprofit")   // 创建账户收费账户
