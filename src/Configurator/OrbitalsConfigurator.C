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

#include "OrbitalsConfigurator.h"
#include "CanonicalOrbitalsLayer.h"
#include "PopulationsDialog.h"
#include "Preferences.h"
#include "CustomPlot.h"
#include "QsLog.h"
#include <QColorDialog>
#include <QMouseEvent>
#include <QVector>


namespace IQmol {
namespace Configurator {

Orbitals::Orbitals(Layer::Orbitals& orbitals)
  : m_orbitals(orbitals), m_nAlpha(0), m_nBeta(0), m_nOrbitals(0), m_customPlot(0)
{
   m_configurator.setupUi(this);

   connect(m_configurator.orbitalRangeMin, SIGNAL(currentIndexChanged(int)),
      m_configurator.orbitalRangeMax, SLOT(setCurrentIndex(int)));

   m_configurator.surfaceType->clear();
   // Watch the ordering of these affects the index selection below
   m_configurator.surfaceType->addItem("Basis Function", Data::SurfaceType::BasisFunction);
  
   if (m_orbitals.m_orbitals.orbitalType() == Data::Orbitals::NaturalTransition) {
      m_configurator.surfaceType->addItem("Alpha NTO",  Data::SurfaceType::AlphaOrbital);
      m_configurator.surfaceType->addItem("Beta NTO",   Data::SurfaceType::BetaOrbital);
    //m_configurator.surfaceType->addItem("Transition Density", TotalDensity);
   }else if (m_orbitals.m_orbitals.orbitalType() == Data::Orbitals::Dyson) {
      m_configurator.surfaceType->addItem("Dyson (Left)",  Data::SurfaceType::DysonLeft);
      m_configurator.surfaceType->addItem("Dyson (Right)", Data::SurfaceType::DysonRight);
   }else {
      m_configurator.surfaceType->addItem("Alpha Orbital",  Data::SurfaceType::AlphaOrbital);
      m_configurator.surfaceType->addItem("Beta Orbital",   Data::SurfaceType::BetaOrbital);
   }

   setPositiveColor(Preferences::PositiveSurfaceColor());
   setNegativeColor(Preferences::NegativeSurfaceColor());
   m_configurator.opacity->setValue(Preferences::SurfaceOpacity());

   m_pen.setColor(Qt::blue);
   m_pen.setStyle(Qt::SolidLine);
   m_pen.setWidth(1);

   m_selectedPen.setColor(Qt::red);
   m_selectedPen.setStyle(Qt::SolidLine);
   m_selectedPen.setWidth(3);
}


Orbitals::~Orbitals()
{
   if (m_customPlot) delete m_customPlot;
}


void Orbitals::init() 
{ 
   m_nAlpha    = m_orbitals.nAlpha();
   m_nBeta     = m_orbitals.nBeta();
   m_nOrbitals = m_orbitals.nOrbitals();

   switch (m_orbitals.orbitalType()) {
      case Data::Orbitals::Undefined:
         QLOG_WARN() << "Undefined orbitals in Orbitals::Configurator::init()";
         break;

      case Data::Orbitals::Dyson:
      case Data::Orbitals::Localized:
         // Orbital energy diagram doesn't make sense here
         m_configurator.energyFrame->hide();
         m_configurator.energyLabel->hide();
         resize(sizeHint());
         break;

      case Data::Orbitals::NaturalBond:
         QLOG_WARN() << "NBOs requested in Orbitals::Configurator::init() (NYI)";
         break;
      
      default:
         if (m_nOrbitals > 0) initPlot();
         break;
   }

   updateOrbitalRange(true);
   // 0 => basis function, 1 => alpha orbitals
   m_configurator.surfaceType->setCurrentIndex(1); 


   Data::DensityList& densities(m_orbitals.m_availableDensities);
   qDebug() << "Appending additional densities" << densities.size();
   Data::DensityList::iterator density;
   for (density = densities.begin(); density != densities.end(); ++density) {
       (*density)->surfaceType().dump();
        //if ((*density)->surfaceType().kind() == Data::SurfaceType::Custom) {
        //   m_configurator.surfaceType->addItem((*density)->label(),  CustomDensity);
       // }
        m_configurator.surfaceType->addItem((*density)->label(),  
            Data::SurfaceType::CustomDensity);
   }

   if (!m_orbitals.hasPopulations()) {
      m_configurator.populationsButton->hide();
   }
}


void Orbitals::initPlot()
{
   m_customPlot = new CustomPlot(); 
   m_customPlot->setMinimumSize(QSize(300,200));
   m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
   m_customPlot->axisRect()->setRangeDrag(m_customPlot->yAxis->orientation());
   m_customPlot->axisRect()->setRangeZoom(m_customPlot->yAxis->orientation());
   m_customPlot->xAxis->setSelectableParts(QCPAxis::spNone);
   m_customPlot->yAxis->setNumberPrecision(3);
   
   QFrame* frame(m_configurator.energyFrame);
   QVBoxLayout* layout(new QVBoxLayout());
   // TODO: energyFrame already has a layout, apparently
   frame->setLayout(layout);
   layout->addWidget(m_customPlot);

   QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
   textTicker->addTick(0.75, "Alpha");
   textTicker->addTick(2.5, "Beta");
   m_customPlot->xAxis->setTicker(textTicker);
   m_customPlot->xAxis->setSubTicks(false);
   m_customPlot->xAxis->setRange(0,3.25);

   unsigned nOrbs(m_orbitals.nOrbitals());
   unsigned nAlpha(m_orbitals.nAlpha());
   unsigned nBeta(m_orbitals.nBeta());

   QVector<double>  xAlpha(nOrbs), yAlpha(nOrbs), xBeta(nOrbs), yBeta(nOrbs); 
   QVector<double> a(1), b(1), y(1);
   QCPGraph* graph(0);

   bool ntos(m_orbitals.orbitalType() == Data::Orbitals::NaturalTransition);

   if (ntos) {
     // nBeta = (nBeta == nAlpha) ? 0 : nOrbs/2;
      nAlpha = nBeta = nOrbs/2;
   }

   unsigned i(0), g(0);
   if (nAlpha > 0) {
      while (i < nOrbs) {

          g = 1; // degeneracy
          if (ntos) {
             y[0] = m_orbitals.alphaOrbitalAmplitude(i);
             while (i+g < nOrbs && 
                 std::abs(y[0]-m_orbitals.alphaOrbitalAmplitude(i+g)) < 0.001) { ++g; }
          }else{
             y[0] = m_orbitals.alphaOrbitalEnergy(i);
             while (i+g < nOrbs && 
                 std::abs(y[0]-m_orbitals.alphaOrbitalEnergy(i+g)) < 0.001) { ++g; }
          }

          for (unsigned k = i; k < i+g; ++k) {
              a[0]  = 0.75 - 0.25*(g-1) + (k-i)*0.50;
              graph = m_customPlot->addGraph();
              graph->setData(a, y);
              graph->setName(QString::number(k));
              if (ntos) {
                 graph->setScatterStyle(k<nAlpha ? QCPScatterStyle::ssHole
                                                 : QCPScatterStyle::ssOccupied);
              }else {
                 graph->setScatterStyle(k<nAlpha ? QCPScatterStyle::ssOccupied 
                                                 : QCPScatterStyle::ssVirtual);
              }
              connect(graph, SIGNAL(selectionChanged(bool)), 
                 this, SLOT(plotSelectionChanged(bool)));
          }
          i += g;
      }
   }

   if (nBeta > 0) {
      i = 0;
      while (i < nOrbs) {
          g = 1; // degeneracy
          if (ntos) {
             y[0] = m_orbitals.betaOrbitalAmplitude(i);
             while (i+g < nOrbs && 
                 std::abs(y[0]-m_orbitals.betaOrbitalAmplitude(i+g)) < 0.001) { ++g; }
          }else{
             y[0] = m_orbitals.betaOrbitalEnergy(i);
             while (i+g < nOrbs && 
                 std::abs(y[0]-m_orbitals.betaOrbitalEnergy(i+g)) < 0.001) { ++g; }
          }

          for (unsigned k = i; k < i+g; ++k) {
              a[0]  = 2.50 - 0.25*(g-1) + (k-i)*0.50;
              graph = m_customPlot->addGraph();
              graph->setData(a, y);
              graph->setName(QString::number(k+nOrbs));
              if (ntos) {
                 graph->setScatterStyle(k<nAlpha ? QCPScatterStyle::ssHole
                                                 : QCPScatterStyle::ssOccupied);
              }else {
                 graph->setScatterStyle(k<nAlpha ? QCPScatterStyle::ssOccupied 
                                                 : QCPScatterStyle::ssVirtual);
              }
              connect(graph, SIGNAL(selectionChanged(bool)), 
                 this, SLOT(plotSelectionChanged(bool)));
          }
          i += g;
      }
   }


   // Set the scale
   double yMin(-1.0), yMax(1.0);
   unsigned index, nShow(5);  // Show 5 occupied and virtual orbitals to start with
   
   if (ntos) {
      // The first half of the orbitals correspond to holes, 
      // with increasing vacancies.
      yMin = std::min(m_orbitals.alphaOrbitalAmplitude(nOrbs/2-1), 
                      m_orbitals.betaOrbitalAmplitude( nOrbs/2-1));
      yMax = std::max(m_orbitals.alphaOrbitalAmplitude(nOrbs/2), 
                      m_orbitals.betaOrbitalAmplitude( nOrbs/2));
      m_customPlot->yAxis->setLabel("Occupancy");

   }else {

      if (nBeta > nAlpha) {
         // Use the beta energies instead of alpha
         index = nBeta < nShow ? 0 : nBeta-nShow;
         yMin  = m_orbitals.betaOrbitalEnergy(index);
         index = nOrbs > nBeta+nShow ? nBeta+nShow : nOrbs-1;
         yMax  = m_orbitals.betaOrbitalEnergy(index);
      }else {
         index = nAlpha < nShow ? 0 : nAlpha-nShow;
         yMin  = m_orbitals.alphaOrbitalEnergy(index);
         index = nOrbs > nAlpha+nShow ? nAlpha+nShow : nOrbs-1;
         yMax  = m_orbitals.alphaOrbitalEnergy(index);
      }
      yMax = std::min(yMax, 0.5*std::abs(yMin));
      m_customPlot->yAxis->setLabel("Energy/Hartree");

   }

   m_customPlot->yAxis->setRange(1.05*yMin,1.05*yMax);
}


void Orbitals::plotSelectionChanged(bool tf)
{
   QCPGraph* graph(qobject_cast<QCPGraph*>(sender()));
   if (!graph) return;
   
   if (tf) {
      graph->setPen(m_selectedPen);
   }else {
      graph->setPen(m_pen);
      m_configurator.energyLabel->setText("");
      return;
   }

   bool ok;
   unsigned orb(graph->name().toUInt(&ok));
   unsigned nOrbs(m_orbitals.nOrbitals());
   if (!ok) return;

   double energy(0.0);
   double amplitude(0.0);
   QString label;

   if (orb < nOrbs) {  //alpha
      energy    = m_orbitals.alphaOrbitalEnergy(orb);
      amplitude = m_orbitals.alphaOrbitalAmplitude(orb);
      label     = "Alpha ";
      m_configurator.surfaceType->setCurrentIndex(1);
      updateOrbitalRange(true);
   }else {  // beta
      orb -= nOrbs;
      energy    = m_orbitals.betaOrbitalEnergy(orb);
      amplitude = m_orbitals.betaOrbitalAmplitude(orb);
      label     = "Beta ";
      m_configurator.surfaceType->setCurrentIndex(2);
      updateOrbitalRange(false);
   }

   m_configurator.orbitalRangeMin->setCurrentIndex(orb);
   m_configurator.orbitalRangeMax->setCurrentIndex(orb);

   label += QString::number(orb+1);
   if (m_orbitals.orbitalType() == Data::Orbitals::Canonical) {
      label += " orbital energy: " + QString::number(energy, 'f', 3) + " Eh";
   }else if (m_orbitals.orbitalType() == Data::Orbitals::NaturalTransition) {
      label += " NTO occupancy: " + QString::number(amplitude, 'f', 3);
   }

   m_configurator.energyLabel->setText(label);
}


void Orbitals::clearSelectedOrbitals(int)
{
   QList<QCPGraph*> selection(m_customPlot->selectedGraphs());
   QList<QCPGraph*>::iterator iter;
   for (iter = selection.begin(); iter != selection.end(); ++iter) {
//       (*iter)->setSelected(false);
   }
}


void Orbitals::on_surfaceType_currentIndexChanged(int index) 
{
   QVariant qvar(m_configurator.surfaceType->itemData(index));

   switch (qvar.toUInt()) {
      case Data::SurfaceType::BasisFunction:
         enableOrbitalSelection(true);
         enableNegativeColor(true);
         enablePopulations(false);
         updateBasisRange();
         break;

      case Data::SurfaceType::AlphaOrbital:
      case Data::SurfaceType::DysonLeft:
         enableOrbitalSelection(true);
         enableNegativeColor(true);
         enablePopulations(false);
         updateOrbitalRange(true);
         break;

      case Data::SurfaceType::BetaOrbital:
      case Data::SurfaceType::DysonRight:
         enableOrbitalSelection(true);
         enableNegativeColor(true);
         enablePopulations(false);
         updateOrbitalRange(false);
         break;

      case Data::SurfaceType::TotalDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(false);
         enablePopulations(true);
         break;

      case Data::SurfaceType::SpinDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(true);
         enablePopulations(true);
         break;

      case Data::SurfaceType::AlphaDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(false);
         enablePopulations(true);
         break;

      case Data::SurfaceType::BetaDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(false);
         enablePopulations(true);
         break;

      case Data::SurfaceType::CustomDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(false);
         enablePopulations(true);
         break;

      default:
         enableOrbitalSelection(false);
         break;
   }
}


void Orbitals::on_positiveColorButton_clicked(bool)
{
   setPositiveColor(QColorDialog::getColor(Preferences::PositiveSurfaceColor(), this));
}


void Orbitals::on_negativeColorButton_clicked(bool)
{
   setNegativeColor(QColorDialog::getColor(Preferences::NegativeSurfaceColor(), this));
}


void Orbitals::setPositiveColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_configurator.positiveColorButton->setStyleSheet(bg);
      Preferences::PositiveSurfaceColor(color);
   }
}


