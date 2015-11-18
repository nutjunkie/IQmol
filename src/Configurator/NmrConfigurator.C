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

#include "ui_NmrConfigurator.h"
#include "NmrConfigurator.h"
#include "NmrReferenceLibrary.h"
#include "NmrLayer.h"
#include "NmrData.h"
#include "qcustomplot.h"
#include "QMsgBox.h"

/* TO DO

   clean up
   plot lorentzian
   reverse axis

*/


namespace IQmol {
namespace Configurator { 

Nmr::Nmr(Layer::Nmr& layer, Data::Nmr& data) : m_layer(layer), m_data(data), m_ui(0),
   m_plot(0)
{
   m_ui = new Ui::NmrConfigurator();
   m_ui->setupUi(this);

   QTableWidget* table(m_ui->shieldingsTable);
   table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   m_plot = new QCustomPlot();
   m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
   m_plot->axisRect()->setRangeDrag(m_plot->xAxis->orientation());
   m_plot->axisRect()->setRangeZoom(m_plot->xAxis->orientation());
   m_plot->xAxis->setSelectableParts(QCPAxis::spNone);

   m_plot->xAxis->setLabel("Nuclear Shielding (ppm)");
   //m_plot->yAxis->setLabel("Intensity");

   QFrame* frame(m_ui->spectrumFrame);
   QVBoxLayout* layout(new QVBoxLayout());
   frame->setLayout(layout);
   layout->addWidget(m_plot);

   m_ui->widthSlider->setEnabled(false);
   m_ui->widthLabel->setEnabled(false);

   m_pen.setColor(Qt::blue);
   m_pen.setStyle(Qt::SolidLine);
   m_pen.setWidthF(1);

   m_selectPen.setColor(Qt::red);
   m_selectPen.setStyle(Qt::SolidLine);
   m_selectPen.setWidthF(3);

   connect(this, SIGNAL(updated()), &m_layer, SIGNAL(updated()));

   initTable();
}


Nmr::~Nmr()
{
   if (m_plot) delete m_plot;
   if (m_ui) delete m_ui;
}


void Nmr::initTable()
{
   QList<QString> const& atomLabels(m_data.atomLabels());
   QList<double> shieldings(m_data.shieldings());

   if (atomLabels.size() != shieldings.size()) {
      QString msg("NMR data size mismatch: ");
      msg += QString::number(atomLabels.size()) + " != " 
           + QString::number(shieldings.size());
      QMsgBox::warning(0, "IQmol", msg);
      return;
   }

   QTableWidgetItem* item;

   QTableWidget* table(m_ui->shieldingsTable);
   table->setRowCount(atomLabels.size());
   
   for (int atom = 0; atom < atomLabels.size(); ++atom) {
       item = new QTableWidgetItem(atomLabels[atom]);
       item->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
       table->setItem(atom, 0, item);

       item = new QTableWidgetItem(QString::number(shieldings[atom], 'f', 2) + "     ");
       item->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
       table->setItem(atom, 1, item);

       item = new QTableWidgetItem();
       item->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
       table->setItem(atom, 2, item);
   }

   table->setCurrentCell(0, 0, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   on_shieldingsTable_itemSelectionChanged();
   //updatePlot();
}


void Nmr::loadShifts(Data::NmrReference const* reference, QString const& isotope)
{
   QList<QString> const& atomLabels(m_data.atomLabels());
   QTableWidget* table(m_ui->shieldingsTable);

   if (reference && reference->contains(isotope)) {
      QList<double> shieldings(m_data.shieldings());

      for (int row = 0; row < atomLabels.size(); ++row) {
          if (atomLabels[row] == isotope) {
             double shift(shieldings[row] - reference->shift(atomLabels[row]));
             table->item(row, 2)->setText(QString::number(shift, 'f', 2) + "     ");
          }else {
             table->item(row, 2)->setText("");
          }
      }
   }else {
      for (int row = 0; row < atomLabels.size(); ++row) {
          table->item(row, 2)->setText("");
      }
   }
}


// Note this only returns shifts for the given isotope
QList<double> Nmr::computeShifts(Data::NmrReference const* reference, QString const& isotope)
{
   QList<double> shifts;

   if (reference && reference->contains(isotope)) {
      QList<QString> const& atomLabels(m_data.atomLabels());
      QList<double> shieldings(m_data.shieldings());
      double shift(reference->shift(isotope));

      for (int i = 0; i < atomLabels.size(); ++i) {
          if (atomLabels[i] == isotope) shifts.append(shieldings[i] - shift);
      }
   }

   return shifts;
}


void Nmr::updatePlot()
{
   m_plot->clearGraphs();

   if (m_ui->impulseButton->isChecked()) {
      plotImpulse();
   }else if (m_ui->lorentzianButton->isChecked()) {
      plotSpectrum();
   }

   m_plot->replot();
}


void Nmr::plotImpulse()
{
   QVector<double> x(1), y(1);
   double maxIntensity(1.0);

   
   QList<double> data;
   QString isotope(currentIsotope());
qDebug() << "Plotting impulse for" << isotope;

   if (isotope.isEmpty()) {
      data = m_data.shieldings();
      m_plot->xAxis->setLabel("Nuclear Shielding (ppm)");
qDebug() << "  shieldings found" << data.size();
   }else {
      data = computeShifts(currentReference(), isotope);
      m_plot->xAxis->setLabel("Chemical Shifts (ppm)");
qDebug() << "  shifts found" << data.size();
   }

   qDebug() << "plotting shifts" << data.size();


   qDebug() << "---------------------------" ;
   m_data.dump();
   qDebug() << "---------------------------" ;

   for (int shift = 0; shift < data.size(); ++shift) {
       x[0] = data[shift];
       y[0] = maxIntensity; 

       QCPGraph* graph(m_plot->addGraph());
       graph->setData(x, y);
       graph->setName(QString::number(shift));
       graph->setLineStyle(QCPGraph::lsImpulse);
       //graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
       graph->setPen(m_pen);
       graph->setSelectedPen(m_selectPen);
       connect(graph, SIGNAL(selectionChanged(bool)), this, SLOT(plotSelectionChanged(bool)));
   }

   m_plot->xAxis->setRange(0, 200);
   m_plot->yAxis->setRange(-0.00, 1.05*maxIntensity);
   m_plot->yAxis->setAutoTickStep(true);
}



void Nmr::plotSpectrum() 
{
   double width(m_ui->widthSlider->value());
//qDebug() << "Plot spectrum called";
   unsigned const bins(400);
   unsigned const maxPpm(300);
   double   const delta(double(maxPpm)/bins);

   QVector<double> x(bins), y(bins);

   for (int xi = 0; xi < bins; ++xi) {
       x[xi] = xi*delta;
       y[xi] = 0.0;
   }

   double A(2.0/M_PI);
   double g(0.5*width);
   double g2(g*g);

   QList<double> shifts(m_data.shieldings());

   for (int mode = 0; mode < shifts.size(); ++mode) {

       double nu(shifts[mode]);
       double I(1.0);
       for (int xi = 0; xi < bins; ++xi) {
           y[xi] += I*A*g / (g2+(x[xi]-nu)*(x[xi]-nu));
       }
   }

   double maxIntensity(0.0);
   for (int xi = 0; xi < bins; ++xi) {
       if (y[xi] > maxIntensity) maxIntensity = y[xi];
   }

   for (int xi = 0; xi < bins; ++xi) {
       y[xi] /= maxIntensity;
   }

   QCPGraph* graph(m_plot->addGraph());
   graph->setData(x, y);
   graph->setPen(m_pen);
   graph->setAntialiased(true);
   graph->setSelectedPen(m_selectPen);

   m_plot->xAxis->setRange(0, 3500);
   m_plot->yAxis->setRange(0, 1.00);
   m_plot->yAxis->setAutoTickStep(false);
   m_plot->yAxis->setTickStep(0.2);
}


void Nmr::plotSelectionChanged(bool tf)
{
   qDebug() << "Plot selection changed called";

   QCPGraph* graph(qobject_cast<QCPGraph*>(sender()));
   if (!graph) return;

   if (tf) {
      graph->setPen(m_selectPen);
      //graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc));
   }else {
      graph->setPen(m_pen);
      //graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
      return;
   }

   if (!tf) return;
   if (!m_ui->impulseButton->isChecked()) return;

   bool ok;
   int atom(graph->name().toInt(&ok));
   if (!ok) return;

   QTableWidget* table(m_ui->shieldingsTable);
   table->setCurrentCell(atom, 0, 
       QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   table->scrollToItem(table->item(atom, 0));
   on_shieldingsTable_itemSelectionChanged();
}


void Nmr::on_shieldingsTable_itemSelectionChanged()
{
   if (!m_ui->impulseButton->isChecked()) return;

   QList<QTableWidgetItem*> selection = m_ui->shieldingsTable->selectedItems();
   if (selection.isEmpty()) return;
   int index(selection.first()->row());

   QCPGraph* graph(m_plot->graph(index));
   if (graph && graph->selected()) return;

   QList<QCPGraph*> selectedGraphs(m_plot->selectedGraphs());
   QList<QCPGraph*>::iterator iter;
   for (iter = selectedGraphs.begin(); iter != selectedGraphs.end(); ++iter) {
       (*iter)->setSelected(false);
   }

   if (graph) {
       graph->setSelected(true);
       m_plot->replot();
   }
}


void Nmr::on_isotopeCombo_currentIndexChanged(QString const&)
{
   QString isotope(currentIsotope());

   Data::NmrReferenceLibrary& library(Data::NmrReferenceLibrary::instance());
   QList<Data::NmrReference const*> refs(library.filter(isotope));

   QStringList systems;
   QList<Data::NmrReference const*>::iterator iter;
   for (iter = refs.begin(); iter != refs.end(); ++iter) {
       QString system((*iter)->system());
       qDebug() << "System:" <<  system;
       if (!systems.contains(system)) systems.append(system);
   }

   QComboBox* combo(m_ui->systemCombo);
   combo->clear();
   combo->addItems(systems);

   loadShifts(currentReference(), currentIsotope()); 

   updatePlot();
}


void Nmr::on_systemCombo_currentIndexChanged(QString const& text)
{
   QString isotope(currentIsotope());

   Data::NmrReferenceLibrary& library(Data::NmrReferenceLibrary::instance());
   QList<Data::NmrReference const*> refs(library.filter(isotope, text));

   qDebug() << "found" << refs.size() << "matches";

   QStringList methods;
   QList<Data::NmrReference const*>::iterator iter;
   for (iter = refs.begin(); iter != refs.end(); ++iter) {
       QString method((*iter)->method());
       if (!methods.contains(method)) methods.append(method);
   }

   QComboBox* combo(m_ui->methodCombo);
   combo->clear();
   combo->addItems(methods);
}


QString Nmr::currentIsotope()
{
   QString isotope;
   QString text(m_ui->isotopeCombo->currentText());

   // Match the isotope in, e.g. "Proton (1H)"
   QRegExp rx("\\(\\d+(\\D+)\\)");

   if (rx.indexIn(text) >= 0) {
      isotope = rx.cap(1);
   }else {
      qDebug() << "No references found for" << text;
   }

   return isotope;
}


Data::NmrReference const* Nmr::currentReference()
{
   Data::NmrReference const* reference;

   QString isotope(currentIsotope());
   QString system(m_ui->systemCombo->currentText());
   QString method(m_ui->methodCombo->currentText());

   Data::NmrReferenceLibrary& library(Data::NmrReferenceLibrary::instance());
   QList<Data::NmrReference const*> refs(library.filter(isotope, system, method));

   if (refs.size() == 1) {
      reference = refs.first();
qDebug() << "Found reference:";
      reference->dump();
   }else {
      reference = 0;
qDebug() << "No reference found for" << isotope << system << method;
   }

   return reference;
}

} } // end namespace IQmol::Configurator
