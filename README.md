# Bifrost bridge contract

## Compile & Deploy bridge contract
```bash
# compile bridge contract
mkdir build
cd build
cmake ..
make

# deploy bridge contract
cleos set contract [contract_account] /path/to/build/contracts/bifrost.bridge bifrost.bridge.wasm bifrost.bridge.abi -p [contract_account]@active
```

## User Action
* Transfer asset to bridge contract and obtain Bifrost v-token
```bash
cleos push action eosio.token transfer '["alice", "[contract_account]", "1.0000 EOS", "[user_bifrost_address]@bifrost:[memo]"]' -p alice@active
```

## Bifrsot Action
* Withdraw asset back to Eos
```bash
cleos push action [contract_account] withdraw '["alice", "1.0000 EOS", "[user_eos_account]@eos:[memo]"]' -p [contract_account]@active
```

## Contract Action
* Enable asset deposit to/withdraw from bridge contract
```bash
cleos push action [contract_account] activate '[]' -p [contract_account]@active
```
* Disable asset deposit to/withdraw from bridge contract
```bash
cleos push action [contract_account] deactivate '[]' -p [contract_account]@active
```