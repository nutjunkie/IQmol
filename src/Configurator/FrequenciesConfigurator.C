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

#include "FrequenciesConfigurator.h"
#include "QVariantPointer.h"
#include "FrequenciesLayer.h"
#include "VibrationalMode.h"
#include "Preferences.h"
#include "AtomLayer.h"
#include <QColorDialog>
#include <QHeaderView>
#include "QsLog.h"
#include <cmath>

#include "CustomPlot.h"


namespace IQmol {
namespace Configurator { 

Frequencies::Frequencies(Layer::Frequencies& frequencies) : m_frequencies(frequencies), 
   m_customPlot(0)
{
   m_configurator.setupUi(this);
   if (m_frequencies.haveRaman()) {
      m_configurator.frequencyTable->setColumnWidth(0,60);
      m_configurator.frequencyTable->horizontalHeaderItem(0)->setText("Freq. (cm⁻¹)");
      m_configurator.frequencyTable->horizontalHeaderItem(1)->setText("Intens. (km/mol)");
      m_configurator.frequencyTable->horizontalHeaderItem(2)->setText("Raman (Å4/amu)");
   }else {
      m_configurator.frequencyTable->setColumnCount(2);
      m_configurator.frequencyTable->horizontalHeaderItem(0)->setText("Frequency (cm⁻¹)");
      m_configurator.frequencyTable->horizontalHeaderItem(1)->setText("Intensity (km/mol)");
      m_configurator.ramanCheckbox->hide();
   }

   m_configurator.frequencyTable->horizontalHeader()
#if QT_VERSION >= 0x050000
     ->setSectionResizeMode(QHeaderView::Stretch);
#else
      ->setResizeMode(QHeaderView::Stretch);
#endif

   connect(this, SIGNAL(update()), &m_frequencies, SIGNAL(update()));

   setVectorColor(Preferences::VibrationVectorColor());

   m_customPlot = new CustomPlot();
   m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
   m_customPlot->axisRect()->setRangeDrag(m_customPlot->xAxis->orientation());
   m_customPlot->axisRect()->setRangeZoom(m_customPlot->xAxis->orientation());
   m_customPlot->xAxis->setSelectableParts(QCPAxis::spNone);
   m_customPlot->xAxis->setLabel("Frequency");
   m_customPlot->yAxis->setLabel("Intensity");

   QFrame* frame(m_configurator.spectrumFrame);
   QVBoxLayout* layout(new QVBoxLayout());
   frame->setLayout(layout);
   layout->addWidget(m_customPlot);

   m_configurator.widthSlider->setEnabled(false);
   m_configurator.widthLabel->setEnabled(false);

   m_pen.setColor(Qt::blue);
   m_pen.setStyle(Qt::SolidLine);
   m_pen.setWidthF(1);

   m_selectPen.setColor(Qt::red);
   m_selectPen.setStyle(Qt::SolidLine);
   m_selectPen.setWidthF(3);
}


Frequencies::~Frequencies()
{
   if (m_customPlot) delete m_customPlot;
}


void Frequencies::load()
{
   QTableWidget* table(m_configurator.frequencyTable);
   QList<Layer::Mode*> modes(m_frequencies.findLayers<Layer::Mode>(Layer::Children));
   table->setRowCount(modes.size());

   QTableWidgetItem* frequency;
   QTableWidgetItem* intensity;
   QTableWidgetItem* raman;
   
   int row(0);
   QList<Layer::Mode*>::iterator iter;
   for (iter = modes.begin(); iter != modes.end(); ++iter, ++row) {

       QString text((*iter)->text());
       frequency = new QTableWidgetItem(text + "     ");
       frequency->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
       frequency->setData(Qt::UserRole, QVariantPointer<Layer::Mode>::toQVariant(*iter));

       text = QString::number((*iter)->modeData().intensity(), 'f', 3);
       intensity = new QTableWidgetItem(text + "     ");
       intensity->setData(Qt::UserRole, QVariantPointer<Layer::Mode>::toQVariant(*iter));
       intensity->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

       table->setItem(row, 0, frequency);
       table->setItem(row, 1, intensity);

       if (m_frequencies.haveRaman()) {
          text = QString::number((*iter)->modeData().ramanIntensity(), 'f', 3);
          raman = new QTableWidgetItem(text + "     ");
          raman->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
          table->setItem(row, 2, raman);
       }

       m_rawIrData.append(qMakePair((*iter)->modeData().frequency(),
          (*iter)->modeData().intensity()));
       m_rawRamanData.append(qMakePair((*iter)->modeData().frequency(),
          (*iter)->modeData().ramanIntensity()));

   }

   m_rawData = m_rawIrData;

   table->setCurrentCell(0, 0, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   on_frequencyTable_itemSelectionChanged();
   updatePlot();
}


/*
void Frequencies::contextMenuEvent(QContextMenuEvent* event)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);
  menu->addAction("Save PDF", this, SLOT(savePDF()));
  menu->popup(m_customPlot->mapToGlobal(event->pos()));
}
*/



void Frequencies::on_widthSlider_valueChanged(int)
{
   updatePlot();
}


void Frequencies::on_ramanCheckbox_clicked(bool tf)
{
   m_rawData = tf ? m_rawRamanData : m_rawIrData;
   updatePlot();
}


void Frequencies::on_scaleFactor_valueChanged(double scale)
{
   QTableWidget* table(m_configurator.frequencyTable);
   for (int mode = 0; mode < m_rawData.size(); ++mode) {
       table->item(mode, 0)->setText(QString::number(m_rawData[mode].first * scale,'f', 2));
   }
   
   updatePlot();
}


void Frequencies::on_impulseButton_clicked(bool)
{
   m_configurator.widthSlider->setEnabled(false);
   m_configurator.widthLabel->setEnabled(false);
   updatePlot();
}


void Frequencies::on_gaussianButton_clicked(bool)
{
   m_configurator.widthSlider->setEnabled(true);
   m_configurator.widthLabel->setEnabled(true);
   updatePlot();
}


void Frequencies::on_lorentzianButton_clicked(bool)
{
   m_configurator.widthSlider->setEnabled(true);
   m_configurator.widthLabel->setEnabled(true);
   updatePlot();
}


void Frequencies::updatePlot()
{
   m_customPlot->clearGraphs();
   double scale(m_configurator.scaleFactor->value());
   double width(m_configurator.widthSlider->value());

   if (m_configurator.impulseButton->isChecked()) {
      plotImpulse(scale);
      m_customPlot->yAxis->setLabel("Intensity");

   }else if (m_configurator.gaussianButton->isChecked()) {
      plotSpectrum(Gaussian, scale, width);
      m_customPlot->yAxis->setLabel("Rel. Intensity");

   }else if (m_configurator.lorentzianButton->isChecked()) {
      plotSpectrum(Lorentzian, scale, width);
      m_customPlot->yAxis->setLabel("Rel. Intensity");

   }

   m_customPlot->replot();
}


void Frequencies::plotImpulse(double const scaleFactor)
{
   QVector<double> x(1), y(1);
   double maxIntensity;

   if (m_configurator.ramanCheckbox->isChecked()) {
      maxIntensity = (m_frequencies.maxRamanIntensity());
   } else {
      maxIntensity = (m_frequencies.maxIntensity());
   }

   for (int mode = 0; mode < m_rawData.size(); ++mode) {
       x[0] = scaleFactor * m_rawData[mode].first;
       y[0] = m_rawData[mode].second;

       QCPGraph* graph(m_customPlot->addGraph());
       graph->setData(x, y);
       graph->setName(QString::number(mode));
       graph->setLineStyle(QCPGraph::lsImpulse);
       graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
       graph->setPen(m_pen);
       graph->setSelectedPen(m_selectPen);
       connect(graph, SIGNAL(selectionChanged(bool)), this, SLOT(plotSelectionChanged(bool)));
   }

   m_customPlot->xAxis->setRange(0, 4000);
   m_customPlot->yAxis->setRange(-0.00*maxIntensity, 1.05*maxIntensity);
   m_customPlot->yAxis->setAutoTickStep(true);
}



void Frequencies::plotSpectrum(Profile const profile, double const scaleFactor, double const width) 
{
//qDebug() << "Plot spectrum called";
   unsigned const bins(400);
   unsigned const maxWavenumber(4000);
   double   const delta(double(maxWavenumber)/bins);

   QVector<double> x(bins), y(bins);

   for (unsigned xi = 0; xi < bins; ++xi) {
       x[xi] = xi*delta;
       y[xi] = 0.0;
   }

   switch (profile) {

      case Gaussian: {
         double g(20*width);
         double A(std::sqrt(4.0*std::log(2.0)/(g*M_PI)));
         double a(-4.0*std::log(2.0)/g);
         for (int mode = 0; mode < m_rawData.size(); ++mode) {
             double nu(scaleFactor*m_rawData[mode].first);
             double I(m_rawData[mode].second);
             for (unsigned xi = 0; xi < bins; ++xi) {
                 y[xi] += I*A*std::exp(a*(x[xi]-nu)*(x[xi]-nu));
             }
         }
      } break;

      case Lorentzian: {
         double A(2.0/M_PI);
         double g(0.5*width);
         double g2(g*g);
         for (int mode = 0; mode < m_rawData.size(); ++mode) {
             double nu(scaleFactor*m_rawData[mode].first);
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

   QCPGraph* graph(m_customPlot->addGraph());
   graph->setData(x, y);
   graph->setPen(m_pen);
   graph->setAntialiased(true);
   graph->setSelectedPen(m_selectPen);

   m_customPlot->xAxis->setRange(0, 3500);
   m_customPlot->yAxis->setRange(0, 1.00);
   m_customPlot->yAxis->setAutoTickStep(false);
   m_customPlot->yAxis->setTickStep(0.2);
}


void Frequencies::plotSelectionChanged(bool tf)
{
//qDebug() << "Plot selection changed called";
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
   if (!m_configurator.impulseButton->isChecked()) return;

   bool ok;
   int mode(graph->name().toInt(&ok));
   if (!ok) return;

   QTableWidget* table(m_configurator.frequencyTable);
   table->setCurrentCell(mode, 0, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   table->scrollToItem(table->item(mode,0));
   on_frequencyTable_itemSelectionChanged();
}


void Frequencies::reset()
{
   m_configurator.playButton->setChecked(false);
   m_frequencies.setPlay(false);
   on_frequencyTable_itemSelectionChanged();
}


void Frequencies::on_playButton_clicked(bool play)
{
   m_frequencies.setPlay(play);
}


void Frequencies::on_backButton_clicked(bool)
{
   QTableWidget* table(m_configurator.frequencyTable);
   int currentRow(table->currentRow());
   if (currentRow > 0) {
      table->setCurrentCell(currentRow-1, 0,
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void Frequencies::on_forwardButton_clicked(bool)
{
   QTableWidget* table(m_configurator.frequencyTable);
   int currentRow(table->currentRow());
   if (currentRow < table->rowCount()-1) {
      table->setCurrentCell(currentRow+1, 0,
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void Frequencies::on_colorButton_clicked(bool)
{
   QColor vectorColor(Preferences::VibrationVectorColor());
   setVectorColor(QColorDialog::getColor(vectorColor, this));
}


void Frequencies::on_loopButton_clicked(bool tf)
{
   m_frequencies.setLoop(tf);
   on_frequencyTable_itemSelectionChanged();
}


void Frequencies::setVectorColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_configurator.colorButton->setStyleSheet(bg);
      Layer::Atom::setVibrationVectorColor(color);
      Preferences::VibrationVectorColor(color);
      update();
   }
}


void Frequencies::on_amplitudeSlider_valueChanged(int value)
{
   m_frequencies.setScale(value/100.0);
}


void Frequencies::on_speedSlider_valueChanged(int value)
{
   // Default is 25 => 0.0625 in Frequencies ctor
   m_frequencies.setSpeed(value/400.0);
}


void Frequencies::on_frequencyTable_itemDoubleClicked(QTableWidgetItem* item)
{
   Layer::Mode* mode = QVariantPointer<Layer::Mode>::toPointer(item->data(Qt::UserRole));
   if (mode) {
      m_frequencies.setActiveMode(*mode);
      m_frequencies.setPlay(true);
      m_configurator.playButton->setChecked(true);
   }
}


void Frequencies::on_frequencyTable_itemSelectionChanged()
{
   QList<QTableWidgetItem*> selection(m_configurator.frequencyTable->selectedItems());
   if (selection.isEmpty()) return;

   Layer::Mode* mode = QVariantPointer<Layer::Mode>::toPointer(
       selection.first()->data(Qt::UserRole));

   if (mode) {
      m_frequencies.setActiveMode(*mode);
      m_frequencies.setPlay();
   }

   if (!m_configurator.impulseButton->isChecked()) return;

   int index(selection.first()->row());
   QCPGraph* graph(m_customPlot->graph(index));
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


void Frequencies::closeEvent(QCloseEvent* e)
{
   m_frequencies.clearActiveMode();
   m_frequencies.setPlay(false);
   Base::closeEvent(e);
}

} } // end namespace IQmol::Configurator
