/***************************************************************************
 *   Copyright (C) 2008 by Bhooshan Supe                                   *
 *   bhooshan.supe@gmail.com                                               *
 *                                                                         *
 *   Before using this code contact the autor (bhooshan.supe@gmail.com)   *
 ***************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaType>
#include <QtCore/QSemaphore>

#include "qinterprocesssignalpropagator.h"

// this class is copied from qobject_p.h to prevent us
// from including private Qt headers.

#if (QT_VERSION >= 0x050000)

class QMetaCallEvent : public QEvent
{
public:
    //BAS_TBR   QMetaCallEvent(ushort method_offset, ushort method_relative, QObjectPrivate::StaticMetaCallFunction callFunction , const QObject *sender, int signalId,
    //BAS_TBR                  int nargs = 0, int *types = 0, void **args = 0, QSemaphore *semaphore = 0);

    QMetaCallEvent(QtPrivate::QSlotObjectBase *slotObj, const QObject *sender, int signalId,
                   int nargs = 0, int *types = 0, void **args = 0, QSemaphore *semaphore = 0);

    ~QMetaCallEvent();

    inline int id() const { return method_offset_ + method_relative_; }
    inline const QObject *sender() const { return sender_; }
    inline int signalId() const { return signalId_; }
    inline void **args() const { return args_; }

    virtual void placeMetaCall(QObject *object);

private:
    QtPrivate::QSlotObjectBase *slotObj_;
    const QObject *sender_;
    int signalId_;
    int nargs_;
    int *types_;
    void **args_;
    QSemaphore *semaphore_;
    //BAS_TBR   QObjectPrivate::StaticMetaCallFunction callFunction_;
    ushort method_offset_;
    ushort method_relative_;
};

#elif ((QT_VERSION >= 0x040400) && (QT_VERSION < 0x050000))

class QMetaCallEvent : public QEvent
{
public:
    QMetaCallEvent(int id, const QObject *sender, int signalId,
                   int nargs = 0, int *types = 0, void **args = 0, QSemaphore *semaphore = 0);
    ~QMetaCallEvent();

    inline int id() const { return id_; }
    inline const QObject *sender() const { return sender_; }
    inline int signalId() const { return signalId_; }
    inline int nargs() const { return nargs_; }
    inline int *types() const { return types_; }
    inline void **args() const { return args_; }

    virtual int placeMetaCall(QObject *object);

private:
    int id_;
    const QObject *sender_;
    int signalId_;
    int nargs_;
    int *types_;
    void **args_;
    QSemaphore *semaphore_;
};

#elif ((QT_VERSION >= 0x040303) && (QT_VERSION < 0x040400))

class QMetaCallEvent : public QEvent
{
public:
    QMetaCallEvent(int id, const QObject *sender, int idFrom, int idTo,
                   int nargs = 0, int *types = 0, void **args = 0, QSemaphore *semaphore = 0);
    ~QMetaCallEvent();

    inline int id() const { return id_; }
    inline const QObject *sender() const { return sender_; }
    inline int signalIdStart() const { return idFrom_; }
    inline int signalIdEnd() const { return idTo_; }
    inline int nargs() const { return nargs_; }
    inline int *types() const { return types_; }
    inline void **args() const { return args_; }

    virtual int placeMetaCall(QObject *object);

private:
    int id_;
    const QObject *sender_;
    int idFrom_;
    int idTo_;
    int nargs_;
    int *types_;
    void **args_;
    QSemaphore *semaphore_;
};

#else

#endif


#ifndef QT_NO_NETWORK

QInterProcessSignalPropogator::QInterProcessSignalPropogator(InterProcessSignalPropogatorType type, QObject *parent, \
                                                             const QString &hostName, quint16 port)
    : QObject(parent)
{
    m_InterProcessSignalPropogatorTcpSocket = NULL;
    m_InterProcessSignalPropogatorTcpServer = NULL;
    m_blockSize = 0;
    m_type = type;
    m_InterProcessSignalPropogatorServerAddress = QHostAddress(hostName);
    m_InterProcessSignalPropogatorServerPort = port;
    delayedConnectionToHostOrBindtoClients();
}

QInterProcessSignalPropogator::QInterProcessSignalPropogator(InterProcessSignalPropogatorType type, QObject *parent, \
                                                             const QHostAddress &address, quint16 port)
    : QObject(parent)
{
    m_InterProcessSignalPropogatorTcpSocket = NULL;
    m_InterProcessSignalPropogatorTcpServer = NULL;
    m_blockSize = 0;
    m_type = type;
    m_InterProcessSignalPropogatorServerAddress = QHostAddress(address);
    m_InterProcessSignalPropogatorServerPort = port;
    delayedConnectionToHostOrBindtoClients();
}

QInterProcessSignalPropogator::~QInterProcessSignalPropogator()
{
}

QTcpSocket *QInterProcessSignalPropogator::getInterProcessSignalPropogatorTcpSocket()
{
    return m_InterProcessSignalPropogatorTcpSocket;
}

void QInterProcessSignalPropogator::delayedConnectionToHostOrBindtoClients()
{
    if(InterProcessSignalPropogatorServer == m_type)
    {
        m_InterProcessSignalPropogatorTcpServer = new QTcpServer(this);
        if (true == m_InterProcessSignalPropogatorTcpServer->listen(m_InterProcessSignalPropogatorServerAddress, m_InterProcessSignalPropogatorServerPort))
        {
            bool bRetvalue = connect(m_InterProcessSignalPropogatorTcpServer, SIGNAL(newConnection()), this, SLOT(interProcessSignalPropogatorTcpServerAcceptConnection()));
            qDebug("%s::%d::bRetvalue = %d", __FILE__, __LINE__, bRetvalue);
            qDebug() << m_InterProcessSignalPropogatorTcpServer->serverAddress();
            qDebug() << m_InterProcessSignalPropogatorTcpServer->serverPort();
            qDebug() << m_InterProcessSignalPropogatorTcpServer->errorString();
        }
        else
        {
            qDebug() << m_InterProcessSignalPropogatorTcpServer->errorString();
            return;
        }
    }
    else if(InterProcessSignalPropogatorclient == m_type)
    {
        m_InterProcessSignalPropogatorTcpSocket = new QTcpSocket(this);
        if(NULL != m_InterProcessSignalPropogatorTcpSocket)
        {
            m_InterProcessSignalPropogatorTcpSocket->connectToHost(m_InterProcessSignalPropogatorServerAddress, m_InterProcessSignalPropogatorServerPort);
            bool bRetvalue = false;
            bRetvalue = connect(m_InterProcessSignalPropogatorTcpSocket, SIGNAL(readyRead()), this, SLOT(interProcessSignalPropogatorTcpSocketReadyRead()));
            bRetvalue = false;
            bRetvalue = connect(m_InterProcessSignalPropogatorTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(interProcessSignalPropogatorTcpSocketErrorHandler(QAbstractSocket::SocketError)));
            bRetvalue = false;
            bRetvalue = connect(m_InterProcessSignalPropogatorTcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(interProcessSignalPropogatorTcpSocketStateChangeHandler(QAbstractSocket::SocketState)));
            qDebug("%s::%d::bRetvalue = %d", __FILE__, __LINE__, bRetvalue);
            qDebug() << m_InterProcessSignalPropogatorTcpSocket->localAddress();
            qDebug() << m_InterProcessSignalPropogatorTcpSocket->localPort();
            qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerAddress();
            qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerPort();
            qDebug() << m_InterProcessSignalPropogatorTcpSocket->state();
            qDebug() << m_InterProcessSignalPropogatorTcpSocket->errorString();
        }
    }
    else
    {
    }
}

void QInterProcessSignalPropogator::disConnectedFromHostOrFromClients()
{
}

void QInterProcessSignalPropogator::interProcessSignalPropogatorTcpServerAcceptConnection()
{
    m_InterProcessSignalPropogatorTcpSocket = m_InterProcessSignalPropogatorTcpServer->nextPendingConnection();
    if(NULL != m_InterProcessSignalPropogatorTcpSocket)
    {
        qDebug("%s::%d", __FILE__, __LINE__);
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->localAddress();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->localPort();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerAddress();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerPort();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->state();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->errorString();
        bool bRetvalue = connect(m_InterProcessSignalPropogatorTcpSocket, SIGNAL(readyRead()), this, SLOT(interProcessSignalPropogatorTcpSocketReadyRead()));
        qDebug("%s::%d::bRetvalue = %d", __FILE__, __LINE__, bRetvalue);
    }
}

bool QInterProcessSignalPropogator::event(QEvent *pEvent)
{
    switch (pEvent->type())
    {
    case QEvent::MetaCall:
    {
        QMetaCallEvent *mce = static_cast<QMetaCallEvent*>(pEvent);
        qDebug("%s::%d", __FILE__, __LINE__);

        //SEND THE SIGNAL OVER ETHERNET
        {
            bool lbIsSignalDataWritten = true;
            QByteArray lInterProcessSignalDataBlock;

            QDataStream lInterProcessSignalOutDataStream(&lInterProcessSignalDataBlock, QIODevice::WriteOnly);

#if (QT_VERSION >= 0x050000)
        lInterProcessSignalOutDataStream.setVersion(QDataStream::Qt_5_4);
#elif ((QT_VERSION >= 0x040400) && (QT_VERSION < 0x050000))
        lInterProcessSignalOutDataStream.setVersion(QDataStream::Qt_4_4);
#elif ((QT_VERSION >= 0x040303) && (QT_VERSION < 0x040400))
        lInterProcessSignalOutDataStream.setVersion(QDataStream::Qt_4_0);
#else
#endif

            lInterProcessSignalOutDataStream << (quint16)0;

#if (QT_VERSION >= 0x050000)

            lInterProcessSignalOutDataStream << 8101980;

#elif ((QT_VERSION >= 0x040400) && (QT_VERSION < 0x050000))

            lInterProcessSignalOutDataStream << mce->id();
            lInterProcessSignalOutDataStream << mce->signalId();
            lInterProcessSignalOutDataStream << mce->nargs();

            for (int n = 1; n < mce->nargs(); ++n)
            {
                lInterProcessSignalOutDataStream << mce->types()[n];

                if(true == QMetaType::save(lInterProcessSignalOutDataStream, mce->types()[n], mce->args()[n]))
                {
                }
                else
                {
                    lbIsSignalDataWritten = false;
                }
            }

#elif ((QT_VERSION >= 0x040303) && (QT_VERSION < 0x040400))

            lInterProcessSignalOutDataStream << mce->id();
            lInterProcessSignalOutDataStream << mce->signalIdStart();
            lInterProcessSignalOutDataStream << mce->signalIdEnd();
            lInterProcessSignalOutDataStream << mce->nargs();

            for (int n = 1; n < mce->nargs(); ++n)
            {
                lInterProcessSignalOutDataStream << mce->types()[n];

                if(true == QMetaType::save(lInterProcessSignalOutDataStream, mce->types()[n], mce->args()[n]))
                {
                }
                else
                {
                    lbIsSignalDataWritten = false;
                }
            }

#else
#endif

            if(true == lbIsSignalDataWritten)
            {
                if(NULL != m_InterProcessSignalPropogatorTcpSocket)
                {
                    lInterProcessSignalOutDataStream.device()->seek(0);
                    lInterProcessSignalOutDataStream << (quint16)(lInterProcessSignalDataBlock.size() - sizeof(quint16));
                    m_InterProcessSignalPropogatorTcpSocket->write(lInterProcessSignalDataBlock);
                }
            }
        }

        break;
    }
    default:
    {
        return QObject::event(pEvent);
    }
    }

    return true;
}

void QInterProcessSignalPropogator::interProcessSignalPropogatorTcpSocketReadyRead()
{
    if(NULL != m_InterProcessSignalPropogatorTcpSocket)
    {
        qDebug("%s::%d", __FILE__, __LINE__);
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->localAddress();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->localPort();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerAddress();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerPort();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->state();
        qDebug() << m_InterProcessSignalPropogatorTcpSocket->errorString();


        QDataStream lInterProcessSignalInDataStream(m_InterProcessSignalPropogatorTcpSocket);

#if (QT_VERSION >= 0x050000)
        lInterProcessSignalInDataStream.setVersion(QDataStream::Qt_5_4);
#elif ((QT_VERSION >= 0x040400) && (QT_VERSION < 0x050000))
        lInterProcessSignalInDataStream.setVersion(QDataStream::Qt_4_4);
#elif ((QT_VERSION >= 0x040303) && (QT_VERSION < 0x040400))
        lInterProcessSignalInDataStream.setVersion(QDataStream::Qt_4_0);
#else
#endif

        if (m_blockSize == 0)
        {
            if (m_InterProcessSignalPropogatorTcpSocket->bytesAvailable() < (int)sizeof(quint16))
            {
                return;
            }
            else
            {
                lInterProcessSignalInDataStream >> m_blockSize;
            }
        }
        else
        {
            if (m_InterProcessSignalPropogatorTcpSocket->bytesAvailable() < m_blockSize)
            {
                return;
            }
        }
        {
            bool lbIsSignalDataRead = true;

#if (QT_VERSION >= 0x050000)
            int msgdata;
            lInterProcessSignalInDataStream >> msgdata;
            qDebug("%s::%d:msgdata=%d", __FILE__, __LINE__, msgdata);

#elif ((QT_VERSION >= 0x040400) && (QT_VERSION < 0x050000))

            int id;
            int signalId;
            int nargs;

            lInterProcessSignalInDataStream >> id;
            lInterProcessSignalInDataStream >> signalId;
            lInterProcessSignalInDataStream >> nargs;

            int *types = (int *) qMalloc(nargs*sizeof(int));
            void **args = (void **) qMalloc(nargs*sizeof(void *));
            args[0] = NULL;

            for (int n = 1; n < nargs; ++n)
            {
                args[n] = NULL;
                lInterProcessSignalInDataStream >> types[n];
                args[n] = QMetaType::construct(types[n]);

                if(true == QMetaType::load(lInterProcessSignalInDataStream, types[n], args[n]))
                {
                }
                else
                {
                    lbIsSignalDataRead = false;
                }
            }

#elif ((QT_VERSION >= 0x040303) && (QT_VERSION < 0x040400))

            int id;
            int idFrom;
            int idTo;
            int nargs;

            lInterProcessSignalInDataStream >> id;
            lInterProcessSignalInDataStream >> idFrom;
            lInterProcessSignalInDataStream >> idTo;
            lInterProcessSignalInDataStream >> nargs;

            int *types = (int *) qMalloc(nargs*sizeof(int));
            void **args = (void **) qMalloc(nargs*sizeof(void *));
            args[0] = NULL;

            for (int n = 1; n < nargs; ++n)
            {
                args[n] = NULL;
                lInterProcessSignalInDataStream >> types[n];
                args[n] = QMetaType::construct(types[n]);

                if(true == QMetaType::load(lInterProcessSignalInDataStream, types[n], args[n]))
                {
                }
                else
                {
                    lbIsSignalDataRead = false;
                }
            }

#else

#endif

            if(true == lbIsSignalDataRead)
            {
                QMetaCallEvent *mce = NULL;

#if (QT_VERSION >= 0x050000)
#elif ((QT_VERSION >= 0x040400) && (QT_VERSION < 0x050000))

                QMetaCallEvent *mce = new QMetaCallEvent(id, this, signalId, nargs, types, args);

#elif ((QT_VERSION >= 0x040303) && (QT_VERSION < 0x040400))

                QMetaCallEvent *mce = new QMetaCallEvent(id, this, idFrom, idTo, nargs, types, args);

#else
#endif

#if defined(QT_NO_EXCEPTIONS)
                mce->placeMetaCall(this);
#else
                try {
                    if (mce) {
                        mce->placeMetaCall(this);
                    }
                } catch (...) {
                    delete mce;
                    throw;
                }
                delete mce;
#endif
            }

            m_blockSize = 0;
        }
    }
}

void QInterProcessSignalPropogator::interProcessSignalPropogatorTcpSocketErrorHandler(QAbstractSocket::SocketError socketError)
{
    qDebug("%s::%d", __FILE__, __LINE__);
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->localAddress();
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->localPort();
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerAddress();
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerPort();
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->state();
    qDebug() << socketError << "::" << m_InterProcessSignalPropogatorTcpSocket->errorString();
}

void QInterProcessSignalPropogator::interProcessSignalPropogatorTcpSocketStateChangeHandler(QAbstractSocket::SocketState socketState)
{
    qDebug("%s::%d", __FILE__, __LINE__);
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->localAddress();
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->localPort();
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerAddress();
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->peerPort();
    qDebug() << socketState << "::" << m_InterProcessSignalPropogatorTcpSocket->state();
    qDebug() << m_InterProcessSignalPropogatorTcpSocket->errorString();
}

#endif //QT_NO_NETWORK

