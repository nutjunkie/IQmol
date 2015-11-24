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
   export plots
   impulse selection should show atom highlighted in viewer (signal required)
*/

// Not pretty, but we use this to indicate when a shift should not be drawn
#define NO_SHIFT 99999


namespace IQmol {
namespace Configurator { 

Nmr::Nmr(Layer::Nmr& layer, Data::Nmr& data) : m_layer(layer), m_data(data), m_ui(0),
   m_plot(0)
{
   m_ui = new Ui::NmrConfigurator();
   m_ui->setupUi(this);
   m_ui->widthSlider->hide();
   //m_ui->widthSlider->setEnabled(false);
   //m_ui->widthLabel->setEnabled(false);

   QTableWidget* table(m_ui->shieldingsTable);
   table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   m_plot = new QCustomPlot();
   m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
   m_plot->axisRect()->setRangeDrag(m_plot->xAxis->orientation());
   m_plot->axisRect()->setRangeZoom(m_plot->xAxis->orientation());
   m_plot->xAxis->setSelectableParts(QCPAxis::spNone);
   m_plot->xAxis->setRangeReversed(true);

   QFrame* frame(m_ui->spectrumFrame);
   QVBoxLayout* layout(new QVBoxLayout());
   frame->setLayout(layout);
   layout->addWidget(m_plot);

   m_pen.setColor(Qt::blue);
   m_pen.setStyle(Qt::SolidLine);
   m_pen.setWidthF(1);

   m_selectPen.setColor(Qt::red);
   m_selectPen.setStyle(Qt::SolidLine);
   m_selectPen.setWidthF(3);

   connect(this, SIGNAL(updated()), &m_layer, SIGNAL(updated()));

   initTable();

   // This needs to occur after the table has been initialized
   Data::NmrReferenceLibrary& library(Data::NmrReferenceLibrary::instance());
   m_ui->isotopeCombo->clear();
   m_ui->isotopeCombo->addItem("Shieldings");
   m_ui->isotopeCombo->addItems(library.availableIsotopes());
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

       item = new QTableWidgetItem(QString::number(shieldings[atom], 'f', 2) + "    ");
       item->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
       table->setItem(atom, 1, item);

       item = new QTableWidgetItem();
       item->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
       table->setItem(atom, 2, item);
   }

