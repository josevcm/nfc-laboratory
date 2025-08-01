/*

  This file is part of NFC-LABORATORY.

  Copyright (C) 2024 Jose Vicente Campos Martinez, <josevcm@gmail.com>

  NFC-LABORATORY is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NFC-LABORATORY is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NFC-LABORATORY. If not, see <http://www.gnu.org/licenses/>.

*/

#ifdef _WIN32

#include <dwmapi.h>

#endif

#include <QWidget>
#include <QEventLoop>

#include "Theme.h"

const QIcon Theme::sortUpIcon(QIcon::fromTheme("caret-up-filled"));
const QIcon Theme::sortDownIcon(QIcon::fromTheme("caret-down-filled"));
const QIcon Theme::filterEmptyIcon(QIcon::fromTheme("filter-empty"));
const QIcon Theme::filterFilledIcon(QIcon::fromTheme("filter-filled"));
const QIcon Theme::filterFilledVoidIcon(QIcon::fromTheme("filter-filled-void"));

const QIcon Theme::vccLowIcon(QIcon::fromTheme("vcc-low"));
const QIcon Theme::vccHighIcon(QIcon::fromTheme("vcc-high"));
const QIcon Theme::rstLowIcon(QIcon::fromTheme("rst-low"));
const QIcon Theme::rstHighIcon(QIcon::fromTheme("rst-high"));
const QIcon Theme::startupIcon(QIcon::fromTheme("frame-startup"));
const QIcon Theme::requestIcon(QIcon::fromTheme("frame-request"));
const QIcon Theme::responseIcon(QIcon::fromTheme("frame-response"));
const QIcon Theme::exchangeIcon(QIcon::fromTheme("frame-exchange"));
const QIcon Theme::carrierOnIcon(QIcon::fromTheme("carrier-on"));
const QIcon Theme::carrierOffIcon(QIcon::fromTheme("carrier-off"));
const QIcon Theme::encryptedIcon(QIcon::fromTheme("lock-flag-filled"));
const QIcon Theme::truncatedIcon(QIcon::fromTheme("alert-triangle-filled"));
const QIcon Theme::crcErrorIcon(QIcon::fromTheme("alert-circle-filled"));
const QIcon Theme::parityErrorIcon(QIcon::fromTheme("alert-circle-filled"));
const QIcon Theme::syncErrorIcon(QIcon::fromTheme("alert-circle-filled"));

const QColor Theme::defaultTextColor = QColor(0xE0, 0xE0, 0xE0, 0xff);
const QPen Theme::defaultTextPen = makePen(defaultTextColor);
const QFont Theme::defaultTextFont = makeFont("Verdana", 9, QFont::Normal);
const QFont Theme::monospaceTextFont = makeFont("Verdana", 9, QFont::Normal, false, QFont::TypeWriter);

const QPen Theme::defaultAxisPen = makePen({0x74, 0x74, 0x7b, 0xff});
const QPen Theme::defaultTickPen = makePen({0x74, 0x74, 0x7b, 0xff});
const QPen Theme::defaultGridPen = makePen({0x44, 0x44, 0x4e, 0xff}, Qt::DotLine);

const QColor Theme::defaultLabelColor({0xF0, 0xF0, 0xF0, 0xFF});
const QPen Theme::defaultLabelPen({0x2B, 0x2B, 0x2B, 0x70});
const QBrush Theme::defaultLabelBrush({0x2B, 0x2B, 0x2B, 0xC0});
const QFont Theme::defaultLabelFont("Roboto", 9, QFont::Normal);

const QPen Theme::defaultBracketPen = makePen({0xC0, 0xC0, 0xC0, 0xff});
const QPen Theme::defaultBracketLabelPen = makePen({0x00, 0x00, 0x00, 0x00}, Qt::NoPen);
const QColor Theme::defaultBracketLabelColor = QColor(0xC0, 0xC0, 0xC0, 0xff);
const QFont Theme::defaultBracketLabelFont = makeFont("Roboto", 9, QFont::Normal);
const QBrush Theme::defaultBracketLabelBrush = makeBrush({0x00, 0x00, 0x00, 0x00}, Qt::NoBrush);

