#include <iostream>
#include <cstring>
#include <string>
#include "Common/StringTools.h"
#include "Common/MemoryInputStream.h"
#include "Serialization/BinaryInputStreamSerializer.h"
#include "CryptoNoteCore/CryptoNoteTools.h"
#include "CryptoNoteCore/CryptoNoteFormatUtils.h"
#include "CryptoNoteCore/CryptoNoteBasic.h"
#include "CryptoNoteCore/CryptoNoteSerialization.h"

using namespace cn;
using namespace common;

static bool parseBlock(const std::string &blobHex, Block &b) {
	BinaryArray ba;
	if (!fromHex(blobHex, ba)) { std::cerr << "bad hex blob\n"; return false; }
	try {
		MemoryInputStream stream(ba.data(), ba.size());
		BinaryInputStreamSerializer serializer(stream);
		serialize(b, serializer);
	} catch (std::exception &e) {
		std::cerr << "EXCEPTION: " << e.what() << "\n";
		return false;
	}
	return true;
}

/* pool.js's blobtool.js calls this once per new block template (not per share) and expects, on
 * stdout, one line each of TEMPLATE/HASHING/TEMPLATE_NONCE_OFFSET/HASHING_NONCE_OFFSET/PREVHASH --
 * see kryptokrona-nodejs-pool's lib/blobtool.js header comment for the established contract this
 * mirrors. Conceal's getblocktemplate blob needs NO merge-mining-tag rewrite (constructMinerTx has
 * no such step), so TEMPLATE/HASHING here are just the original blob's own encodings -- only the
 * nonce offsets need discovering, done empirically with a marker value per the project's own
 * established convention (tracing the serializer's varint-timestamp field by hand is error-prone).
 */
static int runFull(const std::string &blobHex) {
	Block b = boost::value_initialized<Block>();
	if (!parseBlock(blobHex, b)) return 5;

	BinaryArray origTemplateBa;
	if (!fromHex(blobHex, origTemplateBa)) { std::cerr << "bad hex blob\n"; return 2; }

	BinaryArray origHashingBa;
	if (!get_block_hashing_blob(b, origHashingBa)) { std::cerr << "get_block_hashing_blob failed\n"; return 4; }

	const uint32_t marker = 0xAABBCCDDu;
	Block bMarked = b;
	bMarked.nonce = marker;

	BinaryArray markedTemplateBa = toBinaryArray(bMarked);
	BinaryArray markedHashingBa;
	if (!get_block_hashing_blob(bMarked, markedHashingBa)) { std::cerr << "get_block_hashing_blob (marked) failed\n"; return 4; }

	std::string markedTemplateHex = toHex(markedTemplateBa);
	std::string markedHashingHex = toHex(markedHashingBa);

	char markerHex[9];
	snprintf(markerHex, sizeof(markerHex), "%02x%02x%02x%02x",
	         marker & 0xff, (marker >> 8) & 0xff, (marker >> 16) & 0xff, (marker >> 24) & 0xff);

	size_t tOff = markedTemplateHex.find(markerHex);
	size_t hOff = markedHashingHex.find(markerHex);
	if (tOff == std::string::npos || hOff == std::string::npos) {
		std::cerr << "marker not found (tOff=" << (long)tOff << " hOff=" << (long)hOff << ")\n";
		return 6;
	}

	std::cout << "TEMPLATE " << toHex(origTemplateBa) << "\n";
	std::cout << "HASHING " << toHex(origHashingBa) << "\n";
	std::cout << "TEMPLATE_NONCE_OFFSET " << (tOff / 2) << "\n";
	std::cout << "HASHING_NONCE_OFFSET " << (hOff / 2) << "\n";
	std::cout << "PREVHASH " << podToHex(b.previousBlockHash) << "\n";
	return 0;
}

int main(int argc, char **argv) {
	if (argc < 2) { std::cerr << "usage: ccx_blobtool <blobHex> | ccx_blobtool hash|submit <blobHex> [nonceHex]\n"; return 1; }

	/* called with just the blob hex (pool.js's own calling convention) -> full mode */
	if (argc == 2) {
		return runFull(argv[1]);
	}

	std::string mode = argv[1];
	std::string blobHex = argv[2];

	Block b = boost::value_initialized<Block>();
	if (!parseBlock(blobHex, b)) return 5;

	if (mode == "full") return runFull(blobHex);

	if (mode == "hash") {
		BinaryArray hashingBlob;
		if (!get_block_hashing_blob(b, hashingBlob)) { std::cerr << "get_block_hashing_blob failed\n"; return 4; }
		std::cout << toHex(hashingBlob) << std::endl;
		return 0;
	}
	if (mode == "submit") {
		if (argc < 4) { std::cerr << "submit needs nonce\n"; return 1; }
		BinaryArray nonceBytes;
		if (!fromHex(argv[3], nonceBytes) || nonceBytes.size() != 4) { std::cerr << "bad nonce\n"; return 2; }
		uint32_t nonce = 0; memcpy(&nonce, nonceBytes.data(), 4);
		b.nonce = nonce;
		std::cout << toHex(toBinaryArray(b)) << std::endl;
		return 0;
	}
	std::cerr << "unknown mode\n"; return 1;
}
