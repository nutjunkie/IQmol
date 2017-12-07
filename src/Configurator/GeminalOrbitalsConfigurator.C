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

#include "GeminalOrbitalsConfigurator.h"
#include "GeminalOrbitalsLayer.h"
#include "Preferences.h"
#include "CustomPlot.h"
#include <QColorDialog>
#include <QMouseEvent>
#include <QVector>


namespace IQmol {
namespace Configurator {

GeminalOrbitals::GeminalOrbitals(Layer::GeminalOrbitals& geminalOrbitals)
  : m_geminalOrbitals(geminalOrbitals), m_customPlot(0)
{
   m_configurator.setupUi(this);

   connect(m_configurator.orbitalRangeMin, SIGNAL(currentIndexChanged(int)),
      m_configurator.orbitalRangeMax, SLOT(setCurrentIndex(int)));

   m_configurator.surfaceType->clear();
   m_configurator.surfaceType->addItem("Geminal", Geminal);
   m_configurator.surfaceType->addItem("Geminal Correlation", Correlation);
   m_configurator.surfaceType->setCurrentIndex(0); 

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


GeminalOrbitals::~GeminalOrbitals()
{
   if (m_customPlot) delete m_customPlot;
}


void GeminalOrbitals::init() 
{ 
   initGeminalRange();
   if (m_geminalOrbitals.nGeminals() > 0) initPlot();
}


void GeminalOrbitals::initPlot()
{
   m_customPlot = new CustomPlot(); 
   m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
   m_customPlot->axisRect()->setRangeDrag(m_customPlot->yAxis->orientation());
   m_customPlot->axisRect()->setRangeZoom(m_customPlot->yAxis->orientation());
   m_customPlot->xAxis->setSelectableParts(QCPAxis::spNone);
   
   QFrame* frame(m_configurator.energyFrame);
   QVBoxLayout* layout(new QVBoxLayout());
   frame->setLayout(layout);
   layout->addWidget(m_customPlot);

   QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
   m_customPlot->xAxis->setTicker(textTicker);
   textTicker->addTick(1.5, "Geminal");

   m_customPlot->xAxis->setTickLabels(false);
   m_customPlot->xAxis->setSubTicks(false);
   m_customPlot->xAxis->setRange(0,3.0);

   unsigned nTot(m_geminalOrbitals.nGeminals() + m_geminalOrbitals.nOpenShell());
   unsigned nAlpha(m_geminalOrbitals.nAlpha());
   unsigned nBeta( m_geminalOrbitals.nBeta());
   //QVector<double>  xAlpha(nOrbs), yAlpha(nOrbs), xBeta(nOrbs), yBeta(nOrbs); 
   QVector<double> a(1), b(1), y(1);
   QCPGraph* graph(0);

   unsigned i(0), g(0);

   while (i < nTot) {
       y[0] = m_geminalOrbitals.geminalOrbitalEnergy(i);

       g = 1; // degeneracy
       while (i+g < nTot && 
              std::abs(y[0]-m_geminalOrbitals.geminalOrbitalEnergy(i+g)) < 0.001) { ++g; }

       for (unsigned k = i; k < i+g; ++k) {
           a[0]  = 1.5 - 0.25*(g-1) + (k-i)*0.50;
           graph = m_customPlot->addGraph();
           graph->setData(a, y);
           graph->setName(QString::number(k));
           if (k < nBeta) {
              graph->setScatterStyle(QCPScatterStyle::ssDoublyOccupied);
           }else if (k < nAlpha) {
              graph->setScatterStyle(QCPScatterStyle::ssOccupied);
           }else {
              graph->setScatterStyle(QCPScatterStyle::ssVirtual);
           }
           connect(graph, SIGNAL(selectionChanged(bool)), 
              this, SLOT(plotSelectionChanged(bool)));
       }
       i += g;
   }

   i = 0;

   m_customPlot->yAxis->setLabel("Energy/Hartree");
   m_customPlot->yAxis->setNumberPrecision(3);
   // Set the scale
   double yMin(-1.0), yMax(0.5);
   // Show 5 occupied and virtual orbitals to start with
   unsigned index, nShow(5);
   
   index = nAlpha < nShow ? 0 : nAlpha-nShow;
   yMin  = m_geminalOrbitals.geminalOrbitalEnergy(index);
   index = nTot> nAlpha+nShow ? nAlpha+nShow : nTot;
   yMax  = m_geminalOrbitals.geminalOrbitalEnergy(index);

   yMax = std::min(yMax, 0.5*std::abs(yMin));
   m_customPlot->yAxis->setRange(1.05*yMin,1.05*yMax);
}


void GeminalOrbitals::initGeminalRange() 
{
   QComboBox* min(m_configurator.orbitalRangeMin);
   QComboBox* max(m_configurator.orbitalRangeMax);

   min->clear();
   max->clear();
   unsigned nAlpha(m_geminalOrbitals.nAlpha());
   unsigned nBeta( m_geminalOrbitals.nBeta());

   for (unsigned int i = 1; i <= nBeta; ++i) { 
       min->addItem(QString::number(i));
       max->addItem(QString::number(i));
   }
   for (unsigned int i = nBeta+1; i <= nAlpha; ++i) { 
       min->addItem(QString::number(i) + " OS");
       max->addItem(QString::number(i) + " OS");
   }
}


void GeminalOrbitals::plotSelectionChanged(bool tf)
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
  // unsigned nOrbs(m_geminalOrbitals.nAlpha());
   if (!ok) return;

   double energy(0.0);
   QString label;
   energy = m_geminalOrbitals.geminalOrbitalEnergy(orb);
   label  = "Geminal ";

   //m_configurator.surfaceType->setCurrentIndex(0);
   m_configurator.orbitalRangeMin->setCurrentIndex(orb);
   m_configurator.orbitalRangeMax->setCurrentIndex(orb);

   label += QString::number(orb+1);
   label += ": ";
   label += QString::number(energy, 'f', 3);
   label += " Eh";
   m_configurator.energyLabel->setText(label);
}


void GeminalOrbitals::clearSelectedOrbitals(int)
{
   QList<QCPGraph*> selection(m_customPlot->selectedGraphs());
   QList<QCPGraph*>::iterator iter;
   for (iter = selection.begin(); iter != selection.end(); ++iter) {
//       (*iter)->setSelected(false);
   }
}


void GeminalOrbitals::on_positiveColorButton_clicked(bool)
{
   setPositiveColor(QColorDialog::getColor(Preferences::PositiveSurfaceColor(), this));
}


void GeminalOrbitals::on_negativeColorButton_clicked(bool)
{
   setNegativeColor(QColorDialog::getColor(Preferences::NegativeSurfaceColor(), this));
}


void GeminalOrbitals::setPositiveColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_configurator.positiveColorButton->setStyleSheet(bg);
      Preferences::PositiveSurfaceColor(color);
   }
}


