# Changes

## 1.0.0 (2026-10-31)

* First release: Conceal (CCX) pool, adapted from kryptokrona-nodejs-pool (itself derived from
  qwc-nodejs-pool/veil-nodejs-pool/epic-nodejs-pool). Stratum for CryptoNight-GPU (`cn/gpu` in
  poolpayminer), plain cryptonote-nodejs-pool JSON-RPC (`login`/`job`/`submit`), share checking with the
  `ccxhash` helper (the coin's own real hash, vendored verbatim from poolpayminer's own `src/crypto/ccx_gpu/`
  so the pool's check and the real miner's PoW are identical by construction).
* Conceal's real proof-of-work since its block_major_version 8 hard fork (LWMA1 difficulty adjustment,
  CN-GPU, halving emission) is CryptoNight-GPU -- NOT the older CryptoNight variant ("cn/ccx") this coin
  used before that fork, and not the same algorithm as any other CryptoNight-family coin this pool
  template has ever adapted for.
* `blobtool` (a new "full" mode added to the node's own compiled helper, linked against real
  `CryptoNoteCore`) builds the hashing blob from a hex `blocktemplate_blob` in one shot. Unlike
  Kryptokrona, Conceal needs **no merge-mining-tag mutation** before hashing: `TEMPLATE`/`HASHING` are
  just the original blob's own encoding.
* The node's real RPC method names match the template's assumptions (`getblocktemplate`/
  `getlastblockheader`/`submitblock`/`getblockheaderbyheight`), but the REPLY SHAPE did not always match:
  `getblockheaderbyheight` has no `miner_tx_hash` field here (unlike the coin this template was copied
  from), so `blockUnlocker.js`'s `blockReward()` was rewritten to read `header.reward` directly instead of
  querying a transaction -- this is a genuine functional fix found and made BEFORE going live, not after a
  report (it would have silently blocked every future payout otherwise).
* Real consensus facts read directly from the node's own `CryptoNoteConfig.h`/`Currency.cpp` source, not
  assumed from the template: `COIN = 1,000,000` (6 decimals, not the template's 100,000,000/8), block
  target 120s, address prefix `0x7ad4` (`ccx7...`, 98 base58 characters, 3-byte varint -- not a plain
  single-byte prefix), coinbase unlock window 10 blocks (~20 minutes).
* Real end-to-end tested against a live, fully-synced CCX mainnet node and poolpayminer: real shares
  accepted with no `--threads` override, on both Linux and Windows.
* Payments stay `dryRun` until a real block matures and the first real payout is verified end to end
  through the pool's own code.
