#ifndef __GC_REL_H
#define __GC_REL_H

#include<iostream>
#include<cstdlib>
#include<vector>
#include "cryptopp/osrng.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/hex.h"
#include "cryptopp/filters.h"
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/files.h"
#include "cryptopp/ida.h"

using namespace CryptoPP;

void HexEncode(const char *infile, const char *outfile);
void HexDecode(const char *infile, const char *outfile);

void InformationDisperseFile(int threshold, int nShares, const char *filename);
void InformationRecoverFile(int threshold, const char *outFilename, char *const *inFilenames);

void InformationRecoverFile(int threshold, const char *outFilename, const std::vector< const char * > &vectorinfile);

void AESEncryptFile(const char *in, const char *out, CryptoPP::ECB_Mode< AES >::Encryption ee);
void AESDecryptFile(const char *in, const char *out, CryptoPP::ECB_Mode< AES >::Decryption dd);
#endif
