#ifndef IQMOL_SHADERDIALOG_H
#define IQMOL_SHADERDIALOG_H
/*******************************************************************************
         
  Copyright (C) 2011-2015 Andrew Gilbert
      
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

#include "ui_ShaderDialog.h"


namespace IQmol {

   class ShaderLibrary;

   class ShaderDialog : public QDialog {

      Q_OBJECT

      public:
         ShaderDialog(ShaderLibrary&, QWidget* parent);

      Q_SIGNALS:
         void updated();  // to trigger a redraw

      private Q_SLOTS:
         void on_shaderCombo_currentIndexChanged(int index);
         void on_saveAsDefault_clicked(bool);
         void on_ambientOcclusion_clicked(bool);
         void installShaderParameters(int);

         void installFilterParameters();
         void installFilterParameters(bool) { installFilterParameters(); }
         void installFilterParameters(int)  { installFilterParameters(); }

         void setPovRayParameter(int);
         void setPovRayParameter(double);

      private:
         void hideOptionControls();

         void setupShaderTab();
         void setupEffectsTab();
         void setupPovRayTab();

         void copyParametersToDialog(QVariantMap const& parameters);
         QVariantMap getParametersFromDialog();

         void copyFilterParametersToDialog(QVariantMap const& parameters);
         QVariantMap getFilterParametersFromDialog();

         void copyPovRayParametersToDialog(QVariantMap const& parameters);
         QVariantMap getPovRayParametersFromDialog();

         static const int s_maxSliders = 6;
         static const int s_maxCheckBoxes = 6;
         static const int s_maxLightBoxes = 6;

         QLabel*    m_labels[s_maxSliders];
         QSlider*   m_sliders[s_maxSliders];
         QCheckBox* m_checkBoxes[s_maxCheckBoxes];
         QCheckBox* m_lightBoxes[s_maxLightBoxes];
         Ui::ShaderDialog m_dialog;
         ShaderLibrary& m_shaderLibrary;
   };


} // end namespace IQmol

#endif
