/*******************************************************************************
       
  Copyright (C) 2011 Andrew Gilbert
           
  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.
       
  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.
      
  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.  
   
********************************************************************************/

#include "EnigmaMachine.h"
#include <iostream>
#include <ctime>
#include <climits>
#include <stdio.h>


namespace IQmol {

EnigmaMachine::EnigmaMachine(std::string const& password, unsigned int seed) : m_seed(seed) 
{
   // The seed is unsed to generate 64 bits of random salt material.
   // (This assumes sizeof(int) >= 4).  The seed should be stored and 
   // used to initialize any subsequent EnigmaMachine that is to be 
   // used to decrypt the data.
   if (m_seed == 0) m_seed = std::time(0);
#ifdef Q_WS_X11
   srand(m_seed);
   unsigned int salt[] = { rand(), rand() };
#else
   std::srand(m_seed);
   unsigned int salt[] = { std::rand(), std::rand() };
#endif
   const char* cstr(password.c_str());
   m_initialized = init((BinaryData*)cstr, strlen(cstr), (BinaryData*)salt);
}


EnigmaMachine::~EnigmaMachine()
{
  // Remove any sensitive information from memory.  Note the 
  // EnigmaMachine object does not explicitly store any data 
  // that may be sensitive is all handed over to the contexts.
  EVP_CIPHER_CTX_cleanup(&m_encryptContext);
  EVP_CIPHER_CTX_cleanup(&m_decryptContext);
}


bool EnigmaMachine::init(BinaryData* password, int passwordLength, BinaryData* salt)
{
   int nrounds(5);
   BinaryData key[32], iv[32];
  
   // Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the
   // supplied key material.  nrounds is the number of times the we hash the
   // material. More rounds are more secure but slower.
   int keySize = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt, password, 
          passwordLength, nrounds, key, iv);
           
   if (keySize != 32) return false;

   EVP_CIPHER_CTX_init(&m_encryptContext);
   EVP_EncryptInit_ex(&m_encryptContext, EVP_aes_256_cbc(), NULL, key, iv);
   EVP_CIPHER_CTX_init(&m_decryptContext);
   EVP_DecryptInit_ex(&m_decryptContext, EVP_aes_256_cbc(), NULL, key, iv);
 
   return true;
}


char* EnigmaMachine::charToBin (unsigned char c)
{
    static char bin[CHAR_BIT+1] = {0};
    for (int i = CHAR_BIT-1; i >= 0; --i) {
        bin[i] = (c % 2) + '0';
        c /= 2;
    }
    return bin;
}


char* EnigmaMachine::charToHex(unsigned char c)
{
   // This assumes 8-bit chars which can be represented in 2 hex digits
   static const char* key = "0123456789abcdef";
   static char hex[3] = {0};
   hex[0] = key[c / 16];
   hex[1] = key[c % 16];
   return hex;
}


unsigned char EnigmaMachine::hexToChar(char const c1, char const c2)
{
   unsigned char h1(0);

   if (c1 >= '0' && c1 <= '9') {
      h1 = c1 - '0'; 
   }else if (c1 >= 'a' && c1 <= 'f') {
      h1 = c1 - 'a' + 10; 
   }else if (c1 >= 'A' && c1 <= 'F') {
      h1 = c1 - 'A' + 10; 
   }

   unsigned char h2(0);

   if (c2 >= '0' && c2 <= '9') {
      h2 = c2 - '0'; 
   }else if (c2 >= 'a' && c2 <= 'f') {
      h2 = c2 - 'a' + 10; 
   }else if (c2 >= 'A' && c2 <= 'F') {
      h2 = c2 - 'A' + 10; 
   }
    
   return 16*h1 + h2;
}