   on_shieldingsTable_itemSelectionChanged();
   on_isotopeCombo_currentIndexChanged(m_ui->isotopeCombo->currentText());
}


void Nmr::loadShifts(Data::NmrReference const* reference, QString const& isotope)
{
   QList<QString> const& atomLabels(m_data.atomLabels());
   QTableWidget* table(m_ui->shieldingsTable);

   if (reference && reference->contains(isotope)) {
      QList<double> shieldings(m_data.shieldings());

      int count(0);
      for (int row = 0; row < atomLabels.size(); ++row) {
          if (atomLabels[row] == isotope) {
             double shift(reference->shift(atomLabels[row]) - shieldings[row]);
             table->item(row,2)->setText(QString::number(shift, 'f', 2) + "     ");
             ++count;
          }else {
             table->item(row,2)->setText("");
          }
      }
   }else {
      for (int row = 0; row < atomLabels.size(); ++row) {
          table->item(row,2)->setText("");
      }
   }
}


QList<double> Nmr::computeShifts(Data::NmrReference const* reference, QString const& isotope)
{
   QList<double> shifts;
   QList<QString> const& atomLabels(m_data.atomLabels());

   if (reference && reference->contains(isotope)) {
      QList<double> shieldings(m_data.shieldings());
      double shift(reference->shift(isotope));

      for (int i = 0; i < atomLabels.size(); ++i) {
          if (atomLabels[i] == isotope) {
             shifts.append(shift - shieldings[i]);
          }else {
             shifts.append(NO_SHIFT);
          }
      }
   }else {
      for (int i = 0; i < atomLabels.size(); ++i) {
          shifts.append(NO_SHIFT);
      }
   }

   qDebug() << shifts.size() << "Shifts computed";
   return shifts;
}


void Nmr::updatePlot()
{
   m_plot->clearGraphs();

   QList<double> data;
   QString isotope(currentIsotope());

   if (isotope.isEmpty()) {
      data = m_data.shieldings();
      m_plot->xAxis->setLabel("Nuclear Shielding (ppm)");
   }else {
      data = computeShifts(currentReference(), isotope);
      m_plot->xAxis->setLabel("Chemical Shift (ppm)");
   }

   if (m_ui->impulseButton->isChecked()) {
      plotImpulse(data, standardRange(isotope));
   }else if (m_ui->lorentzianButton->isChecked()) {
      plotSpectrum(data, standardRange(isotope));
   }

   m_plot->replot();
}


void Nmr::plotImpulse(QList<double> const& data, QPair<double, double> const& domain)
{
   // Signals that are within the resolution are considered to be the same.
   double width(m_ui->widthSlider->value()/20000.0);
   double resolution(width*(domain.second-domain.first));

   m_graphToRows.clear();

   for (int i = 0; i < data.size(); ++i) {
       double shift(data[i]);
       if (shift < NO_SHIFT-1) {
          int x(shift/resolution);
           //qDebug() << " incrementing " << x << "because of" << shift;
           m_graphToRows[x].append(i);
       }
   }

   qDebug() << "Signals map with resolution" << resolution << width;
   qDebug() << m_graphToRows;

   int range(1);
   QVector<double> x(1), y(1);
   QMap<int, QList<int> >::iterator iter;

   for (iter = m_graphToRows.begin(); iter != m_graphToRows.end(); ++iter) {
       x[0] = resolution*iter.key();
       y[0] = iter.value().size();
       range = std::max(range, iter.value().size());

       QCPGraph* graph(m_plot->addGraph());
       graph->setData(x, y);
       graph->setName(QString::number(iter.key()));
       graph->setLineStyle(QCPGraph::lsImpulse);
       graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
       graph->setPen(m_pen);
       graph->setSelectedPen(m_selectPen);
       connect(graph, SIGNAL(selectionChanged(bool)), this, SLOT(plotSelectionChanged(bool)));
   }

   m_plot->yAxis->setRange(-0.00, 1.05*range);
   m_plot->yAxis->setAutoTickStep(false);
   m_plot->yAxis->setTickStep(1);
   m_plot->yAxis->setLabel("Count");
}



void Nmr::plotSpectrum(QList<double> const& data, QPair<double, double> const& domain)
{
   double width(m_ui->widthSlider->value());
   width = 100;

   unsigned const bins(1000);
   unsigned const range(domain.second-domain.first);
   double   const delta(double(range)/bins);

   QVector<double> x(bins), y(bins);

   for (int xi = 0; xi < bins; ++xi) {
       x[xi] = domain.first + xi*delta;
       y[xi] = 0.0;
   }

   double A(1.0/M_PI);
   double g(0.01*width*delta);
   double g2(g*g);

   qDebug() << "Plot spectrum called between" << domain.first 
            << "and" << domain.second << " delta =" << delta
            << "width = " << width << " g = " << g;

   QList<double> shifts(m_data.shieldings());

   for (int mode = 0; mode < data.size(); ++mode) {
       double nu(data[mode]);
       for (int xi = 0; xi < bins; ++xi) {
           y[xi] += A*g / (g2+(x[xi]-nu)*(x[xi]-nu));
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

   m_plot->yAxis->setRange(-0.00, 1.05);
   m_plot->yAxis->setLabel("Relative Intensity");
   //m_plot->yAxis->setAutoTickStep(false);
   //m_plot->yAxis->setTickStep(0.2);
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
       if (!systems.contains(system)) systems.append(system);
   }

   QComboBox* combo(m_ui->systemCombo);
   combo->clear();
   combo->addItems(systems);

   loadShifts(currentReference(), isotope);

   QPair<double, double> range(standardRange(isotope));
   m_plot->xAxis->setRange(range.first, range.second);

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

   loadShifts(currentReference(), isotope);
   updatePlot();
}

void Nmr::on_methodCombo_currentIndexChanged(QString const& text)
{
   loadShifts(currentReference(), currentIsotope());
   updatePlot();
}


void Nmr::on_impulseButton_clicked(bool)
{
   //m_ui->widthSlider->setEnabled(false);
   //m_ui->widthLabel->setEnabled(false);
   updatePlot();
}


void Nmr::on_lorentzianButton_clicked(bool)
{
   //m_ui->widthSlider->setEnabled(true);
   //m_ui->widthLabel->setEnabled(true);
   updatePlot();
}


void Nmr::on_widthSlider_valueChanged(int value)
{
   updatePlot();
}


void Nmr::plotSelectionChanged(bool tf)
{
   QCPGraph* graph(qobject_cast<QCPGraph*>(sender()));
   if (!graph) return;
   //qDebug() << "Plot selection changed called for" << graph->name();

   if (tf) {
      graph->setPen(m_selectPen);
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc));
   }else {
      graph->setPen(m_pen);
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
      return;
   }

