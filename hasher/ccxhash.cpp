// ccxhash: share validation helper of the Conceal (CCX) pool.
// ccx-src/ is vendored verbatim from poolpayminer's own CN-GPU implementation (src/crypto/ccx_gpu/,
// itself vendored from ConcealNetwork/conceal-core's src/crypto/pow_hash/ + src/crypto/keccak.c, the
// Ryo Currency Project's cn_pow_hash_v3) -- see ccx-project.md. Using poolpayminer's own copy (rather
// than re-vendoring from the node a second time) keeps the pool's share check and the miner's actual
// PoW guaranteed identical by construction.
//
// Reads commands on stdin, writes answers on stdout (one line each):
//   H <id> <hex>   the exact 76-byte hashing blob (see blobtool.js -- HASHING, nonce already spliced
//                  in); answers "H <id> <64 hex>" (32 byte hash) or "H <id> err"
//   Q              quit
// Command line: --hash <hex> prints the hash.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include "ccx_gpu_hash.h"

static int hexval(int c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static int from_hex(const char *s, std::vector<unsigned char> &out)
{
	size_t n = strlen(s);
	if (n % 2) return -1;
	out.resize(n / 2);
	for (size_t i = 0; i < out.size(); i++) {
		int a = hexval(s[2 * i]), b = hexval(s[2 * i + 1]);
		if (a < 0 || b < 0) return -1;
		out[i] = (unsigned char)(a * 16 + b);
	}
	return 0;
}

static void to_hex(const unsigned char *in, size_t n, char *out)
{
	static const char d[] = "0123456789abcdef";
	for (size_t i = 0; i < n; i++) { out[2 * i] = d[in[i] >> 4]; out[2 * i + 1] = d[in[i] & 15]; }
	out[2 * n] = 0;
}

int main(int argc, char **argv)
{
	unsigned char hash[32];
	char hex[65];
	std::vector<unsigned char> data;

	if (!ccxGpuSelfTest()) {
		fprintf(stderr, "ccxhash: self-test FAILED, refusing to start\n");
		return 3;
	}

	if (argc == 3 && strcmp(argv[1], "--hash") == 0) {
		if (from_hex(argv[2], data)) { fprintf(stderr, "bad hex\n"); return 1; }
		ccxGpuHash(data.data(), data.size(), hash);
		to_hex(hash, 32, hex);
		puts(hex);
		return 0;
	}

	char line[8192];
	while (fgets(line, sizeof(line), stdin)) {
		if (line[0] == 'Q') break;
		if (line[0] != 'H') continue;
		char id[64], hexdata[8000];
		if (sscanf(line, "H %63s %7999s", id, hexdata) != 2 || from_hex(hexdata, data)) {
			printf("H %s err\n", (strlen(line) > 2) ? id : "0");
			fflush(stdout);
			continue;
		}
		ccxGpuHash(data.data(), data.size(), hash);
		to_hex(hash, 32, hex);
		printf("H %s %s\n", id, hex);
		fflush(stdout);
	}
	return 0;
}