std::string EnigmaMachine::encrypt(std::string const& text)
{
  int length(text.size()+1); // we include the trailing null character
  // The maximum cipher text length for n bytes of plane text is n + AES_BLOCK_SIZE bytes 
  int cipherTextLength = length + AES_BLOCK_SIZE;
  int finishLength = 0;
  const char* plainText(text.c_str());

/*
  std::cout << "String to encrypt: " << text << std::endl;
  std::cout << "String length: " << length << " (inc. \\0)" << std::endl;
  std::cout << "Seed " << m_seed << std::endl;
  std::cout << "Maximum cipher text length: " << cipherTextLength << std::endl;
   

  for (int i = 0;  i < length; ++i) {
      std::cout << plainText[i] << "  " 
                << charToBin(plainText[i]) << " - " 
                << charToHex(plainText[i]) 
                << std::endl;
  }
*/

  BinaryData* cipherText = new BinaryData[cipherTextLength];

  // allows reusing of m_encryptContext for multiple encryption cycles
  EVP_EncryptInit_ex(&m_encryptContext, NULL, NULL, NULL, NULL);

  // Update ciphertext, cipherTextLength is filled with the length of
  // ciphertext generated, length is the size of plaintext in bytes 
  EVP_EncryptUpdate(&m_encryptContext, cipherText, &cipherTextLength, 
     (BinaryData const*)plainText, length);

  /* update ciphertext with the final remaining bytes */
  EVP_EncryptFinal_ex(&m_encryptContext, cipherText+cipherTextLength, &finishLength);

  length = cipherTextLength + finishLength;

  // convert cipherText to hex encoded data
  // std::cout << "Actual  cipher text length: " << length << std::endl;
  std::string cipher;
  for (int i = 0;  i < length; ++i) {
      cipher += charToHex(cipherText[i]);
  }
  std::cout << cipher << std::endl << std::endl;

  delete[] cipherText;
  return  cipher;
}


std::string EnigmaMachine::decrypt(std::string const& cipherString)
{
   // Each pair of cipher characters corresponds to a single char
   int length(cipherString.size()/2);
   std::cout << "Cipher string length: " << cipherString.size() << std::endl;
   std::cout << "Allocation for text:  " << length << std::endl;

   // plaintext will always be equal to or lesser than length of ciphertext
   BinaryData* plainText  = new BinaryData[length];
   BinaryData* cipherText = new BinaryData[length];

   // Fill the cipherText array with the raw data
   std::cout <<  "Binary string:";
   for (int i = 0; i < length; ++i) {
       cipherText[i] = hexToChar(cipherString[2*i], cipherString[2*i+1]);
       std::cout << " " << charToBin(cipherText[i]) ;
   }
   std::cout <<  std::endl;

   int pLength(length), fLength(0);
  
   EVP_DecryptInit_ex(&m_decryptContext, NULL, NULL, NULL, NULL);
   EVP_DecryptUpdate(&m_decryptContext, plainText, &pLength, cipherText, length);
   EVP_DecryptFinal_ex(&m_decryptContext, plainText+pLength, &fLength);

   length = pLength + fLength;
   std::string ret((char*)plainText);

   std::cout << "Decoded string length: " << length << std::endl;
   std::cout << "Decoded string: " << ret << std::endl;

   delete[] cipherText;
   delete[] plainText;
   return ret;
}



std::string EnigmaMachine::mdHash(std::string const& input)
{
   const EVP_MD* md(EVP_sha1());
   EVP_MD_CTX* mdContext(EVP_MD_CTX_create());
   EVP_DigestInit_ex(mdContext, md, NULL);

   // The input is hashed twice, with the seed used as salt 
   // for good measure.
   EVP_DigestUpdate(mdContext, input.c_str(), input.size());
   EVP_DigestUpdate(mdContext, &m_seed, sizeof(unsigned int));
   EVP_DigestUpdate(mdContext, input.c_str(), input.size());

   unsigned int mdLength;
   BinaryData mdValue[EVP_MAX_MD_SIZE];
   EVP_DigestFinal_ex(mdContext, mdValue, &mdLength);
   EVP_MD_CTX_destroy(mdContext);

   std::string hash;
   for (unsigned int i = 0; i < mdLength; ++i)  hash += charToHex(mdValue[i]);
   return hash;
}


/*
int main(int argc, char **argv)
{
  std::string passPhrase("onestring");
  std::string passWord("pass word#$%^58");

  EnigmaMachine enigma(passPhrase);
  unsigned int seed(enigma.seed());
  std::string encrypted(enigma.encrypt(passWord));
  std::string result(enigma.decrypt(encrypted));

  if (result == passWord) {
     std::cout << "Success!" << std::endl;
  }else {
     std::cout << "Uh oh!" << std::endl;
  }

  std::cout << "Please enter the passphrase:";
  std::string pp;
  std::cin >> pp;
  std::cout << "|" << pp <<"|" << std::endl;

  EnigmaMachine enigma2(pp, seed);
  enigma2.decrypt(encrypted);
  return 0;
}
*/

} // end namespace IQmol
