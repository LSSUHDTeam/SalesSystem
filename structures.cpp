#include "structures.h"

QString subroutine_filterText(QString text)
{
    QStringList res = QStringList() << RESULT_MEMBER_DELIMITER << INCOMING_RESULT_DELIMITER << PACKAGE_DELIMITER
                                    << QUERY_DELIMITER << SERVER_SEND_DELIMITER << LIST_DELIMITER << RESULT_MEMBER_DELIMITER
                                    << PACKAGE_DELIMITER << FOPAL_LIST_DELIMITER << DEAL_DELIMITER << QString(SERVER_RECV_DELIMITER)
                                    << "'" << QString('"') << QString('\n') << QString('\t') << "_" << "`" << "?" << ";";
    QString temp = "";
    foreach(QString ch, text)
    {
        if( !res.contains(ch) )
        {
            temp += ch;
        }
    }

    if ( temp.length() < 1 )
        return QString(EMPTY_DB_ENTRY);
    else
        return temp;
}
