#include "socketclient.h"
#include "globals.h"

socketClient::socketClient(QObject *parent) : QObject(parent)
{
    // Server Connection Information
    host = "127.0.0.1";
    port = 25001;

    receipt_query.insert(0, "s^ar");
    receipt_query.insert(1, "i^nr");
    receipt_query.insert(2, "s^rbmy");
    receipt_query.insert(3, "u^rbi");
    receipt_query.insert(4, "u^rdel");
    receipt_query.insert(5, "s^rfdc");
    receipt_query.insert(6, "s^rbi");

    inventory_query.insert(0, "s^ai");
    inventory_query.insert(1, "i^nii");
    inventory_query.insert(2, "u^ibi");
    inventory_query.insert(3, "u^dibi");

    information_query.insert(0, "s^inf");
    information_query.insert(1, "s^dinf");
    information_query.insert(2, "s^lgn");

    closings_query.insert(0, "i^inc");
    closings_query.insert(1, "s^cbdr");
    closings_query.insert(2, "u^cbi");
}

void socketClient::SubmitQuery(int type, int index, int id, QString update)
{
    // Store the current query for err reporting
    lastQuery.first = type;
    lastQuery.second = index;

    // Setup socket, and connect to host
    socket = new QTcpSocket(this);
    socket->connectToHost(host, port);

    if (socket->waitForConnected(10000))
    {
        qDebug() << "Connected!";

        // Byte array for converting qstring to const char *
        QByteArray bytes;
        QString temp;
        const char * query;

        // Get selected query, and convert
        switch(type)
        {
        case RECEIPT_QUERY:

            temp = receipt_query[index] + QUERY_DELIMITER + update;
            bytes = temp.toLocal8Bit();
            query = bytes.data();

            break;

        case INVENTORY_QUERY:

            if( index == 0 )
            {
                bytes = inventory_query[index].toLocal8Bit();
                query = bytes.data();
            }

            if( index == 1 || index == 2 || index == 3)
            {
                temp = inventory_query[index] + QUERY_DELIMITER + update;
                bytes = temp.toLocal8Bit();
                query = bytes.data();
            }

            break;

        case INFORMATION_QUERY:

            if( index == 0 || index == 1 )
            {
                bytes = information_query[index].toLocal8Bit();
                query = bytes.data();
            }

            if( index == 2 )
            {
                temp = information_query[index] + QUERY_DELIMITER + update;
                bytes = temp.toLocal8Bit();
                query = bytes.data();
            }
            break;

        case CLOSING_QUERY:
                temp = closings_query[index] + QUERY_DELIMITER + update;
                bytes = temp.toLocal8Bit();
                query = bytes.data();
                break;

            default:
                query = "ping";
                break;

        }

        // Send query, and recieve response
        socket->write(query);
        socket->waitForBytesWritten(10000);

        QByteArray arr;
        while(!arr.contains(SERVER_RECV_DELIMITER))
        {
            socket->waitForReadyRead();
            arr += socket->readAll();
        }
        int b = arr.indexOf(SERVER_RECV_DELIMITER);
        QByteArray message = arr.left(b);
        arr = arr.mid(b);
        emit dataReady(message);

        socket->close();
    }
    else
    {
         qDebug() << "Not Connected!";
    }
}
