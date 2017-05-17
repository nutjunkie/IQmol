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
   m_configurator.surfaceType->addItem("Alpha Orbital", Data::SurfaceType::AlphaOrbital);
   m_configurator.surfaceType->addItem("Beta Orbital",  Data::SurfaceType::BetaOrbital);
   m_configurator.surfaceType->setCurrentIndex(0);

   //if (m_orbitals.moTypeID() == Data::moType::NTOs)
   //{
   //m_configurator.surfaceType->addItem("Alpha Orbital", AlphaOrbital);
   //m_configurator.surfaceType->addItem("Beta Orbital",  BetaOrbital);
   //m_configurator.surfaceType->addItem("Transition Density", TotalDensity);
   //}
   //else
   //{
/*
   m_configurator.surfaceType->addItem("Alpha Orbital", AlphaOrbital);
   m_configurator.surfaceType->addItem("Beta Orbital",  BetaOrbital);
   // TO DO Conditional
   m_configurator.surfaceType->addItem("Total Density", TotalDensity);
   m_configurator.surfaceType->addItem("Spin Density",  SpinDensity);
   m_configurator.surfaceType->addItem("Alpha Density", AlphaDensity);
   m_configurator.surfaceType->addItem("Beta Density",  BetaDensity);
*/
   //}

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
   qDebug() << "Configurator::Orbitals ctor w/" 
            << Data::Orbitals::toString(m_orbitals.orbitalType());

   m_nAlpha    = m_orbitals.nAlpha();
   m_nBeta     = m_orbitals.nBeta();
   m_nOrbitals = m_orbitals.nOrbitals();
   m_AlphaHOMO = m_nAlpha;
   m_BetaHOMO  = m_nBeta;

   switch (m_orbitals.orbitalType()) {
      case Data::Orbitals::Localized:
         // Orbital energy diagram doesn't make sense here
         m_configurator.energyFrame->hide();
         m_configurator.energyLabel->hide();
         resize(sizeHint());
         break;

      case Data::Orbitals::NaturalTransition:
         m_AlphaHOMO = m_BetaHOMO = m_nOrbitals/2;
         if (m_nOrbitals > 0) initPlot();
         break;

//      case Data::Orbitals::NaturalBond:
//         break;
      
      case Data::Orbitals::Canonical:
         if (m_nOrbitals > 0) initPlot();
         break;
   }

   updateOrbitalRange(m_AlphaHOMO);

//   static bool densitiesAppended(false);
//   if (!densitiesAppended) {
//      densitiesAppended = true;
   Data::DensityList& densities(m_orbitals.m_availableDensities);
   qDebug() << "Appending additional densities" << densities.size();
   Data::DensityList::iterator density;
   for (density = densities.begin(); density != densities.end(); ++density) {
       (*density)->surfaceType().dump();
        //if ((*density)->surfaceType().kind() == Data::SurfaceType::Custom) {
        //   m_configurator.surfaceType->addItem((*density)->label(),  CustomDensity);
       // }
        m_configurator.surfaceType->addItem((*density)->label(),  Data::SurfaceType::CustomDensity);
   }
//   }
}