void Orbitals::setNegativeColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_configurator.negativeColorButton->setStyleSheet(bg);
      Preferences::NegativeSurfaceColor(color);
   }
}


void Orbitals::on_populationsButton_clicked(bool)
{
   // get current density
   // compute matrix of pops

   // A little dodge, we use the label to match the density
   QString label(m_configurator.surfaceType->currentText());

   Data::DensityList& densities(m_orbitals.m_availableDensities);
   Data::DensityList::iterator density;
   for (density = densities.begin(); density != densities.end(); ++density) {
       if ((*density)->label() == label) break;
   }

   if (density == densities.end()) {
      QLOG_WARN() << "Failed to find density" << label;
   }else {
      qDebug() << "found density" << label;
      PopulationsDialog* dialog(
         new PopulationsDialog(m_orbitals.m_orbitals.shellList(), **density, this));
      dialog->show();
      dialog->raise();
   }

}


void Orbitals::on_cancelButton_clicked(bool)
{ 
   clearQueue(); 
   close(); 
}


void Orbitals::on_calculateButton_clicked(bool)
{ 
   on_addToQueueButton_clicked(true);
   Preferences::SurfaceOpacity(m_configurator.opacity->value());
   accept();
   calculateSurfaces(); 
}


void Orbitals::on_addToQueueButton_clicked(bool) 
{
   int quality(m_configurator.quality->value());
   double isovalue(m_configurator.isovalue->value());
   QColor positive(Preferences::PositiveSurfaceColor());
   QColor negative(Preferences::NegativeSurfaceColor());
   bool simplifyMesh(m_configurator.simplifyMeshCheckBox->isChecked());

   int index(m_configurator.surfaceType->currentIndex());
   QVariant qvar(m_configurator.surfaceType->itemData(index));
   bool isSigned(true);

   int op(m_configurator.opacity->value());
   double opacity = op == 100 ? 0.999 : double(op)/100.0;

   Data::SurfaceInfo info(Data::SurfaceType::Custom, quality, isovalue, 
     positive, negative, isSigned, simplifyMesh, opacity);

   switch (qvar.toUInt()) {

      case Data::SurfaceType::BasisFunction: {
         info.type().setKind(Data::SurfaceType::BasisFunction);
         int fn1(m_configurator.orbitalRangeMin->currentIndex());
         int fn2(m_configurator.orbitalRangeMax->currentIndex());

         for (int i = std::min(fn1,fn2); i <= std::max(fn1, fn2); ++i) {
             info.type().setIndex(i);
             queueSurface(info);
         }
      } break;

      case Data::SurfaceType::AlphaOrbital: {
         info.type().setKind(Data::SurfaceType::AlphaOrbital);
         int orb1(m_configurator.orbitalRangeMin->currentIndex());
         int orb2(m_configurator.orbitalRangeMax->currentIndex());

         for (int i = std::min(orb1,orb2); i <= std::max(orb1, orb2); ++i) {
             info.type().setIndex(i);
             queueSurface(info);
         }
      } break;

      case Data::SurfaceType::BetaOrbital: {
         info.type().setKind(Data::SurfaceType::BetaOrbital);
         int orb1(m_configurator.orbitalRangeMin->currentIndex());
         int orb2(m_configurator.orbitalRangeMax->currentIndex());

         for (int i = std::min(orb1,orb2); i <= std::max(orb1, orb2); ++i) {
             info.type().setIndex(i);
             queueSurface(info);
         }
      } break;

      case Data::SurfaceType::DysonLeft: {
         info.type().setKind(Data::SurfaceType::DysonLeft);
         int orb1(m_configurator.orbitalRangeMin->currentIndex());
         int orb2(m_configurator.orbitalRangeMax->currentIndex());

         for (int i = std::min(orb1,orb2); i <= std::max(orb1, orb2); ++i) {
             info.type().setIndex(i);
             queueSurface(info);
         }
      } break;

      case Data::SurfaceType::DysonRight: {
         info.type().setKind(Data::SurfaceType::DysonRight);
         int orb1(m_configurator.orbitalRangeMin->currentIndex());
         int orb2(m_configurator.orbitalRangeMax->currentIndex());

         for (int i = std::min(orb1,orb2); i <= std::max(orb1, orb2); ++i) {
             info.type().setIndex(i);
             queueSurface(info);
         }
      } break;

      case Data::SurfaceType::TotalDensity: {
         info.setIsSigned(false);
         info.type().setKind(Data::SurfaceType::TotalDensity);
         queueSurface(info);
      } break;

      case Data::SurfaceType::SpinDensity: {
            info.type().setKind(Data::SurfaceType::SpinDensity);
         queueSurface(info);
      } break;

      case Data::SurfaceType::AlphaDensity: {
         info.setIsSigned(false);
         info.type().setKind(Data::SurfaceType::AlphaDensity);
         queueSurface(info);
      } break;

      case Data::SurfaceType::BetaDensity: {
         info.setIsSigned(false);
         info.type().setKind(Data::SurfaceType::BetaDensity);
         queueSurface(info);
      } break;

      case Data::SurfaceType::CustomDensity: {
         info.setIsSigned(false);
         info.type().setKind(Data::SurfaceType::Custom);
         info.type().setLabel(m_configurator.surfaceType->currentText());
         queueSurface(info);
      } break;
   }
}


