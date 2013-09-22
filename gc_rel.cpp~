#include"gc_rel.h"
using namespace std;
using namespace CryptoPP;
void HexEncode(const char *in, const char *out)//Hex编码
{
	FileSource(in, true, new HexEncoder(new FileSink(out)));
}

void HexDecode(const char *in, const char *out)//HEX解码
{
	FileSource(in, true, new HexDecoder(new FileSink(out)));
}
//****************************************
void AESEncryptFile(const char *in, const char *out, CryptoPP::ECB_Mode< AES >::Encryption ee)//加密文件

{
	
       FileSource  f(in, true, new StreamTransformationFilter(ee, new FileSink(out)));

}

void AESDecryptFile(const char *in, const char *out, CryptoPP::ECB_Mode< AES >::Decryption dd)//解密文件

{

       FileSource 

  f(in, true, new StreamTransformationFilter(dd, new FileSink(out)));      

}
//*****************************************
void InformationDisperseFile(int threshold, int nShares, const char *filename)// IDA算法加密分配文件
{
	assert(nShares<=1000);

	ChannelSwitch *channelSwitch;
	FileSource source(filename, false, new InformationDispersal(threshold, nShares, channelSwitch = new ChannelSwitch));

	vector_member_ptrs<FileSink> fileSinks(nShares);
	string channel;
	for (int i=0; i<nShares; i++)
	{
		char extension[5] = ".000";
		extension[1]='0'+byte(i/100);
		extension[2]='0'+byte((i/10)%10);
		extension[3]='0'+byte(i%10);
		fileSinks[i].reset(new FileSink((string(filename)+extension).c_str()));

		channel = WordToString<word32>(i);
		fileSinks[i]->Put((byte *)channel.data(), 4);
		channelSwitch->AddRoute(channel, *fileSinks[i], DEFAULT_CHANNEL);
	}

	source.PumpAll();
}

void InformationRecoverFile(int threshold, const char *outFilename, char *const *inFilenames)//IDA算法解密恢复文件(使用文件名）
{
	assert(threshold<=1000);

	InformationRecovery recovery(threshold, new FileSink(outFilename));

	vector_member_ptrs<FileSource> fileSources(threshold);
	SecByteBlock channel(4);
	int i;
	for (i=0; i<threshold; i++)
	{
		fileSources[i].reset(new FileSource(inFilenames[i], false));
		fileSources[i]->Pump(4);
		fileSources[i]->Get(channel, 4);
		fileSources[i]->Attach(new ChannelSwitch(recovery, string((char *)channel.begin(), 4)));
	}

	while (fileSources[0]->Pump(256))
		for (i=1; i<threshold; i++)
			fileSources[i]->Pump(256);

	for (i=0; i<threshold; i++)
		fileSources[i]->PumpAll();
}

void InformationRecoverFile(int threshold, const char *outFilename, const std::vector<const char * > &vectorinfile)//IDA算法解密恢复文件(使用存储文件名的vector）
{
	assert(threshold<=1000);

	InformationRecovery recovery(threshold, new FileSink(outFilename));

	vector_member_ptrs<FileSource> fileSources(threshold);
	SecByteBlock channel(4);
	int i;
	for (i=0; i<threshold; i++)
	{
		fileSources[i].reset(new FileSource(vectorinfile[i], false));
		fileSources[i]->Pump(4);
		fileSources[i]->Get(channel, 4);
		fileSources[i]->Attach(new ChannelSwitch(recovery, string((char *)channel.begin(), 4)));
	}

	while (fileSources[0]->Pump(256))
		for (i=1; i<threshold; i++)
			fileSources[i]->Pump(256);

	for (i=0; i<threshold; i++)
		fileSources[i]->PumpAll();
}

