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

#ifndef NFC_LAB_THEME_H
#define NFC_LAB_THEME_H

#include <QPen>
#include <QBrush>
#include <QFont>
#include <QIcon>
#include <QMessageBox>
#include <QFileDialog>

class Theme
{
   public:

      static const QIcon sortUpIcon;
      static const QIcon sortDownIcon;
      static const QIcon filterEmptyIcon;
      static const QIcon filterFilledIcon;
      static const QIcon filterFilledVoidIcon;

      static const QIcon startupIcon;
      static const QIcon requestIcon;
      static const QIcon responseIcon;
      static const QIcon exchangeIcon;
      static const QIcon carrierOnIcon;
      static const QIcon carrierOffIcon;
      static const QIcon encryptedIcon;
      static const QIcon truncatedIcon;
      static const QIcon crcErrorIcon;
      static const QIcon parityErrorIcon;
      static const QIcon syncErrorIcon;

      static const QColor defaultTextColor;
      static const QPen defaultTextPen;
      static const QFont defaultTextFont;
      static const QFont monospaceTextFont;

      static const QPen defaultAxisPen;
      static const QPen defaultTickPen;
      static const QPen defaultGridPen;

      static const QColor defaultLabelColor;
      static const QPen defaultLabelPen;
      static const QBrush defaultLabelBrush;
      static const QFont defaultLabelFont;

      static const QPen defaultBracketPen;
      static const QPen defaultBracketLabelPen;
      static const QColor defaultBracketLabelColor;
      static const QFont defaultBracketLabelFont;
      static const QBrush defaultBracketLabelBrush;

      static const QPen defaultMarkerPen;
      static const QBrush defaultMarkerBrush;
      static const QPen defaultMarkerActivePen;
      static const QBrush defaultMarkerActiveBrush;

      static const QPen defaultSelectionPen;
      static const QBrush defaultSelectionBrush;
      static const QPen defaultActiveSelectionPen;
      static const QBrush defaultActiveSelectionBrush;

      static const QColor defaultLogicIOColor;
      static const QPen defaultLogicIOPen;
      static const QBrush defaultLogicIOBrush;

      static const QColor defaultLogicCLKColor;
      static const QPen defaultLogicCLKPen;
      static const QBrush defaultLogicCLKBrush;

      static const QColor defaultLogicRSTColor;
      static const QPen defaultLogicRSTPen;
      static const QBrush defaultLogicRSTBrush;

      static const QColor defaultLogicVCCColor;
      static const QPen defaultLogicVCCPen;
      static const QBrush defaultLogicVCCBrush;

      static const QColor defaultRadioNFCColor;
      static const QPen defaultRadioNFCPen;
      static const QBrush defaultRadioNFCBrush;

      static const QColor defaultNfcAColor;
      static const QPen defaultNfcAPen;
      static const QBrush defaultNfcABrush;
      static const QBrush responseNfcABrush;

      static const QColor defaultNfcBColor;
      static const QPen defaultNfcBPen;
      static const QBrush defaultNfcBBrush;
      static const QBrush responseNfcBBrush;

      static const QColor defaultNfcFColor;
      static const QPen defaultNfcFPen;
      static const QBrush defaultNfcFBrush;
      static const QBrush responseNfcFBrush;

      static const QColor defaultNfcVColor;
      static const QPen defaultNfcVPen;
      static const QBrush defaultNfcVBrush;
      static const QBrush responseNfcVBrush;

      static const QPen defaultSignalPen;
      static const QBrush defaultSignalBrush;

      static const QPen defaultCarrierPen;
      static const QBrush defaultCarrierBrush;

      static const QPen defaultFrequencyPen;
      static const QBrush defaultFrequencyBrush;

      static const QColor defaultCenterFreqColor;
      static const QFont defaultCenterFreqFont;

   public:

      static int messageDialog(QWidget *parent, const QString &title, const QString &text, QMessageBox::Icon icon = QMessageBox::Information, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

      static QString openFileDialog(QWidget *parent = nullptr, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr, QFileDialog::Options options = QFileDialog::Options());

      static QString saveFileDialog(QWidget *parent = nullptr, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr, QFileDialog::Options options = QFileDialog::Options());

      static void showInDarkMode(QWidget *widget);

      static int showModalInDarkMode(QDialog *dialog);

   private:

      static QPen makePen(const QColor &color, Qt::PenStyle style = Qt::SolidLine, float width = 1);

      static QBrush makeBrush(const QColor &color, Qt::BrushStyle style = Qt::SolidPattern);

      static QFont makeFont(const QString &family, int pointSize = -1, int weight = -1, bool italic = false, int hint = -1);
};

#endif // NFC_LAB_THEME_H
