# ccx_blobtool

Builds the real, correct PoW-hashing blob for a Conceal block template, linked directly against the real
node's own compiled `CryptoNoteCore`/`P2P`/`Rpc`/`System`/`Http`/`Logging`/`Common`/`crypto`/
`BlockchainExplorer`/`Serialization` libraries (not reimplemented), so there is no risk of a hand-rolled
parser disagreeing with consensus.

Unlike some other CryptoNote forks (e.g. Kryptokrona), Conceal's reference miner does **not** rewrite the
coinbase transaction's `extra` field with a merge-mining tag before hashing (confirmed directly against
`src/CryptoNoteCore/Currency.cpp`) -- so this tool's "full" mode just re-derives the hashing blob's nonce
offset from the template's own encoding; `TEMPLATE`/`HASHING` are the original `blocktemplate_blob`'s own
bytes, unmodified.

This is a **source addition to the node itself** (a new CMake executable target inside a
[conceal-core](https://github.com/ConcealNetwork/conceal-core) checkout), not a standalone tool linked
against pre-built libraries after the fact -- it has to be added and built together with the node.

## Install

1. Copy `ccx_blobtool.cpp` into the root of a `conceal-core` checkout's `src/` directory.
2. Add it to `src/CMakeLists.txt`, next to the existing `Daemon` target:
   ```cmake
   add_executable(ccx_blobtool ccx_blobtool.cpp)
   ...
   target_link_libraries(ccx_blobtool CryptoNoteCore P2P Rpc System Http Logging Common crypto libminiupnpc-static BlockchainExplorer ${Boost_LIBRARIES} Serialization)
   ```
3. Build the node as usual (`cmake .. && make`); `ccx_blobtool` appears in `build/src/` alongside `conceald`.

## Usage

```
./ccx_blobtool <blocktemplate_blob hex>
```

(a single hex argument runs "full" mode by default; `hash`/`submit` modes also exist for manual debugging,
see the source). Prints 5 lines: `TEMPLATE <hex>` (the block to submit with the nonce spliced in -- here
just the original `blocktemplate_blob`, since no mutation is needed), `HASHING <hex>` (the exact bytes
`cn_pow_hash_v3`/CryptoNight-GPU hashes), `TEMPLATE_NONCE_OFFSET <n>`, `HASHING_NONCE_OFFSET <n>` (found
empirically by a marker-swap-and-locate technique against a real live block template, not by hand-tracing
the varint-timestamp-dependent serializer -- both offsets are 39), and `PREVHASH <hex>` (straight from the
parsed struct). `lib/blobtool.js` in this repo is the Node.js wrapper that calls this binary over SSH.
