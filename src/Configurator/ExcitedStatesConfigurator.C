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

#include "ExcitedStatesConfigurator.h"
#include "ExcitedStatesLayer.h"
#include "ElectronicTransition.h"
#include "ExcitedStates.h"
#include "CustomPlot.h"
#include <cmath>


namespace IQmol {
namespace Configurator {

ExcitedStates::ExcitedStates(Layer::ExcitedStates& excitedStates) : m_excitedStates(excitedStates),
  m_moPlot(0), m_spectrum(0)
{
   m_configurator.setupUi(this);
   m_configurator.energyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
   m_configurator.widthSlider->setEnabled(false);
   m_configurator.widthLabel->setEnabled(false);

   m_pen.setColor(Qt::blue);
   m_pen.setStyle(Qt::SolidLine);
   m_pen.setWidth(1);

   m_selectedPen.setColor(Qt::red);
   m_selectedPen.setStyle(Qt::SolidLine);
   m_selectedPen.setWidth(3);

   initSpectrum();
   initMoPlot();
}


ExcitedStates::~ExcitedStates()
{
   clearTransitionLines();
   if (m_moPlot) delete m_moPlot;
   if (m_spectrum) delete m_spectrum;
}


void ExcitedStates::initSpectrum()
{
   m_spectrum = new CustomPlot(); 
   m_spectrum->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
   m_spectrum->axisRect()->setRangeDrag(m_spectrum->xAxis->orientation());
   m_spectrum->axisRect()->setRangeZoom(m_spectrum->xAxis->orientation());
   m_spectrum->xAxis->setSelectableParts(QCPAxis::spNone);
   m_spectrum->xAxis->setLabel("Energy/eV");

   QFrame* frame(m_configurator.spectrumFrame);
   QVBoxLayout* layout(new QVBoxLayout());
   frame->setLayout(layout);
   layout->addWidget(m_spectrum);
}


void ExcitedStates::load(Data::ExcitedStates const& states)
{
   Data::ElectronicTransitionList const& transitions(states.transitions());

   QString label(states.typeLabel());
   label += " Excited States";
   setWindowTitle(label);

   QTableWidget* table(m_configurator.energyTable); 
   table->setRowCount(transitions.size());
   QTableWidgetItem* item;

   m_maxValues.first  = 0;
   m_maxValues.second = 0;

   int row(0);
   Data::ElectronicTransitionList::const_iterator iter;
   for (iter = transitions.begin(); iter != transitions.end(); ++iter) {
       double energy((*iter)->energy());
       double strength((*iter)->strength());
       double spinSquared((*iter)->spinSquared());

       m_maxValues.first  = std::max(m_maxValues.first,  energy);
       m_maxValues.second = std::max(m_maxValues.second, strength);

       m_rawData.append(qMakePair(energy, strength));

       QString text(QString::number(energy, 'f', 3));
       item = new QTableWidgetItem( text + "     ");
       item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
       item->setData(Qt::UserRole, row);
       table->setItem(row, 0, item);

       text = QString::number(strength, 'f', 3);
       item = new QTableWidgetItem( text + "    ");
       item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
       item->setData(Qt::UserRole, row);
       table->setItem(row, 1, item);

       text = QString::number(spinSquared, 'f', 3);
       item = new QTableWidgetItem( text + "    ");
       item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
       item->setData(Qt::UserRole, row);
       table->setItem(row, 2, item);
 
       ++row;
   }

   updateSpectrum();
}


void ExcitedStates::updateSpectrum()
{
   m_spectrum->xAxis->setLabel("Energy/eV");
   m_spectrum->clearGraphs();
   double width(m_configurator.widthSlider->value());

   if (m_configurator.impulseButton->isChecked()) {
      plotImpulse();
      m_spectrum->yAxis->setLabel("Strength");

   }else if (m_configurator.gaussianButton->isChecked()) {
      plotSpectrum(Gaussian, width);
      m_spectrum->yAxis->setLabel("Rel. Strength");

   }else if (m_configurator.lorentzianButton->isChecked()) {
      plotSpectrum(Lorentzian, width);
      m_spectrum->yAxis->setLabel("Rel. Strength");
   }

   m_spectrum->replot();
}


void ExcitedStates::plotImpulse()
{
   QVector<double> x(1), y(1);

   for (int transition = 0; transition < m_rawData.size(); ++transition) {
       x[0] = m_rawData[transition].first;
       y[0] = m_rawData[transition].second;

       QCPGraph* graph(m_spectrum->addGraph());
       graph->setData(x, y);
       graph->setName(QString::number(transition));
       graph->setLineStyle(QCPGraph::lsImpulse);
       graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
       graph->setPen(m_pen);
       graph->setSelectedPen(m_selectedPen);
       connect(graph, SIGNAL(selectionChanged(bool)), this, SLOT(plotSelectionChanged(bool)));
   }

   double const maxEnergy(m_maxValues.first+3);
   m_spectrum->xAxis->setRange(0, maxEnergy);
   m_spectrum->yAxis->setRange(0, 1.05*m_maxValues.second);
   m_spectrum->yAxis->setAutoTickStep(true);
}


void ExcitedStates::plotSpectrum(Profile const profile, double const width)
{
//qDebug() << "Plot spectrum called";
   unsigned const bins(200);
   double const maxEnergy(m_maxValues.first+3);
   double const delta(maxEnergy/bins);

   QVector<double> x(bins), y(bins);

   for (unsigned xi = 0; xi < bins; ++xi) {
       x[xi] = xi*delta;
       y[xi] = 0.0;
   }

   switch (profile) {

      case Gaussian: {
         double g(0.02*width);
         double A(std::sqrt(4.0*std::log(2.0)/(g*M_PI)));
         double a(-4.0*std::log(2.0)/g);
         for (int mode = 0; mode < m_rawData.size(); ++mode) {
             double nu(m_rawData[mode].first);
             double I(m_rawData[mode].second);
             for (unsigned xi = 0; xi < bins; ++xi) {
                 y[xi] += I*A*std::exp(a*(x[xi]-nu)*(x[xi]-nu));
             }
         }
      } break;

      case Lorentzian: {
         double A(2.0/M_PI);
         double g(0.005*width);
         double g2(g*g);
         for (int mode = 0; mode < m_rawData.size(); ++mode) {
             double nu(m_rawData[mode].first);
             double I(m_rawData[mode].second);
             for (unsigned xi = 0; xi < bins; ++xi) {
                 y[xi] += I*A*g / (g2+(x[xi]-nu)*(x[xi]-nu));
             }
         }
      } break;

   }

   double maxIntensity(0.0);
   for (unsigned xi = 0; xi < bins; ++xi) {
       if (y[xi] > maxIntensity) maxIntensity = y[xi];
   }

   for (unsigned xi = 0; xi < bins; ++xi) {
       y[xi] /= maxIntensity;
   }

   QCPGraph* graph(m_spectrum->addGraph());
   graph->setData(x, y);
   graph->setPen(m_pen);
   graph->setAntialiased(true);
   graph->setSelectedPen(m_selectedPen);

   m_spectrum->xAxis->setRange(0, maxEnergy);
   m_spectrum->yAxis->setRange(0, 1.00);
   m_spectrum->yAxis->setAutoTickStep(false);
   m_spectrum->yAxis->setTickStep(0.2);
}



void ExcitedStates::plotSelectionChanged(bool tf)
{      
   QCPGraph* graph(qobject_cast<QCPGraph*>(sender()));
   if (!graph) return;
       
   if (tf) {
      graph->setPen(m_selectedPen);
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc));
   }else {
      graph->setPen(m_pen);
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
      return;
   }

