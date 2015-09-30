#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QHash>


#define DELIVERY_QUERY 1
#define DEVICE_QUERY 2
#define GENERATE_QUERY 3
#define ADMIN_QUERY 4


class socketClient : public QObject
{
    Q_OBJECT
public:
    explicit socketClient(QObject *parent = 0);

    void SubmitQuery(int type, int index, int id = 0, QString update = "");

    QPair<int,int> lastQuery;

signals:
    void dataReady(QString);

public slots:

private :

    // Connection Variables
    int port;
    QString host;
    QTcpSocket *socket;

    // Selecet Queries
    QHash<int, QString> receipt_query, inventory_query,
                        information_query, closings_query;
    qint64 sizeLastTransfer;

};

#endif // SOCKETCLIENT_H
