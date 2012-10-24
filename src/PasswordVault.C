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

#include "PasswordVault.h"
#include "EnigmaMachine.h"
#include "Preferences.h"
#include "QMsgBox.h"
#include "QsLog.h"
#include <QDebug>
#include <QInputDialog>


namespace IQmol {

/// Used to securely overwrite the contents of a string containing 
/// sensitive data. This ensures the data does not persist in memory 
/// after going out of scope.
void OverwriteString(std::string& string)
{
   string.replace(0, string.size(), string.size(), '0');
   string.clear();
}
void OverwriteString(QString& string)
{
   QString zero(QString('0').repeated(string.length()));
   string.replace(0, string.size(), zero);
   string.clear();
}


PasswordVault* PasswordVault::s_instance = 0;

PasswordVault& PasswordVault::instance() {
   if (s_instance == 0) {
      s_instance = new PasswordVault();
#ifdef Q_WS_X11
      atexit(PasswordVault::destroy);
#else
      ::std::atexit(PasswordVault::destroy);
#endif
   }
   return *s_instance;
}


PasswordVault::~PasswordVault() { 
   delete m_enigmaMachine;
}


bool PasswordVault::unlock()
{
   if (!m_enigmaMachine) {
      // Attempt to recreate the last EnigmaMachine
      unsigned int seed(Preferences::PasswordVaultSeed());

      if (seed == 0) {
         // The vault is uninitialized, grab a new key from the user
         // (this initializes the enigmaMachine)
         QLOG_INFO() << "Generating new vault key";
         initializeVaultKey();
      }else {
         GetVaultKeyDialog dialog(seed, Preferences::PasswordVaultKey());
         if (dialog.exec() == QDialog::Accepted) {
            m_enigmaMachine = dialog.enigmaMachine();
         }
      }
   }

   return (m_enigmaMachine != 0);
}


void PasswordVault::displayVaultContents()
{
   QMap<QString,QString> vault(Preferences::PasswordVaultContents());
   qDebug() << "Printing the contents of the Password Vault";

   QMap<QString,QString>::iterator iter;
   for (iter = vault.begin(); iter != vault.end(); ++iter) {
       qDebug() << iter.key() << " -> " << iter.value();
   }
   
}


QString PasswordVault::getServerPassword(QString const& server)
{
   QString password;
   if (unlock()) {
      QMap<QString, QString> vault(Preferences::PasswordVaultContents());
      password = vault.value(server);
      if (!password.isEmpty()) {
         std::string str(m_enigmaMachine->decrypt(password.toStdString()));
         password = QString::fromStdString(str);
         OverwriteString(str);
      }
      displayVaultContents();
   }else {
      QLOG_DEBUG() << "EnigmaMachine uninitialized";
   }
   return password;
}


void PasswordVault::storeServerPassword(QString const& server, QString const& password)
{
   if (unlock()) {
      std::string str(m_enigmaMachine->encrypt(password.toStdString()));
      QMap<QString, QString> vault(Preferences::PasswordVaultContents());
      vault.insert(server, QString::fromStdString(str));
      Preferences::PasswordVaultContents(vault);
      displayVaultContents();
   }else {
      QLOG_DEBUG() << "EnigmaMachine uninitialized";
   }
}


void PasswordVault::initializeVaultKey()
{
   QString info("Setting the key for the Password Vault.\n"
                "Note: If you have server passwords saved in the Vault,\n"
                "these will no longer be valid and you will need to \n"
                "re-enter them in order to encrypt them with the new \n"
                "Vault Key.\n\n"
                "The Password Vault key is used to encrypt server\n"
                "passwords before being stored.  The key is not\n"
                "stored on file and must therefore be entered each\n"
                "time you restart IQmol.  If you forget your vault\n" 
                "key, you must reconfigure all the server settings\n"
                "saved in IQmol\n");

   SetPasswordDialog dialog(info, "Set Password Vault Key", "Vault Key" ); 

   if (dialog.exec() == QDialog::Accepted) {
      QString key(dialog.password());
      QLOG_INFO() << "Vault key accepted";

      delete m_enigmaMachine;
      m_enigmaMachine = new EnigmaMachine(key.toStdString());
      std::string hash(m_enigmaMachine->mdHash(key.toStdString()));
      OverwriteString(key);

      Preferences::ClearPasswordVaultContents();
      Preferences::PasswordVaultKey(QString::fromStdString(hash));
      Preferences::PasswordVaultSeed(m_enigmaMachine->seed());
   }
}



SetPasswordDialog::SetPasswordDialog(QString const& info, QString const& title,
   QString const& label1, QString const& label2,  QWidget* parent) : QDialog(parent)
{
   setWindowTitle(title);
   m_dialog.setupUi(this);
   m_dialog.infoLabel->setText(info);
   m_dialog.label1->setText(label1);
   m_dialog.label2->setText(label2);

   connect(m_dialog.buttonBox, SIGNAL(accepted()), this, SLOT(verify()));
}


SetPasswordDialog::~SetPasswordDialog()
{
   QString text;
   text = m_dialog.password1->text();
   OverwriteString(text);
   m_dialog.password1->setText(text);
   text = m_dialog.password2->text();
   OverwriteString(text);
   m_dialog.password2->setText(text);
}


void SetPasswordDialog::verify()
{
   if (m_dialog.password1->text() == m_dialog.password2->text()) {
      if (m_dialog.password1->text().size() < 6) {
         QMsgBox::warning(0, "IQmol", 
            "A key at least 6 characters long is recommended");
      }else {
         accept();
      }
   }else {
      QMsgBox::warning(0, "IQmol", "Keys do not match");
   }
}


GetVaultKeyDialog::GetVaultKeyDialog(unsigned int const seed, QString const& hash,
   QWidget* parent) : QDialog(parent), m_seed(seed), m_hash(hash), m_enigmaMachine(0)
{
   m_dialog.setupUi(this);
   connect(m_dialog.buttonBox, SIGNAL(accepted()), this, SLOT(verify()));
}


GetVaultKeyDialog::~GetVaultKeyDialog()
{
   QString text;
   text = m_dialog.password->text();
   OverwriteString(text);
   m_dialog.password->setText(text);
}


void GetVaultKeyDialog::verify()
{
   std::string key(m_dialog.password->text().toStdString());
   m_enigmaMachine = new EnigmaMachine(key, m_seed);

   QString hash(QString::fromStdString(m_enigmaMachine->mdHash(key)));
   OverwriteString(key);

   QLOG_DEBUG() << "Existing Vault Key" << m_hash;
   QLOG_DEBUG() << "Entered  Vault Key" << hash;

   if (hash == m_hash) {
      accept();
      QMsgBox::information(0, "IQmol", "Password Vault unlocked");
   }else {
      delete m_enigmaMachine;
      m_enigmaMachine = 0;
      QMsgBox::warning(0, "IQmol", "Passwords do not match");
   }
}


} // end namespace IQmol
