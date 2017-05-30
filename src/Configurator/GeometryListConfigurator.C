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

#include "MoleculeLayer.h"
#include "GeometryLayer.h"
#include "GeometryListLayer.h"
#include "GeometryListConfigurator.h"
#include "Constraint.h"
#include <QHeaderView>

#include "CustomPlot.h"


using namespace qglviewer;

namespace IQmol {
namespace Configurator {


GeometryList::GeometryList(Layer::GeometryList& geometryList) : m_geometryList(geometryList),
   m_customPlot(0)
{
   m_configurator.setupUi(this);
   m_configurator.energyTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   m_customPlot = new CustomPlot();

   m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
   m_customPlot->xAxis->setSelectableParts(QCPAxis::spNone);
   m_customPlot->xAxis->setLabel("Geometry");
   m_customPlot->yAxis->setLabel("Energy");
   
   QFrame* frame(m_configurator.plotFrame);
   QVBoxLayout* layout(new QVBoxLayout());
   frame->setLayout(layout);
   layout->addWidget(m_customPlot);
   
   m_pen.setColor(Qt::blue);
   m_pen.setStyle(Qt::SolidLine);
   m_pen.setWidthF(1);
   
   m_selectPen.setColor(Qt::red);
   m_selectPen.setStyle(Qt::SolidLine);
   m_selectPen.setWidthF(3);
}


GeometryList::~GeometryList()
{
   if (m_customPlot) delete m_customPlot;
}


void GeometryList::load()
{
   QTableWidget* table(m_configurator.energyTable);
   QList<Layer::Geometry*> 
      geometries(m_geometryList.findLayers<Layer::Geometry>(Layer::Children));
   table->setRowCount(geometries.size());

   QTableWidgetItem* energy;

   if (geometries.size() < 2) {
      m_configurator.playButton->setEnabled(false);
      m_configurator.forwardButton->setEnabled(false);
      m_configurator.backButton->setEnabled(false);
      m_configurator.bounceButton->setEnabled(false);
      m_configurator.updateBondsButton->setEnabled(false);
      m_configurator.speedSlider->setEnabled(false);
      m_configurator.speedLabel->setEnabled(false);
      return;      
   }

   int row(0);
   bool property(false);
   QList<Layer::Geometry*>::iterator iter;
   for (iter = geometries.begin(); iter != geometries.end(); ++iter, ++row) {
       energy = new QTableWidgetItem( (*iter)->text() );
       energy->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
       table->setItem(row, 0, energy);
       double e((*iter)->energy());
       double x(row);
       Data::Geometry& geom((*iter)->data());

       if (geom.hasProperty<Data::Constraint>()) {
          x = geom.getProperty<Data::Constraint>().value();
          property = true;
       }

       m_rawData.append(qMakePair(x, e));
   }

   if (property) {
      m_customPlot->xAxis->setLabel("Geometric Parameter");
   }else {
   m_customPlot->xAxis->setLabel("Geometry");
   }

   plotEnergies();
}


void GeometryList::plotEnergies()
{
   m_customPlot->clearGraphs();

   double xmax(m_rawData.first().first),  xmin(m_rawData.first().first);
   double ymax(m_rawData.first().second), ymin(m_rawData.first().second);
   QVector<double> xx(m_rawData.size()), yy(m_rawData.size());

   for (int i = 0; i < m_rawData.size(); ++i) {
       xx[i] = m_rawData[i].first;
       yy[i] = m_rawData[i].second;
       xmin = std::min(xmin, xx[i]);
       xmax = std::max(xmax, xx[i]);
       ymin = std::min(ymin, yy[i]);
       ymax = std::max(ymax, yy[i]);
   }

   QCPGraph* graph(m_customPlot->addGraph());
   graph->setData(xx, yy);
   graph->setPen(m_pen);
   graph->setSelectable(false);
   graph->setLineStyle(QCPGraph::lsLine);

   QVector<double> x(1), y(1);
   for (int geom = 0; geom < m_rawData.size(); ++geom) {
       x[0] = m_rawData[geom].first;
       y[0] = m_rawData[geom].second;

       QCPGraph* graph(m_customPlot->addGraph());
       graph->setData(x, y);
       graph->setName(QString::number(geom));
       graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
       graph->setPen(m_pen);
       graph->setSelectedPen(m_selectPen);
       connect(graph, SIGNAL(selectionChanged(bool)), this, SLOT(plotSelectionChanged(bool)));
   }

   m_customPlot->xAxis->setRange(0, m_rawData.size());
   m_customPlot->yAxis->setRange(ymin, ymax);
   m_customPlot->xAxis->setRange(xmin, xmax);
   m_customPlot->yAxis->setAutoTickStep(true);
   m_customPlot->replot();
}


void GeometryList::plotSelectionChanged(bool tf)
{
   QCPGraph* graph(qobject_cast<QCPGraph*>(sender()));
   if (!graph) return;

   if (tf) {
      graph->setPen(m_selectPen);
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc));
   }else {
      graph->setPen(m_pen);
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
      return;
   }   
       
