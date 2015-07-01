/***************************************************************************
 *   Copyright (C) 2008 by Bhooshan Supe                                   *
 *   bhooshan.supe@gmail.com                                               *
 *                                                                         *
 *   Before using this code contact the autor (bhooshan.supe@gmail.com)   *
 ***************************************************************************/

#ifndef QINTERPROCESSSIGNALPROPOGATOR_H
#define QINTERPROCESSSIGNALPROPOGATOR_H

#include <QtCore/qobject.h>

#ifndef QT_NO_NETWORK
#include <QtNetwork/qabstractsocket.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qhostaddress.h>


class QInterProcessSignalPropogator : public QObject
{
    Q_OBJECT

public:
	enum InterProcessSignalPropogatorType {
		InterProcessSignalPropogatorServer,
			InterProcessSignalPropogatorclient
	};

public:
    explicit QInterProcessSignalPropogator(InterProcessSignalPropogatorType type, QObject *parent=0, const QString &hostName = QString::null, quint16 port = 0);
    explicit QInterProcessSignalPropogator(InterProcessSignalPropogatorType type, QObject *parent=0, const QHostAddress &address = QHostAddress::Null, quint16 port = 0);

    virtual ~QInterProcessSignalPropogator();

	QTcpSocket *getInterProcessSignalPropogatorTcpSocket();

    virtual bool event(QEvent *pEvent);

protected slots:
	virtual void delayedConnectionToHostOrBindtoClients();
	virtual void disConnectedFromHostOrFromClients();
	virtual void interProcessSignalPropogatorTcpSocketReadyRead();
	virtual void interProcessSignalPropogatorTcpServerAcceptConnection();

	virtual void interProcessSignalPropogatorTcpSocketErrorHandler(QAbstractSocket::SocketError socketError);
	virtual void interProcessSignalPropogatorTcpSocketStateChangeHandler(QAbstractSocket::SocketState socketState);

protected:
	InterProcessSignalPropogatorType m_type;
	QHostAddress m_InterProcessSignalPropogatorServerAddress;
	quint16 m_InterProcessSignalPropogatorServerPort;

	QTcpServer *m_InterProcessSignalPropogatorTcpServer;

	QTcpSocket *m_InterProcessSignalPropogatorTcpSocket;
	quint16 m_blockSize;
};

#endif //QT_NO_NETWORK

#endif // QINTERPROCESSSIGNALPROPOGATOR_H