void Orbitals::initPlot()
{
   m_customPlot = new CustomPlot(); 
   m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
   m_customPlot->axisRect()->setRangeDrag(m_customPlot->yAxis->orientation());
   m_customPlot->axisRect()->setRangeZoom(m_customPlot->yAxis->orientation());
   m_customPlot->xAxis->setSelectableParts(QCPAxis::spNone);
   
   QFrame* frame(m_configurator.energyFrame);
   QVBoxLayout* layout(new QVBoxLayout());
   // TODO: energyFrame already has a layout, apparently
   frame->setLayout(layout);
   layout->addWidget(m_customPlot);

   QVector<double>  ticks;
   QVector<QString> labels;

   ticks << 0.75 << 2.50;
   labels << "Alpha" << "Beta";

   m_customPlot->xAxis->setAutoTicks(false);
   m_customPlot->xAxis->setAutoTickLabels(false);
   m_customPlot->xAxis->setTickVector(ticks);
   m_customPlot->xAxis->setTickVectorLabels(labels);
   m_customPlot->xAxis->setSubTickCount(0);
   m_customPlot->xAxis->setRange(0,3.25);

   unsigned nOrbs(m_orbitals.nOrbitals());
   unsigned nAlpha(m_orbitals.nAlpha());
   unsigned nBeta(m_orbitals.nBeta());
   //unsigned moTypeID(m_orbitals.moTypeID());
   QVector<double>  xAlpha(nOrbs), yAlpha(nOrbs), xBeta(nOrbs), yBeta(nOrbs); 
   QVector<double> a(1), b(1), y(1);
   QCPGraph* graph(0);

   // For NTOs
   if (m_orbitals.orbitalType() == Data::Orbitals::NaturalTransition) {
     if (nBeta == nAlpha) {
       nAlpha = nOrbs/2;
       nBeta = 0;
     } else 
       nAlpha = nBeta = nOrbs/2;
   }

   // Alpha
   unsigned i(0), g(0);
   while (i < nOrbs) {
       y[0] = m_orbitals.alphaOrbitalEnergy(i);

       g = 1; // degeneracy
       while (i+g < nOrbs && 
              std::abs(y[0]-m_orbitals.alphaOrbitalEnergy(i+g)) < 0.001) { ++g; }

       for (unsigned k = i; k < i+g; ++k) {
           a[0]  = 0.75 - 0.25*(g-1) + (k-i)*0.50;
           graph = m_customPlot->addGraph();
           graph->setData(a, y);
           graph->setName(QString::number(k));
           graph->setScatterStyle(k<nAlpha ? QCPScatterStyle::ssOccupied 
                                           : QCPScatterStyle::ssVirtual);
           connect(graph, SIGNAL(selectionChanged(bool)), 
              this, SLOT(plotSelectionChanged(bool)));
       }
       i += g;
   }

   if (nBeta > 0) 
   {

   i = 0;
   // Beta
   while (i < nOrbs) {
       y[0] = m_orbitals.betaOrbitalEnergy(i);

       g = 1; // degeneracy
       while (i+g < nOrbs && 
              std::abs(y[0]-m_orbitals.betaOrbitalEnergy(i+g)) < 0.001) { ++g; }

       for (unsigned k = i; k < i+g; ++k) {
           a[0]  = 2.50 - 0.25*(g-1) + (k-i)*0.50;
           graph = m_customPlot->addGraph();
           graph->setData(a, y);
           graph->setName(QString::number(k+nOrbs));
           graph->setScatterStyle(k<nBeta ? QCPScatterStyle::ssOccupied 
                                           : QCPScatterStyle::ssVirtual);
           connect(graph, SIGNAL(selectionChanged(bool)), 
              this, SLOT(plotSelectionChanged(bool)));
       }
       i += g;
   }

   }

   m_customPlot->yAxis->setLabel("Energy/Hartree");
   m_customPlot->yAxis->setNumberPrecision(3);
   // Set the scale
   double yMin(-1.0), yMax(0.5);
   // Show 5 occupied and virtual orbitals to start with
   unsigned index, nShow(5);
   
   if (nBeta > nAlpha) {
      // Use the beta energies instead
      index = nBeta < nShow ? 0 : nBeta-nShow;
      yMin  = m_orbitals.betaOrbitalEnergy(index);
      index = nOrbs > nBeta+nShow ? nBeta+nShow : nOrbs;
      yMax  = m_orbitals.betaOrbitalEnergy(index);
   }else {
      index = nAlpha < nShow ? 0 : nAlpha-nShow;
      yMin  = m_orbitals.alphaOrbitalEnergy(index);
      index = nOrbs > nAlpha+nShow ? nAlpha+nShow : nOrbs;
      yMax  = m_orbitals.alphaOrbitalEnergy(index);
   }
   yMax = std::min(yMax, 0.5*std::abs(yMin));
   // For NTOs
   if (m_orbitals.orbitalType() == Data::Orbitals::NaturalTransition) {
     m_customPlot->yAxis->setLabel("Occupation");
     m_customPlot->yAxis->setRange(-1.1,1.1);
   } else
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
   QString label;
   if (orb < nOrbs) {  //alpha
      energy = m_orbitals.alphaOrbitalEnergy(orb);
      label  = "Alpha orbital ";
      //updateOrbitalRange(m_nAlpha);
      updateOrbitalRange(m_AlphaHOMO);
   }else {  // beta
      orb -= nOrbs;
      energy = m_orbitals.betaOrbitalEnergy(orb);
      label  = "Beta orbital ";
      //updateOrbitalRange(m_nBeta);
      updateOrbitalRange(m_BetaHOMO);
   }

   m_configurator.surfaceType->setCurrentIndex(0);
   m_configurator.orbitalRangeMin->setCurrentIndex(orb);
   m_configurator.orbitalRangeMax->setCurrentIndex(orb);

   label += QString::number(orb+1);
   label += ": ";
   label += QString::number(energy, 'f', 3);
   if (m_orbitals.orbitalType() != Data::Orbitals::NaturalTransition) {
      label += " Eh";
   }
   m_configurator.energyLabel->setText(label);
}