   if (!tf) return;
   if (!m_configurator.impulseButton->isChecked()) return;

   bool ok;
   int row(graph->name().toInt(&ok));
   if (!ok) return;

   QTableWidget* table(m_configurator.energyTable);
   table->setCurrentCell(row, 0, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   table->scrollToItem(table->item(row,0));
   updateMoPlot(row);
}


void ExcitedStates::on_impulseButton_clicked(bool)
{
   m_configurator.widthSlider->setEnabled(false);
   m_configurator.widthLabel->setEnabled(false);
   updateSpectrum();
}


void ExcitedStates::on_gaussianButton_clicked(bool)
{
   m_configurator.widthSlider->setEnabled(true);
   m_configurator.widthLabel->setEnabled(true);
   updateSpectrum();
}


void ExcitedStates::on_lorentzianButton_clicked(bool)
{
   m_configurator.widthSlider->setEnabled(true);
   m_configurator.widthLabel->setEnabled(true);
   updateSpectrum();
}


void ExcitedStates::on_widthSlider_valueChanged(int)
{
   updateSpectrum();
}


void ExcitedStates::on_energyTable_itemSelectionChanged()
{      
   if (!m_configurator.impulseButton->isChecked()) return;

   QList<QTableWidgetItem*> selection = m_configurator.energyTable->selectedItems();
   if (selection.isEmpty()) return;

   int index(selection.first()->row());
   QCPGraph* graph(m_spectrum->graph(index));
   if (graph && graph->selected()) return;

   QList<QCPGraph*> selectedGraphs(m_spectrum->selectedGraphs());
   QList<QCPGraph*>::iterator iter;
   for (iter = selectedGraphs.begin(); iter != selectedGraphs.end(); ++iter) {
       (*iter)->setSelected(false);
   }

   if (graph) {
       graph->setSelected(true);
       m_spectrum->replot();
   }

   updateMoPlot(index);
}


void ExcitedStates::updateMoPlot(int const index)
{
   clearTransitionLines();
   QList<Data::Amplitude>  amplitudes(m_excitedStates.stateData().amplitudes(index));

   qDebug() << "Number of amplitudes" << amplitudes.size();

   QPen pen;
   QColor color;
   pen.setStyle(Qt::SolidLine);
   pen.setWidth(3);

   double dx(-0.2);
   QList<Data::Amplitude>::const_iterator iter;
   for (iter = amplitudes.begin(); iter != amplitudes.end(); ++iter) {

       color.setHsv(280, 255*std::abs((*iter).m_amplitude), 255);
       pen.setColor(color);

       QCPItemLine* line(new QCPItemLine(m_moPlot));
       line->setSelectable(true);
       m_transitionLines.append(line);

       double x( ((*iter).m_spin == Data::Alpha) ? 0.75 : 2.5 );
       line->position("start")->setCoords(x+dx, (*iter).m_ei);
       line->position("end")->setCoords(x+dx, (*iter).m_ea);
       dx += 0.1;

       line->setPen(pen);
       line->setSelectedPen(m_selectedPen);
       line->setHead(QCPLineEnding::esSpikeArrow);
   }

   m_moPlot->replot();
}


void ExcitedStates::initMoPlot()
{
   m_moPlot   = new CustomPlot(); 
   m_moPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
   m_moPlot->axisRect()->setRangeDrag(m_moPlot->yAxis->orientation());
   m_moPlot->axisRect()->setRangeZoom(m_moPlot->yAxis->orientation());
   m_moPlot->xAxis->setSelectableParts(QCPAxis::spNone);
   
   QFrame* frame(m_configurator.moFrame);
   QVBoxLayout* layout(new QVBoxLayout());
   frame->setLayout(layout);
   layout->addWidget(m_moPlot);

   //layout->addWidget(new QLabel("This is a test"));
   //layout->setStretch(0,1);
   //layout->setStretch(1,0);

   QVector<double>  ticks;
   QVector<QString> labels;

   ticks << 0.75 << 2.50;
   labels << "Alpha" << "Beta";

   m_moPlot->xAxis->setAutoTicks(false);
   m_moPlot->xAxis->setAutoTickLabels(false);
   m_moPlot->xAxis->setTickVector(ticks);
   m_moPlot->xAxis->setTickVectorLabels(labels);
   m_moPlot->xAxis->setSubTickCount(0);
   m_moPlot->xAxis->setRange(0,3.25);

   Data::OrbitalSymmetries const& orbitals(m_excitedStates.stateData().orbitalSymmetries());

   unsigned nOrbs(orbitals.nOrbitals());
   unsigned nAlpha(orbitals.nAlpha());
   unsigned nBeta(orbitals.nBeta());
   QVector<double>  xAlpha(nOrbs), yAlpha(nOrbs), xBeta(nOrbs), yBeta(nOrbs); 
   QVector<double> a(1), b(1), y(1);
   QCPGraph* graph(0);

   unsigned i(0), g(0);

   Data::Spin spin(Data::Alpha);

   while (i < nOrbs) {
       y[0] = orbitals.energy(spin, i);

       g = 1; // degeneracy
       while (i+g < nOrbs && 
              std::abs(y[0]-orbitals.energy(spin, i+g)) < 0.001) { ++g; }

       for (unsigned k = i; k < i+g; ++k) {
           a[0]  = 0.75 - 0.25*(g-1) + (k-i)*0.50;
           graph = m_moPlot->addGraph();
           graph->setData(a, y);
           graph->setName(QString::number(k));
           graph->setScatterStyle(k<nAlpha ? QCPScatterStyle::ssOccupied 
                                           : QCPScatterStyle::ssVirtual);
           connect(graph, SIGNAL(selectionChanged(bool)), this, SLOT(moSelectionChanged(bool)));
       }

       i += g;
   }

   i    = 0;
   spin = Data::Beta;

   while (i < nOrbs) {
       y[0] = orbitals.energy(spin, i);

       g = 1; // degeneracy
       while (i+g < nOrbs && 
              std::abs(y[0]-orbitals.energy(spin, i+g)) < 0.001) { ++g; }

       for (unsigned k = i; k < i+g; ++k) {
           a[0]  = 2.50 - 0.25*(g-1) + (k-i)*0.50;
           graph = m_moPlot->addGraph();
           graph->setData(a, y);
           graph->setName(QString::number(k+nOrbs));
           graph->setScatterStyle(k<nBeta ? QCPScatterStyle::ssOccupied 
                                           : QCPScatterStyle::ssVirtual);
           connect(graph, SIGNAL(selectionChanged(bool)), this, SLOT(moSelectionChanged(bool)));
       }

       i += g;
   }

   m_moPlot->yAxis->setLabel("Energy/Hartree");
   m_moPlot->yAxis->setNumberPrecision(3);
   // Set the scale
   double yMin(-1.0), yMax(0.5);
   // Show 5 occupied and virtual orbitals to start with
   unsigned index, nShow(5);
   
   if (nBeta > nAlpha) {
      // Use the beta energies instead
      index = nBeta < nShow ? 0 : nBeta-nShow;
      yMin  = orbitals.energy(Data::Beta, index);
      index = nOrbs > nBeta+nShow ? nBeta+nShow : nOrbs;
      yMax  = orbitals.energy(Data::Beta, index);
   }else {
      index = nAlpha < nShow ? 0 : nAlpha-nShow;
      yMin  = orbitals.energy(Data::Alpha, index);
      index = nOrbs > nAlpha+nShow ? nAlpha+nShow : nOrbs;
      yMax  = orbitals.energy(Data::Alpha, index);
   }

   yMax = std::min(yMax, 0.5*std::abs(yMin));
   m_moPlot->yAxis->setRange(1.05*yMin,1.05*yMax);

   // add the orbital label
   m_label = new QCPItemText(m_moPlot);
   m_moPlot->addItem(m_label);
   m_label->position->setType(QCPItemPosition::ptViewportRatio); 
   m_label->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
   m_label->position->setCoords(0, 0); 
   m_label->setText("Orbital");
   m_label->setFont(QFont(font().family(), 10));
}


void ExcitedStates::clearTransitionLines()
{
   QCPAbstractItem* item; 
   while (!m_transitionLines.isEmpty()) {
      item = m_transitionLines.takeLast();
      //m_moPlot->removeItem(item);
      delete item;
   }
}




void ExcitedStates::moSelectionChanged(bool tf)
{
   QCPGraph* graph(qobject_cast<QCPGraph*>(sender()));
   if (!graph) return;
   
   if (tf) {
      graph->setPen(m_selectedPen);
   }else {
      graph->setPen(m_pen);
      m_configurator.orbitalLabel->setText("");
      return;
   }

   bool ok;
   unsigned orb(graph->name().toUInt(&ok));

   Data::OrbitalSymmetries const& orbitals(m_excitedStates.stateData().orbitalSymmetries());

   unsigned nOrbs(orbitals.nOrbitals());
   if (!ok) return;

   double energy(0.0);
   QString label;
   if (orb < nOrbs) {  //alpha
      energy = orbitals.energy(Data::Alpha, orb);
      label  = "Alpha orbital ";
   }else {  // beta
      orb -= nOrbs;
      energy = orbitals.energy(Data::Beta, orb);
      label  = "Beta orbital ";
   }

   label += QString::number(orb+1);
   label += ": ";
   label += QString::number(energy, 'f', 3);
   label += " Eh";
   m_configurator.orbitalLabel->setText(label);
}


} } // end namespace IQmol::Configurator
