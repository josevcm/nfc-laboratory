//
// Created by jvcampos on 26/02/2025.
//

#include <QApplication>
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>

#include "SerialSi5351.h"

SerialSi5351::SerialSi5351(QWidget *parent) : QMainWindow(parent)
{
   QWidget *centralWidget = new QWidget(this);
   QVBoxLayout *layout = new QVBoxLayout(centralWidget);

   portSelector = new QComboBox(this);
   refreshPorts();
   layout->addWidget(new QLabel("Seleccionar Puerto:"));
   layout->addWidget(portSelector);

   QPushButton *connectButton = new QPushButton("Conectar", this);
   layout->addWidget(connectButton);
   connect(connectButton, &QPushButton::clicked, this, &SerialSi5351::connectSerial);

   QPushButton *sendButton = new QPushButton("Configurar SI5351 a 1MHz", this);
   layout->addWidget(sendButton);
   connect(sendButton, &QPushButton::clicked, this, &SerialSi5351::sendCommand);

   QPushButton *statusButton = new QPushButton("Obtener Estado del SI5351", this);
   layout->addWidget(statusButton);
   connect(statusButton, &QPushButton::clicked, this, &SerialSi5351::requestStatus);

   log = new QTextEdit(this);
   log->setReadOnly(true);
   layout->addWidget(log);

   serial = new QSerialPort(this);
   connect(serial, &QSerialPort::readyRead, this, &SerialSi5351::readData);

   setCentralWidget(centralWidget);
   setWindowTitle("SI5351 Control");
}

void SerialSi5351::refreshPorts()
{
   portSelector->clear();

   for (const QSerialPortInfo &info: QSerialPortInfo::availablePorts())
   {
      portSelector->addItem(info.portName());
   }
}

void SerialSi5351::connectSerial()
{
   if (serial->isOpen())
   {
      serial->close();
   }

   serial->setPortName(portSelector->currentText());
   serial->setBaudRate(QSerialPort::Baud115200);
   serial->setDataBits(QSerialPort::Data8);
   serial->setParity(QSerialPort::NoParity);
   serial->setStopBits(QSerialPort::OneStop);
   serial->setFlowControl(QSerialPort::NoFlowControl);

   if (serial->open(QIODevice::ReadWrite))
   {
      log->append("Conectado a " + serial->portName());
   }
   else
   {
      log->append("Error al conectar: " + serial->errorString());
   }
}

void SerialSi5351::sendCommand()
{
   if (serial->isOpen())
   {
      QByteArray command = "SET_FREQ 1000000\n";
      serial->write(command);
      log->append("Enviado: " + QString(command));
   }
   else
   {
      log->append("No hay conexión activa.");
   }
}

void SerialSi5351::requestStatus()
{
   if (serial->isOpen())
   {
      QByteArray command = "GET_STATUS\n";
      serial->write(command);
      log->append("Enviado: " + QString(command));
   }
   else
   {
      log->append("No hay conexión activa.");
   }
}

void SerialSi5351::readData()
{
   QByteArray data = serial->readAll();
   log->append("Recibido: " + QString(data));
}
