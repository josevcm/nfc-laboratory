#ifndef SERIALSI5351_H
#define SERIALSI5351_H

#include <QObject>
#include <QWidget>
#include <QComboBox>
#include <QTextEdit>
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialSi5351 : public QMainWindow
{
      Q_OBJECT

   public:

      SerialSi5351(QWidget *parent = nullptr);

   private slots:

      void refreshPorts();

      void connectSerial();

      void sendCommand();

      void requestStatus();

      void readData();

   private:

      QSerialPort *serial;
      QComboBox *portSelector;
      QTextEdit *log;
};

#endif //SERIALSI5351_H