   if (!tf) return;
   if (!m_ui->impulseButton->isChecked()) return;

   bool ok;
   int sig(graph->name().toInt(&ok));
   if (!ok) return;

   QList<int> rows(m_graphToRows.value(sig));

   QTableWidget* table(m_ui->shieldingsTable);
   QItemSelectionModel *selectionModel(table->selectionModel());
   selectionModel->clearSelection();
   QItemSelection itemSelection;

   for (int i = 0; i < rows.size(); ++i) {
       table->selectRow(rows[i]);
       itemSelection.merge(selectionModel->selection(), QItemSelectionModel::Select);
   }
   selectionModel->select(itemSelection, QItemSelectionModel::Select);
}


void Nmr::on_shieldingsTable_itemSelectionChanged()
{
   QList<QTableWidgetItem*> selection(m_ui->shieldingsTable->selectedItems());
//qDebug() << "on_shieldingsTable_itemSelectionChanged called with slection"
//         << selection.size();
   if (selection.isEmpty()) return;
   int row(selection.last()->row());


   if (!m_ui->impulseButton->isChecked()) return;

   // First find the name of the graph
   QString name;
   QMap<int, QList<int> >::iterator iter;
   for (iter = m_graphToRows.begin(); iter != m_graphToRows.end(); ++iter) {
       if (iter.value().contains(row)) {
          //qDebug() << "row selected" << row << "found in" << iter.key();
          //qDebug() << "Selected rows: " << iter.value();
          selectAtoms(iter.value());
          name = QString::number(iter.key());
          break;
       } 
   }
   if (name.isEmpty()) return;

   // now find the graph
   int index;
   for (index = 0; index < m_plot->graphCount(); ++index) {
       if (m_plot->graph(index)->name() == name) break;
   }
   if (index == m_plot->graphCount()) return; 

   QCPGraph* graph(m_plot->graph(index));
   if (graph && graph->selected()) return;

   QList<QCPGraph*> selectedGraphs(m_plot->selectedGraphs());
   QList<QCPGraph*>::iterator it;
   for (it = selectedGraphs.begin(); it != selectedGraphs.end(); ++it) {
       (*it)->setSelected(false);
   }

   if (graph) graph->setSelected(true);
   m_plot->replot();
}



QPair<double, double> Nmr::standardRange(QString const& isotope)
{
   double min(0.0), max(0.0);

   if (isotope == "H") {
      min =   0.0;
      max =  10.0;
   }else if (isotope == "B") {
      min = -130.0;
      max =   65.0;
   }else if (isotope == "C") {
      min =    0.0;
      max =  220.0;
   }else if (isotope == "N") {
      min =    0.0;
      max =  900.0;
   }else if (isotope == "F") {
      min = -280.0;
      max =  280.0;
   }else if (isotope == "Si") {
      min = -380.0;
      max =   80.0;
   }else if (isotope == "P") {
      min = -480.0;
      max =  270.0;
   }else {
      QList<double> const& data(m_data.shieldings());
      min = max = data.first();
      for (int i = 1; i < data.size(); ++i) {
          min = std::min(min, data[i]);
          max = std::max(max, data[i]);
      }
      min *= min < 0 ? 1.10 : 0.90;
      max *= max > 0 ? 1.10 : 0.90;
   }

   return qMakePair(min,max);
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
