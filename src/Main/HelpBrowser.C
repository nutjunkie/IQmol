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

#include "HelpBrowser.h"
#include "QsLog.h"
#include <QApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <cmath>

#include <QtDebug>


namespace IQmol {

HelpBrowser::HelpBrowser(QWidget* parent) : QDialog(parent)
{
   m_helpBrowser.setupUi(this);
   m_helpBrowser.textBrowser->setOpenExternalLinks(true);
   connect(m_helpBrowser.textBrowser, SIGNAL(sourceChanged(QUrl const&)),
      this, SLOT(checkUrlForSearch(QUrl const&)));

   loadImages();
   loadHelpFiles();
   setStyleSheet();

   loadPage("index.html");
   m_searchResults = "";
}


void HelpBrowser::setStyleSheet()
{
   QString css;
   css  = "h1 {"
          "   color:#909090;"
          "   background-image: url('header.png') top repeat-x;"
          "}";

   css += "h3 {"
          "   color:#666666;"
          "}";

   css += "a {"
          "   color:#4488bb;"
          "   text-decoration:none;"
          "}";

   css += "html, body {"
          "   color: #777777;"
          "   background: #f1f1f1;"
          "}";

   css += "p {"
          "   margin-left: 10px;"
          "   margin-right: 10px;"
          "}";

   css += "p {"
          "   margin-right: 10px;"
          "}";



   QTextDocument* document(m_helpBrowser.textBrowser->document());
   document->addResource(QTextDocument::StyleSheetResource, QUrl("style.css"), css);
}


void HelpBrowser::loadImages()
{
   QTextDocument* document(m_helpBrowser.textBrowser->document());
   document->addResource(QTextDocument::ImageResource, 
      QUrl("header.png"), QPixmap(":/help/images/header.png"));
}


void HelpBrowser::loadPage(QString const& fileName)
{
   QString src("qrc:///help/");
   src += fileName;
   m_helpBrowser.textBrowser->setSource(QUrl(src));
}


void HelpBrowser::on_homeButton_clicked(bool)
{
   loadPage("index.html");
}


void HelpBrowser::on_forwardButton_clicked(bool)
{
   m_helpBrowser.textBrowser->forward(); 
   QString url(m_helpBrowser.textBrowser->source().toString()); 
   if (url.contains("search.html")) {
      m_helpBrowser.textBrowser->setHtml(m_searchResults);
   }
}


void HelpBrowser::on_backButton_clicked(bool)
{
   m_helpBrowser.textBrowser->backward(); 
   QString url(m_helpBrowser.textBrowser->source().toString()); 
   if (url.contains("search.html")) {
      m_helpBrowser.textBrowser->setHtml(m_searchResults);
   }
}


QString HelpBrowser::readFile(QString const& fileName)
{
   QString src(":/help/");
   src += fileName;
   QString contents;

   QFile file(src);
   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      contents = file.readAll();
      file.close();
   }else {
      QLOG_ERROR() << "HelpBrowser: could not read contents of file:" << src;
   }

   return contents;
}


void HelpBrowser::loadHelpFiles()
{
   QStringList files;
   files << "surfaces.html"
         << "overview.html"
         << "toolbar.html"
         << "modes.html"
         << "modelview.html"
         << "credits.html"
         << "layers.html"
         << "calculations.html"
         << "paths.html"
         << "vibrations.html";

   QStringList::iterator iter;
   QRegExp htmlTags("<[^>]*>");
   for (iter = files.begin(); iter != files.end(); ++iter) {
       m_fileContents.insert(*iter, readFile(*iter).remove(htmlTags));
   }
}


void HelpBrowser::on_searchButton_clicked(bool)
{
   QString s(m_helpBrowser.searchLineEdit->text());
   if (!s.isEmpty()) {
      m_searchResults = createSearchResults(s);
      m_helpBrowser.textBrowser->setSource(QUrl("search.html"));
      m_helpBrowser.textBrowser->setHtml(m_searchResults);
   }
}


QString HelpBrowser::createSearchResults(QString const& s)
{
   int const hitLength(50); 
   QString results;
   results  = "<html>"
              "<head><link href='style.css' rel='stylesheet' type='text/css' /></head>"
              "<body>"
              "<center><h1>Search Results<br></h1></center>";

   QString contents, list;
   int index, start, finish;
   QMap<QString, QString>::iterator iter;
   for (iter = m_fileContents.begin(); iter != m_fileContents.end(); ++iter) {
       index = 0;
       contents = iter.value();
       QStringList hits;

       while (index >= 0) {
          index = contents.indexOf(s, index, Qt::CaseInsensitive);
          if (index >= 0) {
             start  = std::max(0,index-hitLength/2);
             finish = std::min(start+hitLength+s.size(), contents.size());
             hits.append(contents.mid(start, finish-start));
             ++index;
          }
       }

       if (!hits.isEmpty()) {
          QString file(iter.key());
		  // This is a bit of a hack.  We append 'searchString; to the path so
		  // that when the link is activated the loadPage() function can advance to
          // the first occurence of the search.
          list += "<li><a href='" + file + "#searchString'>" + file + "</a></li>";
          list += "<ul>";
          for (int i = 0; i < hits.size(); ++i) {
              list += "<li>..." + hits[i] + "...</li>";
          }
          list += "</ul><br>";
       }
   }

   if (list.isEmpty()) {
      results += "<br>No results found.";
   }else {
      results += "<ul>" + list + "</ul>";
   }

   results += "</body></html>";
   return results;
}


void HelpBrowser::on_searchLineEdit_returnPressed()
{
   QString s(m_helpBrowser.searchLineEdit->text());
   if (!s.isEmpty()) {
      QTextBrowser* tb(m_helpBrowser.textBrowser);
      // Wrap the search if not found, there must be a better way
      if (!tb->find(s)) {
          tb->moveCursor(QTextCursor::Start); 
          tb->find(s);
      }
   }
}


void HelpBrowser::checkUrlForSearch(QUrl const& url)
{
   if (url.fragment().endsWith("searchString")) on_searchLineEdit_returnPressed();
}

} // end namespace IQmol
