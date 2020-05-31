#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "default.h"

using namespace std;
using namespace CryptoPP;

// https://stackoverflow.com/questions/3381614/c-convert-string-to-hexadecimal-and-vice-versa
string strToHex(string str)
{
    stringstream ss;
    ss << hex << setfill('0');

    for(auto ch : str) {
        // setw(2) ռ��λ��setfill('0') ��λ��� 0
        ss << setw(2) << (int)(unsigned char)ch;
    }

    return ss.str();
}

// https://stackoverflow.com/questions/11608878/c-convert-string-to-hex
string hexToStr(string hex)
{
    string str;

    for(string::size_type i = 0; i < hex.length(); i += 2) {
        string tempStr = hex.substr(i, 2);
        // string to int
        unsigned char ch = (unsigned char)stoi(tempStr, nullptr, 16);
        str.push_back(ch);
    }

    return str;
}

// ���
string padding(string plaintext)
{
    string lastBlock;
    int len = plaintext.length();
    int paddingNum = AES::BLOCKSIZE - len % AES::BLOCKSIZE;
    int quotient = len / AES::BLOCKSIZE;

    lastBlock = plaintext.substr(AES::BLOCKSIZE * quotient, len % AES::BLOCKSIZE);
    for(int i = 0; i < AES::BLOCKSIZE - len % AES::BLOCKSIZE; i++) {
        lastBlock.push_back((unsigned char)paddingNum);
    }

    return plaintext.substr(0, AES::BLOCKSIZE * quotient) + lastBlock;
}

string encrypt(string plaintext, string key, string vi, string ciphertext)
{
    plaintext = padding(plaintext);
    ciphertext += vi;
    key = hexToStr(key);
    vi = hexToStr(vi);
    int multiple = plaintext.length() / AES::BLOCKSIZE;

    AESEncryption aesEncryptor;
    aesEncryptor.SetKey((byte*)key.c_str(), key.length());

    for(int i = 0; i < multiple; i++) {
        string plaintextBlock = plaintext.substr(i * AES::BLOCKSIZE, AES::BLOCKSIZE);
        string xorBlock;
        unsigned char outBlock[AES::BLOCKSIZE];
        memset(outBlock, 0, AES::BLOCKSIZE);

        for(int j = 0; j < AES::BLOCKSIZE; j++) {
            xorBlock.push_back(plaintextBlock[j] ^ (unsigned char)vi[j]);
        }

        aesEncryptor.ProcessBlock((byte*)xorBlock.c_str(), outBlock);
        vi = "";

        // unsigned char[] ת string ��Ҫͨ�������ʽ��vi = (char *) outBlock
        for(int j = 0; j < AES::BLOCKSIZE; j++) {
            vi.push_back(outBlock[j]);
        }
        vi = vi.substr(0, AES::BLOCKSIZE);
        ciphertext += strToHex(vi);
    }

    return ciphertext;
}

string decrypt(string ciphertext, string key, string plaintext)
{
    // ԭʼ key Ϊ 16 ������ʽ���谴�ֽ�ת��Ϊ char
    key = hexToStr(key);
    ciphertext = hexToStr(ciphertext);
    // ���ĵ�ǰ 16 ���ֽ�Ϊ vi
    string vi = ciphertext.substr(0, AES::BLOCKSIZE);
    ciphertext = ciphertext.substr(AES::BLOCKSIZE, ciphertext.length() - AES::BLOCKSIZE);
    int multiple = ciphertext.length() / AES::BLOCKSIZE;

    AESDecryption aesDecryptor;
    aesDecryptor.SetKey((byte*)key.c_str(), key.length());

    for(int i = 0; i < multiple; i++) {
        // ��������
        string ciphertextBlock = ciphertext.substr(i * AES::BLOCKSIZE, AES::BLOCKSIZE);
        unsigned char outBlock[AES::BLOCKSIZE];
        memset(outBlock, 0, AES::BLOCKSIZE);

        aesDecryptor.ProcessBlock((byte*)ciphertextBlock.c_str(), outBlock);

        // AES ���������������Ļ� vi ��򣬵õ�����
        for(int j = 0; j < AES::BLOCKSIZE; j++) {
            plaintext.push_back(outBlock[j] ^ (unsigned char)vi[j]);
        }

        vi = ciphertextBlock;
    }

    // ���ܺ����һ�����ĵ�������
    string lastBlock = plaintext.substr((multiple - 1) * AES::BLOCKSIZE, AES::BLOCKSIZE);
    // ���ַ������һ���ַ���ȡ����ַ�
    int paddingNum = (unsigned char)lastBlock[AES::BLOCKSIZE - 1];

    // ������ַ���������ȥ��
    for(int i = 0; i < paddingNum; i++) {
        // ������ַ����ֲ�ͬ����˵��������������
        if(plaintext.back() != paddingNum) {
            return "Ciphertext is invalid!";
        }
        plaintext.pop_back();
    }

    return plaintext;
}

int main(int argc, char** argv)
{
    string plaintext = "";
    string ciphertext = "5b68629feb8606f9a6667670b75b38a5b4832d0f26e1ab7da33249de7d4afc48e713ac646ace36e872ad5fb8a51242"
                        "8a6e21364b0c374df45503473c5242a253";
    string key = "140b41b22a29beb4061bda66b6747e14";
    string vi = "";

    plaintext = decrypt(ciphertext, key, plaintext);

    cout << "plaintext: " << plaintext << endl;

    return 0;
}
