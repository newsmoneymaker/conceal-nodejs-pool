/**
 * Conceal (CCX) Pool (based on cryptonote-nodejs-pool, GPL-2.0)
 *
 * Wrapper around the real node's own block-template helper (ccx_blobtool, linked against the node's own
 * compiled CryptoNoteCore -- see ccx-project.md). Unlike some other CryptoNote forks (e.g. Kryptokrona),
 * Conceal's reference miner does NOT rewrite the coinbase transaction's `extra` field with a merge-mining
 * tag before computing the PoW hash (confirmed directly against src/CryptoNoteCore/Currency.cpp) -- so
 * `ccx_blobtool`'s "full" mode just re-derives the hashing blob's nonce offset from the template's own
 * encoding, no mutation needed. Still delegated to the node's own compiled code (via SSH to the node
 * host) rather than hand-rolled in JS, for the same reason: real consensus code backing it is safer than
 * re-deriving block-template internals independently. Runs once per new block template, not per share.
 *
 * run(blobHex, cb)  cb(err, {template, hashing, templateNonceOffset, hashingNonceOffset})
 **/
let execFile = require('child_process').execFile;

let toolConfig = config.blobtool || {};
let sshHost = toolConfig.sshHost || 'srv3';
let remotePath = toolConfig.remotePath || '/root/conceal-core-v8/build/src/ccx_blobtool';

exports.run = function (blobHex, callback) {
	execFile('ssh', [sshHost, remotePath, blobHex], {timeout: 10000, maxBuffer: 1024 * 1024}, function (err, stdout, stderr) {
		if (err) return callback(new Error('blobtool failed: ' + (stderr || err.message).trim()));

		let result = {};
		stdout.split('\n').forEach(function (line) {
			let parts = line.trim().split(' ');
			if (parts[0] === 'TEMPLATE') result.template = parts[1];
			else if (parts[0] === 'HASHING') result.hashing = parts[1];
			else if (parts[0] === 'TEMPLATE_NONCE_OFFSET') result.templateNonceOffset = parseInt(parts[1], 10);
			else if (parts[0] === 'HASHING_NONCE_OFFSET') result.hashingNonceOffset = parseInt(parts[1], 10);
			else if (parts[0] === 'PREVHASH') result.prevHash = parts[1];
		});

		if (!result.template || !result.hashing || !(result.templateNonceOffset >= 0) || !(result.hashingNonceOffset >= 0)) {
			return callback(new Error('blobtool gave an incomplete answer: ' + stdout.trim()));
		}

		callback(null, result);
	});
};
