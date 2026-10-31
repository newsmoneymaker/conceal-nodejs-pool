# conceal-nodejs-pool

Mining pool software for **Conceal (CCX)** written in Node.js: a stratum server for Conceal's real post-hard-fork
proof of work, CryptoNight-GPU (`cn/gpu`), with share checking, block accounting through the pool wallet, and
batch payouts. It is a fork of [cryptonote-nodejs-pool](https://github.com/dvandal/cryptonote-nodejs-pool) by
Dvandal (GNU GPL v2), via its [kryptokrona-nodejs-pool](https://github.com/newsmoneymaker/kryptokrona-nodejs-pool)
adaptation, for Conceal's own daemon and wallet-service RPC. Live example: <https://conceal.pool-pay.com>.

## What it does

* **Stratum server** (plain TCP and TLS ports, the plain cryptonote-nodejs-pool JSON-RPC shape -- `login`/`job`/
  `submit`): the pool asks the node for a block template (`getblocktemplate`), has `blobtool` (a small C++ helper
  linked against the node's own compiled code) build the real hashing blob, gives every miner that blob and a
  share target that follows the miner's hashrate (vardiff), and checks every share itself with a small C++
  helper (`hasher/ccxhash`): CryptoNight-GPU (`cn_pow_hash_v3`, Ryo Currency Project) of the hashing blob. A
  share that meets the network difficulty is submitted to the node with `submitblock`. Unlike some other
  CryptoNote forks, Conceal needs **no merge-mining-tag mutation** before hashing -- `blobtool`'s `TEMPLATE`/
  `HASHING` outputs are just the original block template's own encoding.
  **Use poolpayminer 1.1.33+ with `-a cn/gpu`** -- a stock [XMRig](https://github.com/xmrig/xmrig) does not
  know this algorithm, and it is NOT the same algorithm as Conceal's own CryptoNight variant from before its
  block_major_version 8 hard fork.
* **Accounts** are Conceal addresses (`ccx7...`, base58, 98 characters, 3-byte varint prefix), optionally
  `+worker`, with a reward mode prefix `prop:` (shared, default) or `solo:`.
* **Rewards:** PROP with time weighting (slush) or SOLO.
* **Block unlocker:** a block is settled after `depth` blocks (the real coinbase unlock window is 10 blocks /
  ~20 minutes; the pool waits longer to be safe). The reward is read directly off the block header's own
  `reward` field (no `miner_tx_hash`/wallet lookup needed -- unlike some other CryptoNote forks, this daemon's
  `getblockheaderbyheight` already carries the reward). A block that is no longer on the chain is marked
  orphaned and nothing is credited.
* **Payment processor:** everyone who is due is paid in **one wallet `sendTransaction` per round** (the network
  fee is shared by the paid miners or paid by the pool). The balance is debited before sending; a batch whose
  outcome is unknown (crash, timeout) is found again among the wallet's recent transactions on its own address
  and never sent twice; refused or failed batches go back to the balances. Dry-run mode, a whitelist for
  rehearsals and an emergency brake (`deployment/pause-payments.sh`).
* **Website and API:** a ready website (`website_example/`) with the dashboard and its graphs, blocks, payments,
  top miners, worker statistics, a "Getting started" page with a config generator, and the public read-only
  JSON API.
* **Protection against connection floods:** limits per IP, a login deadline, an optional IP allow list, banning
  of miners with many invalid shares.

## Developer donation (please read)

The pool takes a **developer donation** from the reward of every block it finds, before the miners' shares are
computed. It is configured in `config.json` (see `config_examples/ccx.json`):

```json
"blockUnlocker": {
  "poolFee": 2,
  "donations": { "ccx7PT3VrsbUHbFugUnGTJVZ58n8RrSZuSL3TWhL4iSAez6LL7QKKER42nkA4gJay1dKqRn5zSEkXYkSD1gBrwCq6Vj1Vs9RQV": 1 }
}
```

* `poolFee` is the fee of the pool operator (percent). `donations` is a table `Conceal address -> percent` for
  the developers of this software; **the default is 1% to the address of the project's own pool**. The
  donation is paid like any other balance, and is additive with `poolFee` (total real cut from miners here is
  `poolFee + donations`, e.g. 2% + 1% = 3%).
* It is included in the "Pool Fee" figure that the pool's website and API show to the miners (as in the
  original software).
* You are free to change the percentage, the address or to empty the table (`"donations": {}`): it is your
  pool and the license is the GPL. Please tell your miners the truth about the fees of your pool.
* This has nothing to do with the miner poolpayminer (a separate project with its own fee).

## Installation

Requirements: Linux, Node.js 18 or newer, Redis, a Conceal node (`conceald`, built from
[github.com/ConcealNetwork/conceal-core](https://github.com/ConcealNetwork/conceal-core)) synchronised with the
network (block_major_version 8 or newer -- the node must have gone through the CN-GPU/LWMA1/halving hard fork)
and its own `walletd` wallet-service with the pool wallet, a C++17 compiler (for `blobtool`, linked against the
node's own compiled code, and for the `ccxhash` helper, which needs AES-NI and AVX2), a web server for the
website and a TLS certificate for the TLS stratum ports.

`blobtool` is a small CLI mode added to the node's own compiled helper: given a hex `blocktemplate_blob`, it
outputs `TEMPLATE`/`HASHING`/`TEMPLATE_NONCE_OFFSET`/`HASHING_NONCE_OFFSET`/`PREVHASH` in one shot (see
`lib/blobtool.js` and the node-side `ccx_blobtool.cpp` source, built alongside the node itself). If your node
and pool run on separate hosts, point `config.json`'s `blobtool.sshHost`/`remotePath` at the node host and run
`blobtool` there over SSH, same as this pool's own live deployment.

## Miners

**Stock [XMRig](https://github.com/xmrig/xmrig) does NOT support this algorithm.** Use
[poolpayminer](https://github.com/newsmoneymaker/poolpayminer) 1.1.33+ (Windows and Linux, free, has its own
fee) instead. Example: `poolpayminer -o conceal.pool-pay.com:5201 --tls -u YOUR_CCX_ADDRESS+rigname -p x -a cn/gpu`.

## Money warning

The payment processor moves real coins. Rehearse first: `"dryRun": true`, then a whitelist (`onlyAccounts`)
with a few small payouts of your own, then enable it. A transaction that has been sent to the network can not
be cancelled. Keep the wallet's keys/seed backed up separately and privately -- an address created with the
wallet-service's plain `createAddress` (no arguments) is an independently generated keypair, NOT derived from
any seed: back up its own secret spend key (`getSpendKeys`) and view key (`getViewKey`) separately, or it
cannot be recovered.

## License and credits

GNU GPL v2 (see [LICENSE](LICENSE)). Based on cryptonote-nodejs-pool, Copyright (c) Dvandal and contributors.
Conceal itself is a separate program and is not included.