void Orbitals::clearSelectedOrbitals(int)
{
   QList<QCPGraph*> selection(m_customPlot->selectedGraphs());
   QList<QCPGraph*>::iterator iter;
   for (iter = selection.begin(); iter != selection.end(); ++iter) {
       (*iter)->setSelected(false);
   }
}


void Orbitals::on_surfaceType_currentIndexChanged(int index) 
{
   QVariant qvar(m_configurator.surfaceType->itemData(index));

   switch (qvar.toUInt()) {
      case Data::SurfaceType::AlphaOrbital:
         enableOrbitalSelection(true);
         enableNegativeColor(true);
         //updateOrbitalRange(m_nAlpha);
         updateOrbitalRange(m_AlphaHOMO);
         break;

      case Data::SurfaceType::BetaOrbital:
         enableOrbitalSelection(true);
         enableNegativeColor(true);
         //updateOrbitalRange(m_nBeta);
         updateOrbitalRange(m_BetaHOMO);
         break;

      case Data::SurfaceType::TotalDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(false);
         break;

      case Data::SurfaceType::SpinDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(true);
         break;

      case Data::SurfaceType::AlphaDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(false);
         break;

      case Data::SurfaceType::BetaDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(false);
         break;

      case Data::SurfaceType::CustomDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(false);
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

      case Data::SurfaceType::AlphaOrbital: {
         info.type().setKind(Data::SurfaceType::AlphaOrbital);
         int orb1(m_configurator.orbitalRangeMin->currentIndex()+1);
         int orb2(m_configurator.orbitalRangeMax->currentIndex()+1);

         for (int i = std::min(orb1,orb2); i <= std::max(orb1, orb2); ++i) {
             info.type().setIndex(i);
             queueSurface(info);
         }
      } break;

      case Data::SurfaceType::BetaOrbital: {
         info.type().setKind(Data::SurfaceType::BetaOrbital);
         int orb1(m_configurator.orbitalRangeMin->currentIndex()+1);
         int orb2(m_configurator.orbitalRangeMax->currentIndex()+1);

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


void Orbitals::enableNegativeColor(bool tf)
{
   m_configurator.negativeColorButton->setVisible(tf);
   m_configurator.negativeLabel->setVisible(tf);
}


void Orbitals::updateOrbitalRange(int nElectrons) 
{
   int index;
   QComboBox* combo;

   combo = m_configurator.orbitalRangeMin;
   index = combo->currentIndex();
   updateOrbitalRange(nElectrons, combo);
   if (index < 0 || index >= (int)m_nOrbitals) index = nElectrons-1;
   combo->setCurrentIndex(index);

   combo = m_configurator.orbitalRangeMax;
   index = combo->currentIndex();
   updateOrbitalRange(nElectrons, combo);
   if (index < 0 || index >= (int)m_nOrbitals) index = nElectrons;
   combo->setCurrentIndex(index);
}


void Orbitals::updateOrbitalRange(int nElectrons, QComboBox* combo) 
{
   combo->clear();
   for (unsigned int i = 1; i <= m_nOrbitals; ++i) {
       combo->addItem(QString::number(i));
   }
   
   combo->setItemText(nElectrons-1, QString::number(nElectrons) + " (HOMO)");
   combo->setItemText(nElectrons,   QString::number(nElectrons+1) + " (LUMO)");
}

} } // end namespace IQmol::Configurator
