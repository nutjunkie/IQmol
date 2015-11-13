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
#include "NmrLayer.h"
#include "NmrData.h"
#include "qcustomplot.h"
#include "QMsgBox.h"


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

   m_plot->xAxis->setLabel("Shift (ppm)");
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

   load();
}


Nmr::~Nmr()
{
   if (m_plot) delete m_plot;
   if (m_ui) delete m_ui;

   QList<Data::NmrReference*>::iterator iter;
   for (iter = m_references.begin(); iter != m_references.end(); ++iter) {
       delete (*iter);
   }
}


void Nmr::load()
{
   QList<QString> const& atomLabels(m_data.atomLabels());

   QTableWidget* table(m_ui->shieldingsTable);
   table->setRowCount(atomLabels.size());

   QList<double> shifts;
   if (m_data.haveRelativeShifts()) {
      shifts = m_data.relativeShifts();
   }else {
      loadReferences();
      shifts = m_data.isotropicShifts();
   }

   if (atomLabels.size() != shifts.size()) {
      QMsgBox::warning(0, "IQmol", "NMR data size mismatch");
      return;
   }

   QTableWidgetItem* label;
   QTableWidgetItem* shift;
   
   for (int atom = 0; atom < atomLabels.size(); ++atom) {
       label = new QTableWidgetItem(atomLabels[atom]);
       shift = new QTableWidgetItem(QString::number(shifts[atom], 'f', 2) + "     ");
       label->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
       shift->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
       table->setItem(atom, 0, label);
       table->setItem(atom, 1, shift);
   }

   table->setCurrentCell(0, 0, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   on_shieldingsTable_itemSelectionChanged();
   updatePlot();
}


void Nmr::loadReferences()
{
   QComboBox* combo(m_ui->referenceCombo);
   QString name;

   name = "None";
   combo->addItem(name);

   name = "TMS HF/6-31G(d)";
   combo->addItem(name);

   name = "TMS B3LYP/6-31G(d,p)";
   combo->addItem(name);

   name = "TMS B3LYP/6-31G(d,p)";
   combo->addItem(name);

   name = "DMSO B3LYP/6-31G(d,p)";
}


/*
void Nmr::on_widthSlider_valueChanged(int)
{
   updatePlot();
}


void Frequencies::on_referenceCombo_indexChanged(int index)
{
   QTableWidget* table(m_ui.frequencyTable);
   for (int mode = 0; mode < m_rawData.size(); ++mode) {
       table->item(mode, 0)->setText(QString::number(m_rawData[mode].first * scale,'f', 2));
   }
   
   updatePlot();
}


void Nmr::on_impulseButton_clicked(bool)
{
   m_ui.widthSlider->setEnabled(false);
   m_ui.widthLabel->setEnabled(false);
   updatePlot();
}


void Nmr::on_gaussianButton_clicked(bool)
{
   m_ui.widthSlider->setEnabled(true);
   m_ui.widthLabel->setEnabled(true);
   updatePlot();
}


void Nmr::on_lorentzianButton_clicked(bool)
{
   m_ui.widthSlider->setEnabled(true);
   m_ui.widthLabel->setEnabled(true);
   updatePlot();
}

*/


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

   QList<double> shifts(m_data.isotropicShifts());

   for (int shift = 0; shift < shifts.size(); ++shift) {
       x[0] = shifts[shift];
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

   QList<double> const& shifts(m_data.isotropicShifts());

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


void Nmr::on_typeCombo_currentIndexChanged(QString const& text)
{
   qDebug() << "Setting spectrum type to " << text;
}

} } // end namespace IQmol::Configurator