const QPen Theme::defaultMarkerPen = makePen({0xFF, 0x90, 0x90, 0xFF}, Qt::SolidLine, 2.5f);
const QBrush Theme::defaultMarkerBrush = makeBrush({0xC0, 0x65, 0x91, 0x30});
const QPen Theme::defaultMarkerActivePen = makePen({0xC0, 0x65, 0x91, 0x52});
const QBrush Theme::defaultMarkerActiveBrush = makeBrush({0xC0, 0x65, 0x91, 0x52});

const QPen Theme::defaultSelectionPen = makePen({0x00, 0x80, 0xFF, 0x50});
const QBrush Theme::defaultSelectionBrush = makeBrush({0x00, 0x80, 0xFF, 0x50});
const QPen Theme::defaultActiveSelectionPen = makePen({0x00, 0x80, 0xFF, 0x50});
const QBrush Theme::defaultActiveSelectionBrush = makeBrush({0x00, 0x80, 0xFF, 0x50});

const QColor Theme::defaultLogicIOColor = QColor(0x13, 0x99, 0x49, 0xF0);
const QPen Theme::defaultLogicIOPen = makePen(defaultLogicIOColor);
const QBrush Theme::defaultLogicIOBrush = makeBrush(defaultLogicIOColor);

const QColor Theme::defaultLogicCLKColor = QColor(0x75, 0x50, 0x7B, 0xF0);
const QPen Theme::defaultLogicCLKPen = makePen(defaultLogicCLKColor);
const QBrush Theme::defaultLogicCLKBrush = makeBrush(defaultLogicCLKColor);

const QColor Theme::defaultLogicRSTColor = QColor(0x34, 0x65, 0xA4, 0xF0);
const QPen Theme::defaultLogicRSTPen = makePen(defaultLogicRSTColor);
const QBrush Theme::defaultLogicRSTBrush = makeBrush(defaultLogicRSTColor);

const QColor Theme::defaultLogicVCCColor = QColor(0xA4, 0x40, 0x40, 0xF0);
const QPen Theme::defaultLogicVCCPen = makePen(defaultLogicVCCColor);
const QBrush Theme::defaultLogicVCCBrush = makeBrush(defaultLogicVCCColor);

const QColor Theme::defaultRadioNFCColor = QColor(0x20, 0x90, 0x35, 0xF0);
const QPen Theme::defaultRadioNFCPen = makePen(defaultRadioNFCColor);
const QBrush Theme::defaultRadioNFCBrush = makeBrush(defaultRadioNFCColor);

const QColor Theme::defaultNfcAColor = QColor(0x13, 0x99, 0x49, 0xF0);
const QPen Theme::defaultNfcAPen = makePen(defaultNfcAColor);
const QBrush Theme::defaultNfcABrush = makeBrush(defaultNfcAColor);
const QBrush Theme::responseNfcABrush = makeBrush(defaultNfcAColor, Qt::Dense4Pattern);

const QColor Theme::defaultNfcBColor = QColor(0x34, 0x65, 0xA4, 0xF0);
const QPen Theme::defaultNfcBPen = makePen(defaultNfcBColor);
const QBrush Theme::defaultNfcBBrush = makeBrush(defaultNfcBColor);
const QBrush Theme::responseNfcBBrush = makeBrush(defaultNfcBColor, Qt::Dense4Pattern);

const QColor Theme::defaultNfcFColor = QColor(0xA4, 0x40, 0x40, 0xF0);
const QPen Theme::defaultNfcFPen = makePen(defaultNfcFColor);
const QBrush Theme::defaultNfcFBrush = makeBrush(defaultNfcFColor);
const QBrush Theme::responseNfcFBrush = makeBrush(defaultNfcFColor, Qt::Dense4Pattern);