void Orbitals::enableOrbitalSelection(bool tf)
{
   m_configurator.orbitalRangeMin->setEnabled(tf);
   m_configurator.orbitalRangeMax->setEnabled(tf);
}


void Orbitals::enablePopulations(bool tf)
{
   if (m_orbitals.hasPopulations()) {
      m_configurator.populationsButton->setEnabled(tf);
   }
   
}


void Orbitals::enableNegativeColor(bool tf)
{
   m_configurator.negativeColorButton->setVisible(tf);
   m_configurator.negativeLabel->setVisible(tf);
}


void Orbitals::updateOrbitalRange(bool alpha)
{
   m_configurator.orbitalLabel->setText("Orbital(s):");

   QComboBox* combo(0);

   combo = m_configurator.orbitalRangeMin;
   combo->clear();
   combo->addItems(m_orbitals.m_orbitals.labels(alpha));
   combo->setCurrentIndex(m_orbitals.m_orbitals.labelIndex(alpha));

   combo = m_configurator.orbitalRangeMax;
   combo->clear();
   combo->addItems(m_orbitals.m_orbitals.labels(alpha));
   combo->setCurrentIndex(m_orbitals.m_orbitals.labelIndex(alpha)+1);
}


void Orbitals::updateBasisRange() 
{
   m_configurator.orbitalLabel->setText("Function(s):");
   Data::ShellList const& shellList(m_orbitals.m_orbitals.shellList());

   QComboBox* comboMin(m_configurator.orbitalRangeMin);
   QComboBox* comboMax(m_configurator.orbitalRangeMax);

   comboMin->clear();
   comboMax->clear();

   for (int i = 0; i < shellList.size(); ++i) {
       Data::Shell* shell(shellList[i]);
       QString label(QString::number(shell->atomIndex()+1));
       label += " : ";

       for (unsigned j = 0; j < shell->nBasis(); ++j) {
           comboMin->addItem(label+ shell->label(j));
           comboMax->addItem(label+ shell->label(j));
       }
   }
   comboMin->setCurrentIndex(0);
   comboMin->setCurrentIndex(0);
}

} } // end namespace IQmol::Configurator
