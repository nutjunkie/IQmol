#ifndef IQMOL_PASSWORDVAULT_H
#define IQMOL_PASSWORDVAULT_H
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

#include "ui_SetPasswordDialog.h"
#include "ui_GetVaultKeyDialog.h"
#include <QString>
#include <QDialog>


namespace IQmol {

class EnigmaMachine;

/// Singleton class
class PasswordVault {

   public:
      static PasswordVault& instance();

	  /// This prompts the user for a new Vault key.  This could either be
	  /// at the request of the user, or when first using a PasswordVault 
	  /// with an uninitialized key.
      void initializeVaultKey();

	  /// Unlocks the Vault in preparation for storing or retrieving 
	  /// passwords.  This ensures the Vault is in a valid state before 
	  /// doing any encryption/decryption.  This means either loading 
      /// any exisiting EnigmaMachine parameters, or prompting the user
      /// if none exist.  Returns true if the Vault is correctly unlocked.
      bool unlock();

	  /// Stores the given server password in the vault.  Note that the server
	  /// name does not acctually have to be the machine name.  Several Servers
	  /// can be set with different names that correspond to different accounts
	  /// on the same server.
      void storeServerPassword(QString const& server, QString const& password);

	  /// Returns the unencrypted password for the given server, 
	  /// or an empty string if unsuccessful.
      QString getServerPassword(QString const& server);

   private:
      PasswordVault() : m_enigmaMachine(0) { }
      explicit PasswordVault(PasswordVault const&);
      ~PasswordVault();

      void displayVaultContents();  // debug
      static void destroy() { delete s_instance; }

      EnigmaMachine* m_enigmaMachine;
      static PasswordVault* s_instance;
};



class SetPasswordDialog : public QDialog {

   Q_OBJECT 

   public:
	  /// Creates a dialog that prompts the user for a password and to verify
	  /// it.  The info is displayed at the top of the dialog
      SetPasswordDialog(QString const& info,
                        QString const& title = "Set Password", 
                        QString const& label1 = "Password", 
                        QString const& label2 = "Verify", QWidget* parent = 0);
      ~SetPasswordDialog();
      QString password() const { return m_dialog.password1->text(); }

   private Q_SLOTS:
      void verify();

   private:
      Ui::SetPasswordDialog m_dialog;
};


class GetVaultKeyDialog : public QDialog {

   Q_OBJECT 

   public:
	  /// Creates a dialog that prompts the user for a password.
	  /// The info is displayed at the top of the dialog
      GetVaultKeyDialog(unsigned int const seed, QString const& hash, QWidget* parent = 0);
      ~GetVaultKeyDialog();
      EnigmaMachine* enigmaMachine() const { return m_enigmaMachine; }

   private Q_SLOTS:
      void verify();

   private:
      Ui::GetVaultKeyDialog m_dialog;
      unsigned int m_seed;
      QString m_hash;
      EnigmaMachine* m_enigmaMachine;
};


void OverwriteString(QString&);
void OverwriteString(std::string&);

} // end namespace IQmol

#endif
