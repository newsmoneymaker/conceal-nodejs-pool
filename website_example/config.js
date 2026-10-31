/* Conceal (CCX) pool website settings */

// Coin name shown next to the pool name (must equal the "symbol" of the pool config)
var parentCoin = "CCX";

// Pool API: proxied by the web server from /api to the pool's API on 127.0.0.1
var api = "/api";

// Pool host name shown on the "Getting started" page
var poolHost = "conceal.pool-pay.com";

// A notice shown above every page (empty = none)
var poolNotice = {en: "<i class=\"fa fa-bolt\"></i> It's usually more convenient to mine through <a href=\"https://pool-pay.com/#sec-account\">pool-pay.com</a> &mdash; register once, get a personal port, and the miner automatically switches to whichever pool-pay.com coin is most profitable right now (payouts available in USDT). Example: <code>poolpayminer --auto YOUR_PORT</code>.", ru: "<i class=\"fa fa-bolt\"></i> Обычно удобнее майнить через <a href=\"https://pool-pay.com/#sec-account\">pool-pay.com</a> &mdash; зарегистрируйтесь один раз, получите личный порт, и майнер сам переключается на самую выгодную монету pool-pay.com прямо сейчас (выплаты доступны в USDT). Пример: <code>poolpayminer --auto ВАШ_ПОРТ</code>."};

// Contact / community links (leave empty to hide)
var email = "admin@pool-pay.com";
var telegram = "";
var discord = "https://discord.com/invite/Bqwz43R";
var github = "";      // pool software (menu item); conceal-nodejs-pool isn't published yet, see ccx-project memory
var minerDownload = "/downloads/";      // poolpayminer (optional menu item; leave empty to hide)
var allPoolsUrl = "https://pool-pay.com/";      // link to the page listing every pool-pay.com pool

// Only a tiny, thin market exists (Nonkyc.io CCX/USDT + CCX/BTC, ~$15-16/day total volume per CoinGecko
// on 2026-10-31) -- not liquid enough to show a meaningful price widget; market widgets hidden
var marketCurrencies = [];

// Block explorer links ({id} = block height / transaction id).
var blockchainExplorer = "https://explorer.conceal.network/?hash={id}#blockchain_block";
var blockExplorerId = "height";
var transactionExplorer = "https://explorer.conceal.network/?hash={id}#blockchain_transaction";

// Theme and default language ("en" or "ru")
var themeCss = "themes/default.css";
var defaultLang = "en";