const QColor Theme::defaultNfcVColor = QColor(0x75, 0x50, 0x7B, 0xF0);
const QPen Theme::defaultNfcVPen = makePen(defaultNfcVColor);
const QBrush Theme::defaultNfcVBrush = makeBrush(defaultNfcVColor);
const QBrush Theme::responseNfcVBrush = makeBrush(defaultNfcVColor, Qt::Dense4Pattern);

const QPen Theme::defaultSignalPen = makePen({0xE0, 0xE0, 0xE0, 0xFF});
const QBrush Theme::defaultSignalBrush = makeBrush({0x00, 0x40, 0x90, 0x50});

const QPen Theme::defaultCarrierPen = makePen(0x707070);
const QBrush Theme::defaultCarrierBrush = makeBrush({0x80, 0x70, 0x60, 0x50}, Qt::Dense4Pattern);

const QPen Theme::defaultFrequencyPen = makePen({0x90, 0x90, 0x90, 0xFF}, Qt::SolidLine, 2.0f);
const QBrush Theme::defaultFrequencyBrush = makeBrush({0x00, 0x00, 0xFF, 0x14});

const QColor Theme::defaultCenterFreqColor({0xF0, 0xF0, 0xF0, 0xFF});
const QFont Theme::defaultCenterFreqFont("Roboto", 9, QFont::Bold);

int Theme::messageDialog(QWidget *parent, const QString &title, const QString &text, QMessageBox::Icon icon, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
   QMessageBox messageBox(parent);

   messageBox.setIcon(icon);
   messageBox.setWindowTitle(title);
   messageBox.setText(text);
   messageBox.setStandardButtons(buttons);
   messageBox.setDefaultButton(defaultButton);

   return showModalInDarkMode(&messageBox);
}

QString Theme::openFileDialog(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options)
{
   QFileDialog fileDialog(parent, caption, dir, filter);

   fileDialog.setOptions(options);
   fileDialog.setFileMode(QFileDialog::ExistingFile);

   //   if (showModalInDarkMode(&fileDialog) == QDialog::Accepted)
   if (fileDialog.exec() == QDialog::Accepted)
   {
      if (selectedFilter)
         *selectedFilter = fileDialog.selectedNameFilter();

      return fileDialog.selectedFiles().value(0);
   }

   return {};
}

QString Theme::saveFileDialog(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options)
{
   QFileDialog fileDialog(parent, caption, dir, filter);

   fileDialog.setOptions(options);
   fileDialog.setAcceptMode(QFileDialog::AcceptSave);

   //   if (showModalInDarkMode(&fileDialog) == QDialog::Accepted)
   if (fileDialog.exec() == QDialog::Accepted)
   {
      if (selectedFilter)
         *selectedFilter = fileDialog.selectedNameFilter();

      return fileDialog.selectedFiles().value(0);
   }

   return {};
}

int Theme::showModalInDarkMode(QDialog *dialog)
{
   QEventLoop loop;

   // show widget in dark mode
   showInDarkMode(dialog);

   // connect signal to event loop fon synchronous wait
   QObject::connect(dialog, &QDialog::finished, &loop, &QEventLoop::exit);

   // synchronous wait
   return loop.exec();
}

void Theme::showInDarkMode(QWidget *widget)
{
   widget->show();

#ifdef _WIN32
   // BOOL darkMode = TRUE;
   // DwmSetWindowAttribute((HWND) widget->winId(), 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &darkMode, sizeof(darkMode));
#endif
}

QPen Theme::makePen(const QColor &color, Qt::PenStyle style, float width)
{
   QPen pen(color);

   pen.setStyle(style);
   pen.setWidthF(width);

   return pen;
}

QBrush Theme::makeBrush(const QColor &color, Qt::BrushStyle style)
{
   QBrush brush(color);

   brush.setStyle(style);;

   return brush;
}

QFont Theme::makeFont(const QString &family, int pointSize, int weight, bool italic, int hint)
{
   QFont font(family, pointSize, weight, italic);

   if (hint >= 0)
      font.setStyleHint(QFont::StyleHint(hint));

   return font;
}
