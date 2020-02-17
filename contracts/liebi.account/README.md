liebi.account - Liebi 账户创建合约
-----------

# 项目构建
```
mkdir build
cd build
cmake ..
make
```

# 部署合约
```
cleos set contract liebi.acct [contract-dir] [wasm-file] [abi-file]
```

# 初始化合约
cleos push action liebi.acct init '[]' -p liebi.acct@active

# 激活合约
cleos push action liebi.acct active '[]' -p liebi.acct@active

# 关闭合约
cleos push action liebi.acct deactive '[]' -p liebi.acct@active

## 新建账号 => memo 格式：account,新建账户名称,owner_public_key[,active_public_key]
cleos push action eosio.token transfer '["alice", "liebi.acct", "10.0000 EOS", "account,bob,owner_public_key,active_public_key"]' -p alice