   if (!tf) return;
       
   bool ok;
   int geom(graph->name().toInt(&ok));
   if (!ok) return;

   QTableWidget* table(m_configurator.energyTable);
   table->setCurrentCell(geom, 0, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   table->scrollToItem(table->item(geom,0));
   on_energyTable_itemSelectionChanged();
}


void GeometryList::reset()
{
   m_configurator.playButton->setChecked(false);
   m_configurator.backButton->setEnabled(true);
   m_configurator.forwardButton->setEnabled(true);
}


void GeometryList::on_playButton_clicked(bool play)
{
   if (play) {
      QTableWidget* table(m_configurator.energyTable);
      table->setCurrentCell(-1, 0,  // clear the current selection 
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }

   m_configurator.backButton->setEnabled(!play);
   m_configurator.forwardButton->setEnabled(!play);
   m_geometryList.setPlay(play);
}


void GeometryList::on_backButton_clicked(bool)
{
   QTableWidget* table(m_configurator.energyTable);
   int currentRow(table->currentRow());
   if (currentRow > 0) {
      table->setCurrentCell(currentRow-1, 0, 
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void GeometryList::on_forwardButton_clicked(bool)
{
   QTableWidget* table(m_configurator.energyTable);
   int currentRow(table->currentRow());
   if (currentRow < table->rowCount()-1) {
      table->setCurrentCell(currentRow+1, 0, 
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void GeometryList::on_speedSlider_valueChanged(int value)
{
   // Default is 25 => 0.125 in GeometryList ctor
   m_geometryList.setSpeed(value/200.0);
}


void GeometryList::on_energyTable_itemSelectionChanged()
{
   QList<QTableWidgetItem*> selection = m_configurator.energyTable->selectedItems();
   if (selection.isEmpty()) return;

   int index(selection.first()->row());
   if (index > -1) m_geometryList.setCurrentGeometry(index);

   QCPGraph* graph(m_customPlot->graph(index+1));
   if (graph && graph->selected()) return;
         
   QList<QCPGraph*> selectedGraphs(m_customPlot->selectedGraphs());
   QList<QCPGraph*>::iterator iter;
   for (iter = selectedGraphs.begin(); iter != selectedGraphs.end(); ++iter) {
       (*iter)->setSelected(false);
   }

   if (graph) {
       graph->setSelected(true);
       m_customPlot->replot();
   }
}


void GeometryList::on_bounceButton_clicked(bool tf)
{
   m_geometryList.setBounce(tf);
}


void GeometryList::on_loopButton_clicked(bool tf)
{
   m_geometryList.setLoop(tf);
}


void GeometryList::on_updateBondsButton_clicked(bool tf)
{
   m_geometryList.setReperceiveBonds(tf);
   on_energyTable_itemSelectionChanged(); 
}


void GeometryList::closeEvent(QCloseEvent* e)
{
   on_playButton_clicked(false);
   m_geometryList.resetGeometry();
   Base::closeEvent(e);
}

} } // end namespace IQmol::Configurator
