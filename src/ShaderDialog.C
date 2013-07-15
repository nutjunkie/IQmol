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

#include "ShaderDialog.h"
#include "ShaderLibrary.h"
#include "Preferences.h"
#include "QMsgBox.h"

#include <QDebug>


namespace IQmol {

ShaderDialog::ShaderDialog(QWidget* parent) : QDialog(parent)
{
   m_dialog.setupUi(this);
   m_labels[0]  = m_dialog.label0;
   m_labels[1]  = m_dialog.label1;
   m_labels[2]  = m_dialog.label2;
   m_labels[3]  = m_dialog.label3;
   m_sliders[0] = m_dialog.slider0;
   m_sliders[1] = m_dialog.slider1;
   m_sliders[2] = m_dialog.slider2;
   m_sliders[3] = m_dialog.slider3;

   m_checkBoxes[0] = m_dialog.checkBox0;
   m_checkBoxes[1] = m_dialog.checkBox1;
   m_checkBoxes[2] = m_dialog.checkBox2;
   m_checkBoxes[3] = m_dialog.checkBox3;

   for (int i = 0; i < s_maxSliders; ++i) {
       connect(m_sliders[i], SIGNAL(valueChanged(int)), 
          this, SLOT(installShaderParameters(int)));
   }
   for (int i = 0; i < s_maxCheckBoxes; ++i) {
       connect(m_checkBoxes[i], SIGNAL(stateChanged(int)), 
          this, SLOT(installShaderParameters(int)));
   }

   ShaderLibrary& library(ShaderLibrary::instance());
   QStringList shaderNames(library.availableShaders());
   m_dialog.shaderCombo->addItems(shaderNames);

   hideOptionControls();
   int index(m_dialog.shaderCombo->findText(Preferences::DefaultShader()));
   if (index >= 0) {
      m_dialog.shaderCombo->setCurrentIndex(index);
      on_shaderCombo_currentIndexChanged(index);
   }

   if (!library.filtersAvailable()) {
      m_dialog.shaderFilterTabWidget->removeTab(1);
      return;
   }

   connect(m_dialog.antialias, SIGNAL(clicked(bool)), 
      this, SLOT(installFilterParameters(bool)));
   connect(m_dialog.border, SIGNAL(clicked(bool)), 
      this, SLOT(installFilterParameters(bool)));
   connect(m_dialog.ambientOcclusion, SIGNAL(clicked(bool)), 
      this, SLOT(installFilterParameters(bool)));

   connect(m_dialog.aoRadius, SIGNAL(valueChanged(int)), 
      this, SLOT(installFilterParameters(int)));
   connect(m_dialog.aoStrength, SIGNAL(valueChanged(int)), 
      this, SLOT(installFilterParameters(int)));
   connect(m_dialog.aoStrength, SIGNAL(valueChanged(int)), 
      this, SLOT(installFilterParameters(int)));

   setFilterParameters(Preferences::DefaultFilterParameters());

   // Hide the effects page for now
   m_dialog.shaderFilterTabWidget->removeTab(1);
}


void ShaderDialog::on_shaderCombo_currentIndexChanged(int)
{
   ShaderLibrary& library(ShaderLibrary::instance());
   QString name(m_dialog.shaderCombo->currentText());

   if (!library.bindShader(name)) {
      QMsgBox::warning(this, "IQmol", "Shader not found");
      return;
   }

   setParameters(library.uniformUserVariableList(name));
   updated();
}


void ShaderDialog::hideOptionControls()
{
   for (int i = 0; i < s_maxSliders; ++i) {
       m_labels[i]->hide();
       m_sliders[i]->hide();
   }
   for (int i = 0; i < s_maxCheckBoxes; ++i) {
       m_checkBoxes[i]->hide();
   }
}


void ShaderDialog::on_ambientOcclusion_clicked(bool tf)
{
   m_dialog.aoOptionsGroupBox->setEnabled(tf);
}


void ShaderDialog::setFilterParameters(QVariantMap const& map)
{

   // Don't allow these to be enabled from the Preferences
/*
   bool tf;
   tf = map.contains("Antialias") ? map.value("Antialias").toBool() : false;
   m_dialog.antialias->setChecked(tf);

   tf = map.contains("Border") ? map.value("Border").toBool() : false;
   m_dialog.border->setChecked(tf);

   tf = map.contains("AmbientOcclusion") ? map.value("AmbientOcclusion").toBool() : false;
   m_dialog.ambientOcclusion->setChecked(tf);
   m_dialog.aoOptionsGroupBox->setEnabled(tf);
*/

   double value;
   
   value = map.contains("AORadius") ? map.value("AORadius").toDouble() : 0.0;
   m_dialog.aoRadius->setValue(int(100.0*value));

   value = map.contains("AOStrength") ? map.value("AOStrength").toDouble() : 0.0;
   m_dialog.aoStrength->setValue(int(100.0*value));

   value = map.contains("AOTotal") ? map.value("AOTotal").toDouble() : 0.0;
   m_dialog.aoTotal->setValue(int(100.0*value));
}


void ShaderDialog::setParameters(QVariantMap const& map)
{
   QString name;
   double val;
   bool ok;

   hideOptionControls();
   int sliderCount(0);
   int checkBoxCount(0);

   for (QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
       name = iter.key();
       name.replace("user_","");
       name.replace("_"," ");
   
       switch (iter.value().type()) {
          case QVariant::Bool:
             if (checkBoxCount < s_maxCheckBoxes) {
                m_checkBoxes[checkBoxCount]->show();
                m_checkBoxes[checkBoxCount]->setText(name);
                m_checkBoxes[checkBoxCount]->setChecked(iter.value().toBool());
                ++checkBoxCount;
             }
             break;

          case QVariant::Double:
          case QMetaType::Float:
             val = iter.value().toDouble(&ok);  
             if (ok && val <= 1.0 && sliderCount < s_maxSliders) {  
                m_labels[sliderCount]->show();
                m_labels[sliderCount]->setText(name);
                m_sliders[sliderCount]->show();
                m_sliders[sliderCount]->setValue(100*val);
                ++sliderCount;
             }
             break;

          default:
             qDebug() << "Unsupported QVariant type in ShaderDialog" << iter.value().type();
             break;

       }

   }  
}


QVariantMap ShaderDialog::getFilterParameters()
{
   QVariantMap map;
   map.insert("Antialias", QVariant(m_dialog.antialias->isChecked()));
   map.insert("Border", QVariant(m_dialog.border->isChecked()));
   map.insert("AmbientOcclusion", QVariant(m_dialog.ambientOcclusion->isChecked()));
   map.insert("AORadius", QVariant(m_dialog.aoRadius->value()/100.0));
   map.insert("AOStrength", QVariant(m_dialog.aoStrength->value()/100.0));
   map.insert("AOTotal", QVariant(m_dialog.aoTotal->value()/100.0));
   return map;
}


QVariantMap ShaderDialog::getParameters()
{
   QVariantMap map;
   for (int i = 0; i < s_maxSliders; ++i) {
       if (m_sliders[i]->isVisible()) {
          QString name(m_labels[i]->text().replace(" ","_"));
          name.prepend("user_");
          double value(m_sliders[i]->value()/100.0);
          map.insert(name, QVariant(value));
       }
   }

   for (int i = 0; i < s_maxCheckBoxes; ++i) {
       if (m_checkBoxes[i]->isVisible()) {
          QString name(m_checkBoxes[i]->text().replace(" ","_"));
          name.prepend("user_");
          bool value(m_checkBoxes[i]->isChecked());
          map.insert(name, QVariant(value));
       }
   }
 
   return map;
}



void ShaderDialog::installFilterParameters()
{
   ShaderLibrary& library(ShaderLibrary::instance());
   library.setFilterVariables(getFilterParameters());
   updated();
}


void ShaderDialog::installShaderParameters(int)
{
   ShaderLibrary& library(ShaderLibrary::instance());
   QString name(m_dialog.shaderCombo->currentText());
   library.setUniformVariables(name, getParameters());
   updated();
}


void ShaderDialog::on_saveAsDefault_clicked(bool)
{
   qDebug() << "Setting Default Shader" << m_dialog.shaderCombo->currentText();
   qDebug() << getParameters();
   qDebug() << "Setting Default Shader";
   qDebug() << getFilterParameters();
   Preferences::DefaultShader(m_dialog.shaderCombo->currentText());
   Preferences::DefaultShaderParameters(getParameters());
   Preferences::DefaultFilterParameters(getFilterParameters());
}


} // end namespace IQmol
