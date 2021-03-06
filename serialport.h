#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QStringList>
#include <QDebug>

#include "currentalldata.h"

class SerialPort : public QObject
{
    Q_OBJECT

signals:
    connected();
    noConnected();
    disconnectedCorrectly();
    gotSample();

public slots:
    void readData();
    void connectWithDevice();

public:
    SerialPort();
    void send(QString data);
    void copyData(CurrentAllData *obj);

    QSerialPort *myDevice;
    QStringList BufferSplit;
    QVector<int> samplesIntVector;
    QByteArray byteArrayToReadData;
    int samplingFrequency = 0;
private:

    static const quint16 vendorID = 1027;
    static const quint16 productID = 24577;
};

#endif // SERIALPORT_H
