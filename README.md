# Bifrost bridge contract

## Dependencies
1. Ensure an appropriate version of `eosio` has been built from source and installed. Installing `eosio` from binaries `is not` sufficient. You can find instructions on how to do it [here](https://github.com/EOSIO/eos/blob/master/README.md) in section `Building from Sources`.
2. Ensure an appropriate version of `eosio.cdt` is installed. Installing `eosio.cdt` from binaries is sufficient, follow the [`eosio.cdt` installation instructions steps](https://github.com/EOSIO/eosio.cdt/tree/master/#binary-releases) to install it. To verify if you have `eosio.cdt` installed and its version run the following command 

```sh
eosio-cpp -v
```

## Build bridge contract simply
```bash
./build
```

#### Build contracts manually

To build the `bifrost.contracts` execute the following commands.

On all platforms except macOS:
```sh
cd you_local_path_to/bifrost.contracts/
rm -fr build
mkdir build
cd build
cmake ..
make -j$( nproc )
cd ..
```

For macOS:
```sh
cd you_local_path_to/bifrost.contracts/
rm -fr build
mkdir build
cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
cd ..
```

## Deploy bridge contract
```bash
cleos set contract [contract_account] /path/to/build/contracts/bifrost.bridge bifrost.bridge.wasm bifrost.bridge.abi -p [contract_account]@active
```

## Usage
### User Action
* Transfer asset to bridge contract and obtain Bifrost v-token. 
```bash
cleos push action eosio.token transfer '["[user_account]", "[contract_account]", "1.0000 EOS", "[user_bifrost_address]@bifrost:[memo]"]' -p [user_account]@active
```

### Bifrsot Action
* Withdraw asset back to Eos. 
Let's assume `alice` is account which user owned, `bifrost` is account which bridge contract deplayed.
```bash
cleos push action [contract_account] withdraw '["[user_account]", "1.0000 EOS", "[user_bifrost_address]@bifrost:[memo]"]' -p [contract_account]@active
```

### Contract Action
* Enable all token deposit to bridge contract. 
```bash
cleos push action [contract_account] activate '[]' -p [contract_account]@active
```

* Disable all token deposit to bridge contract. 
```bash
cleos push action [contract_account] deactivate '[]' -p [contract_account]@active
```

* If user asset has been bridged to Bifrost, make sure to call this action to confirm original deposit action. 
```bash
cleos push action [contract_account] depositcnfm '["deposit_id"]' -p [contract_account]@active
```

* If user asset has been bridged to Bifrost, make sure to call this action to rollback original deposit action. 
```bash
cleos push action [contract_account] depositrlbk '["deposit_id"]' -p [contract_account]@active
```

* Resigter token information.
```bash
cleos push action [contract_account] regtoken '["eosio.token", "EOS,4", "10000.0000 EOS", "1.0000 EOS", "1000.0000 EOS", "10000.0000 EOS", "1"]' -p [contract_account]@active
```

* Enable one token deposit to bridge contract. 
```bash
cleos push action [contract_account] activetk '[]' -p [contract_account]@active
```

* Disable one token deposit to bridge contract. 
```bash
cleos push action [contract_account] deactivetk '[]' -p [contract_account]@active
```

* Update token deposit information. 
```bash
cleos push action [contract_account] setdeposittk '["1.0000 EOS", "1000.0000 EOS", "10000.0000 EOS"]' -p [contract_account]@active
```