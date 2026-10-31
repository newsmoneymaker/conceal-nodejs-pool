# Installing a Conceal pool

Paths below are examples: the pool in `/opt/conceal-nodejs-pool`, the node and wallet in `/opt/conceal-core`, all run by the user
`ccxpool` (the systemd templates in `deployment/systemd/` use these paths).

## 1. Conceal node and wallet

Build the node and wallet-service from source (C++17, Boost, the project's own vendored dependencies). The node MUST be synced
past its block_major_version 8 hard fork (LWMA1 difficulty, CryptoNight-GPU, halving emission) -- an older build predating that
fork validates the wrong proof of work entirely:

```
git clone --recursive https://github.com/ConcealNetwork/conceal-core && cd conceal-core
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j4 Daemon PaymentGateService
# binaries in build/src/: conceald, walletd
```

Run the node (`deployment/systemd/ccx-node.service`):
`conceald --data-dir /opt/conceal-core/data --p2p-bind-ip 0.0.0.0 --p2p-bind-port 15000 --rpc-bind-ip 127.0.0.1 --rpc-bind-port 16000`.
It finds its peers by itself and synchronises over time; check with
`curl -s 127.0.0.1:16000/getinfo` (look for `"synced": true` and `"block_major_version": 8` or newer). Open the P2P port 15000
in your firewall for incoming peers.

Create the pool wallet with `walletd`'s own JSON-RPC (`createAddress`), then note the returned address -- it is the pool
address (`poolServer.poolAddress`): every block template pays its coinbase there. **Back up the wallet's keys right away
(`getSpendKeys`/`getViewKey`, or the wallet container file + its password) and write them down somewhere safe and offline** --
do not let them only ever exist inside the running wallet-service. An address created with `walletd`'s plain `createAddress`
(no arguments) is an independently generated keypair, not derived from any single seed.

Then run the wallet-service (`deployment/systemd/ccx-wallet-rpc.service`): `walletd --container-file /opt/conceal-core/wallet/pool.wallet
--container-password <password> --bind-address 127.0.0.1 --bind-port 46011 --daemon-address 127.0.0.1 --daemon-port 16000`.
It listens on the local machine only: keep only the coins that are about to be paid out in this wallet.

If the node and the pool run on different machines, forward both RPC ports over SSH instead of exposing them (this daemon's own
RPC has no authentication at all) -- two `-L` forwards over one SSH connection, same pattern as `ccx-tunnel.service` in this
project's own production deployment.

## 2. blobtool

Conceal's `getblocktemplate` does not hand back a ready-to-hash blob directly -- `ccx_blobtool` (source in `blobtool/`, see
`blobtool/README.md`) is a small executable **added to the node's own source tree** (a new CMake target inside your
`conceal-core` checkout, linked directly against its compiled `CryptoNoteCore`/`P2P`/`Rpc`/`System`/`Http`/`Logging`/`Common`/
`crypto`/`BlockchainExplorer`/`Serialization` libraries) that re-derives the hashing blob's nonce offset from the template's own
encoding. Unlike some other CryptoNote forks, Conceal needs no merge-mining-tag mutation before hashing, so this is simpler than
it sounds -- see `blobtool/README.md` for the exact install steps. Point `config.json`'s `blobtool.remotePath` (plus
`blobtool.sshHost`, or `127.0.0.1` if it runs locally) at the resulting binary. It only runs once per new block template (every
`poolServer.templateRefresh` seconds), not per share, so an SSH round trip is fine.

## 3. Hasher

The helper (`hasher/ccxhash`) is this coin's own real CryptoNight-GPU (`cn_pow_hash_v3`, Ryo Currency Project), vendored
verbatim from poolpayminer's own `src/crypto/ccx_gpu/` (symbols renamed to avoid any collision with another vendored
CryptoNight-family hash elsewhere on the same machine) -- the pool's share check and the real miner's PoW are identical by
construction. Build it with `cd hasher && make` (needs a CPU with AES-NI and AVX2). No key/dataset setup is needed, unlike a
RandomX-based pool's own hasher.

## 4. Redis

Use a dedicated instance with a password and AOF (`deployment/redis-pool.conf.example`, unit `ccx-pool-redis`, port 6404).

## 5. The pool

```
cd /opt/conceal-nodejs-pool && npm install --production
cp config_examples/ccx.json config.json      # then edit it
```

Edit `config.json`: `poolHost`, `poolServer.poolAddress`, the ports and the certificate for TLS (`poolServer.sslCert/sslKey`), `redis`,
`api.password`, `blockUnlocker.poolFee` and `donations`, `payments`, `node`, `wallet`, `hasher.path`, `blobtool`.
**Keep `payments.dryRun: true` until the rehearsal below.**

```
cp deployment/systemd/*.service /etc/systemd/system/ && systemctl daemon-reload
systemctl enable --now ccx-pool-redis ccx-node ccx-wallet-rpc
systemctl enable --now ccx-pool ccx-pool-api ccx-pool-unlocker ccx-pool-payments ccx-pool-charts
```

The pool runs as separate modules (`init.js -module=pool|api|unlocker|payments|chartsDataCollector`), each in its own unit.
Until payouts are proven, restrict the stratum ports with `poolServer.allowIPs`. **After editing `config.json`, restart every
one of these units, not just the one you were actively changing -- each runs on its own in-memory copy loaded at startup.**

## 6. Website

Copy `website_example/` to the web root, set `poolHost`, the contact and links in `config.js`, and proxy `/api` to the pool API on
127.0.0.1:8146 (`deployment/apache-vhost.conf.example` exposes only the read-only methods).

## 7. Rehearse the payments

1. `payments.dryRun: true`: the log of `ccx-pool-payments` shows what would be paid.
2. Fund the pool wallet with a few coins (or wait for the first block), credit a small balance in Redis to your own test addresses
   (`<coin>:workers:<address>`, field `balance`), set `payments.onlyAccounts` to them, `dryRun: false`, and watch the payout confirm.
3. Remove the test accounts from Redis and set `onlyAccounts` to `[]`.

Good to know: the real coinbase unlock window is 10 blocks / ~20 minutes (`blockUnlocker.depth` is set higher to be safe);
`sendTransaction`'s `fee`/`anonymity` fields have no automatic default the way a Monero-style wallet infers them (see
`payments.sendFee`/`sendAnonymity` in config.json); `deployment/pause-payments.sh` stops new payouts at once; a wallet cannot
abandon a transaction, so a payout that stays unconfirmed for `pendingTimeoutHours` is only reported in the log and must be
checked by hand.
