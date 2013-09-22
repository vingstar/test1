//g++ -g3 -ggdb -o0 -Wall -Wextra -Wno-unused -o gc1 gc1.cpp -lcryptopp -lpthread
#include"gc_rel.h"
#include<iostream>
#include<cstdlib>
#include<vector>
using namespace std;
#include "cryptopp/osrng.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/hex.h"
#include "cryptopp/filters.h"
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/files.h"
#include "cryptopp/ida.h"
using namespace CryptoPP;


int main(int argc, char* argv[])
{
	AutoSeededRandomPool prng;

  	int threshold = 2;
    int nshares = 6;
	string IDAin;
    string IDAkey("key_en");
	byte key[AES::DEFAULT_KEYLENGTH];
	
    prng.GenerateBlock(key, sizeof(key));
         
	string plain = "ECB Mode Test";
	string cipher, keyencoded, recovered;

	keyencoded.clear();
	StringSource(key, sizeof(key), true,
		new HexEncoder(
			new StringSink(keyencoded)
		) // HexEncoder
	); // StringSource
	cout << "key(hex): " << keyencoded << endl;


	try
	{
		cout << "plain text: " << plain << endl;

		ECB_Mode< AES >::Encryption e;
		e.SetKey(key, sizeof(key));
                
		// The StreamTransformationFilter adds padding
		//  as required. ECB and CBC Mode must be padded
		//  to the block size of the cipher.
		StringSource(plain, true, 
			new StreamTransformationFilter(e,
				new StringSink(cipher)
			) // StreamTransformationFilter      
		); // StringSource
      // FileSource  f("testin", true, new StreamTransformationFilter(e, new FileSink("testcipher")));
    AESEncryptFile("License.txt","testcipher1",e);
    HexEncode("testcipher1","testcipher_en16");
    IDAin="testcipher_en16";
    InformationDisperseFile(threshold, nshares,IDAin.data());
    #ifndef NDEBUG 
    cout<<"keyencoded: "<<keyencoded<<endl;
    #endif
   ofstream fkey(IDAkey.data());
   fkey<< keyencoded;
   fkey.close();
   //StringSource(keyencoded,true,
	//	new StreamTransformationFilter(e, new FileSink(IDAkey.data()))); // StringSource
	InformationDisperseFile(threshold, nshares,IDAkey.data());
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	/*********************************\
	\*********************************/

// THE ABOVE CODES ARE RUNNING IN THE INITIATOR
//
	// Pretty print
	keyencoded.clear();
	StringSource(cipher, true,
		new HexEncoder(
			new StringSink(keyencoded)
		) // HexEncoder
	); // StringSource
	cout << "cipher text: " << keyencoded << endl;

	/*********************************\
	\*********************************/

    // THE CODES BELOW SHOULD BE RUNNING BY THE OTHER SERVER!!!!
    //
	try
	{
		
 	    string keyrecover;
        string key_de;
        vector< const char * > clovefiles;
        string IDApf1=IDAin+".000";
        string IDApf2=IDAin+".001";
        clovefiles.push_back(IDApf1.data());
        clovefiles.push_back(IDApf2.data());
        vector< const char *> clovekey;
        string IDAkeyp1=IDAkey+".000";
        string IDAkeyp2=IDAkey+".001";
        clovekey.push_back(IDAkeyp1.data());
	    clovekey.push_back(IDAkeyp2.data());
    	InformationRecoverFile(threshold, "IDArecovertest_en16",clovefiles);
 	    InformationRecoverFile(threshold, "IDArekeytest_en16",clovekey);
        FileSource("IDArekeytest_en16", true, new StringSink(keyrecover));
        cout<<"recover key:"<<keyrecover<<endl;
        HexDecode("IDArecovertest_en16","IDArecovertest_de16");
        StringSource(keyrecover, true, new HexDecoder(new StringSink(key_de)));
        cout<<"size of recover key_de: "<<key_de.size()<<endl;
	    ECB_Mode< AES >::Decryption d;
        d.SetKey((const byte *)key_de.data(), key_de.size());
		// The StreamTransformationFilter removes
		//  padding as required.
		StringSource s(cipher, true, 
			new StreamTransformationFilter(d,
				new StringSink(recovered)
			) // StreamTransformationFilter
		); // StringSource
                
		cout << "recovered text: " << recovered << endl; 
        HexDecode("testcipher_en16","testcipher_de16");
		AESDecryptFile("testcipher_de16","testout1",d);
	AESDecryptFile("IDArecovertest_de16","IDAtestout1",d);
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	/*********************************\
	\*********************************/

	return 0;
}