void GeminalOrbitals::setNegativeColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_configurator.negativeColorButton->setStyleSheet(bg);
      Preferences::NegativeSurfaceColor(color);
   }
}


void GeminalOrbitals::on_cancelButton_clicked(bool)
{ 
   clearQueue(); 
   close(); 
}


void GeminalOrbitals::on_calculateButton_clicked(bool)
{ 
   on_addToQueueButton_clicked(true);
   Preferences::SurfaceOpacity(m_configurator.opacity->value());
   accept();
   calculateSurfaces(); 
}


void GeminalOrbitals::on_addToQueueButton_clicked(bool) 
{
   int quality(m_configurator.quality->value());
   double isovalue(m_configurator.isovalue->value());
   QColor positive(Preferences::PositiveSurfaceColor());
   QColor negative(Preferences::NegativeSurfaceColor());
   bool simplifyMesh(m_configurator.simplifyMeshCheckBox->isChecked());

   int index(m_configurator.surfaceType->currentIndex());
   bool isSigned(true);

   int op(m_configurator.opacity->value());
   double opacity = op == 100 ? 0.999 : double(op)/100.0;

   Data::SurfaceInfo info(Data::SurfaceType::Custom, quality, isovalue, 
     positive, negative, isSigned, simplifyMesh, opacity);

   QVariant qvar(m_configurator.surfaceType->itemData(index));
   switch (qvar.toUInt()) {
      case Geminal: info.type().setKind(Data::SurfaceType::Geminal); break;
      case Correlation: info.type().setKind(Data::SurfaceType::Correlation); break;
    }

    int orb1(m_configurator.orbitalRangeMin->currentIndex()+1);
    int orb2(m_configurator.orbitalRangeMax->currentIndex()+1);

    for (int i = std::min(orb1,orb2); i <= std::max(orb1, orb2); ++i) {
        info.type().setIndex(i);
        queueSurface(info);
    }
}



} } // end namespace IQmol::Configurator
