#ifndef IQMOL_ENIGMAMACHINE_H
#define IQMOL_ENIGMAMACHINE_H
/*******************************************************************************
       
  Copyright (C) 2011-2013 Andrew Gilbert
           
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

#include <string>
#include <openssl/evp.h>
#include <openssl/aes.h>


namespace IQmol {

typedef unsigned char BinaryData;

/// Class that encrypts/decrypts strings using AES algorithms.  
/// Must add -lcrypto to the link command
class EnigmaMachine {

   public:
      /// The seed is unsed to generate 64 bits of random salt material.
      /// (This assumes sizeof(int) >= 4).  The seed should be stored and 
      /// used to initialize any subsequent EnigmaMachine that is to be 
      /// used to decrypt the data.
      EnigmaMachine(std::string const& password, unsigned int seed = 0);
      ~EnigmaMachine();

	  /// Encrypts an ASCII string and returns the encrypted string in a hex
	  /// encoded representation.  Note that this is probably not very efficient
	  /// and is only designed for small amounts of text.
      std::string encrypt(std::string const&);

      /// Takes a hex-encoded string representing binary data and decrypts it
      std::string decrypt(std::string const&);

	  /// Returns the seed used to generate the salt for the encryption.  This
	  /// needs to be stored if the data is to be decrypted later.
	  unsigned int seed() const { return m_seed; }
      bool initialized() const { return m_initialized; }

      /// Returns a hash value for the given string.
      std::string mdHash(std::string const&);
      

   private:
	  // Returns a null-terminated char* with eight binary digets representing
	  // the 8-bit char input (mainly for debugging). 
      char* charToBin(unsigned char);

	  // Returns a null-terminated char* with two hex digits representing the
	  // 8-bit char input.  Note that this assumes 8-bit chars (which is
	  // reasonable).
      char* charToHex(unsigned char c);

      /// Converts two hex digits into a raw binary representation.
      unsigned char hexToChar(char const c1, char const c2);

	  /// Create an 256 bit key and IV using the supplied password. Salt can be
	  /// added for taste. Fills in the encryption and decryption context
	  /// objects and returns true on success.
      bool init(BinaryData* password, int key_data_len, BinaryData* salt);

      EVP_CIPHER_CTX m_encryptContext, m_decryptContext;
      unsigned int m_seed;
      bool m_initialized;
};


} // end namespace IQmol

#endif
