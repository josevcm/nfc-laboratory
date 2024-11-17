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

#include <QFileDialog>
#include <QDate>

#include <styles/Theme.h>

#include "ui_LicenseDialog.h"

#include "LicenseDialog.h"

struct LicenseDialog::Impl
{
   LicenseDialog *dialog = nullptr;

   QSharedPointer<Ui_LicenseDialog> ui;

   QString selectedFile;

   explicit Impl(LicenseDialog *dialog) : dialog(dialog), ui(new Ui_LicenseDialog())
   {
      setup();
   }

   void setup()
   {
      ui->setupUi(dialog);

      QObject::connect(ui->activateButton, &QPushButton::pressed, [=]() {
         selectFile();
      });

      QObject::connect(ui->removeButton, &QPushButton::pressed, [=]() {
         dialog->done(Deactivate);
      });
   }

   void selectFile()
   {
      selectedFile = QFileDialog::getOpenFileName(dialog, tr("Select your license file"), QDir::homePath(), tr("License Files (*.lic)"));

      if (selectedFile.isEmpty())
         return;

      dialog->done(Activate);
   }
};

LicenseDialog::LicenseDialog(QWidget *parent) : QDialog(parent), impl(new Impl(this))
{
}

QString LicenseDialog::selectedFile()
{
   return impl->selectedFile;
}

int LicenseDialog::showModal(const QString &message)
{
   impl->ui->infoLabel->setText(message);
   impl->ui->infoLabel->setVisible(!message.isEmpty());

   return Theme::showModalInDarkMode(this);
}

void LicenseDialog::setLicenseId(const QString &value)
{
   impl->ui->licenseId->setText(value);
   impl->ui->licenseForm->setVisible(true);
   impl->ui->removeButton->setVisible(true);
}

void LicenseDialog::setLicenseOwner(const QString &value)
{
   impl->ui->licenseName->setText(value);
   impl->ui->licenseForm->setVisible(true);
   impl->ui->removeButton->setVisible(true);
}

void LicenseDialog::setMachineId(const QString &value)
{
   impl->ui->machineId->setText(value);
   impl->ui->licenseForm->setVisible(true);
   impl->ui->removeButton->setVisible(true);
}

void LicenseDialog::setExpiryDate(const QDate &value)
{
   if (value.isNull())
      impl->ui->expiryDate->setText(tr("Unlimited"));
   else
      impl->ui->expiryDate->setText(value.toString("dd/MM/yyyy"));

   impl->ui->licenseForm->setVisible(true);
   impl->ui->removeButton->setVisible(true);
}

