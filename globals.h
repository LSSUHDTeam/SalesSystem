#ifndef GLOBALS_H
#define GLOBALS_H

#endif // GLOBALS_H

#define RECEIPT_QUERY 1
#define INVENTORY_QUERY 2
#define INFORMATION_QUERY 3
#define CLOSING_QUERY 4

#define PURGE_INVENTORY   0
#define PURGE_RECEIPTS    1
#define PURGE_INFORMATION 2
#define PURGE_DEPARTMENTS 3

#define UPDATE_INVENTORY              0
#define UPDATE_RECEIPTS               1
#define UPDATE_INFORMATION            2
#define UPDATE_ALL_AND_PURGE_RECEIPTS 3
#define UPDATE_DEPARTMENT_INFORMATION 4

// An iteration of structures uses an unsigned int in transactionviewer,
// if MAX_RECEIPTS_PER_SELECT is increased substantially it must be edited
#define MAX_RECEIPTS_PER_SELECT 100
#define MAX_INVENTORY_ITEMS 1000
#define MAX_PREVIOUS_FOPALS 50
#define MAX_DEPARTMENTS 300
#define MAX_DAILY_SALES 3000
#define MAX_COMMENT_CHAR 150

#define PRINTER_RECEIPT_STANDARD 0

#define QUERY_DELIMITER "^"
#define LIST_DELIMITER ","
#define CSV_INCELL_DELIMITER "."
#define INCOMING_RESULT_DELIMITER "%"
#define PACKAGE_DELIMITER "#"
#define RESULT_MEMBER_DELIMITER "|"
#define EMPTY_DB_ENTRY "NA"
#define FOPAL_LIST_DELIMITER "_"
#define DEAL_DELIMITER "="
#define SERVER_RECV_DELIMITER '\n'
#define SERVER_SEND_DELIMITER "~"

#define STANDARD_DATE_FORMAT   "yyyy-MM-dd hh:mm:ss"
#define STANDARD_DATE_FORMAT_2 "yyyy-MM-dd"

#define MESSAGE_OKAY_ONLY "0"
#define MESSAGE_CANCEL_ONLY "1"
#define MESSAGE_OKAY_CANCEL "2"

#define ERR_INCOMPLETE_FORM "Incomplete Form"

#define ACCOUNT_IT "7005"
#define ACCOUNT_COPY "7030"
#define ACCOUNT_ILL "5531"
#define ACCOUNT_LIB "5530"

#define RECEIPT_IT_CODE "IT"
#define RECEIPT_ILL_CODE "ILL"
#define RECEIPT_LIB_CODE "Library"
#define RECEIPT_COPY_CODE "COPY"
#define RECEIPT_CASH "CASH"
#define RECEIPT_CHECK "CHECK"
#define RECEIPT_DAILY_SALE "DAILY RECEIPT"
#define RECEIPT_CHARGE_BACK "CHARGE BACK"

#define RECEIPT_FILE "C:\\Program Files\\JBSalesSystem\\Receipts\\"
#define BUTTON_SOUND_FOLDER "C:\\Program Files\\JBSalesSystem\\sounds\\"
#define BUTTON_CHOOSE "buttonClick.wav"
#define BUTTON_UNDO "undoButton.wav"
#define PLAY_CHOOSE_BUTTON 1
#define PLAY_UNDO_BUTTON 2

#define QUICKSALE_GREEN_SCANTRON 1
#define QUICKSALE_FIVE_GREEN_SCANTRONS 2
#define QUICKSALE_BLUEBOOK 3
#define QUICKSALE_RED_SCANTRON 4
#define QUICKSALE_FIVE_RED_SCANTRONS 5
#define QUICKSALE_FOLDER 6
#define QUICKSALE_REG_PENCIL 7
#define QUICKSALE_M_PENCIL 8
#define QUICKSALE_PEN 9
#define QUICKSALE_FIVE_DOLLAR_EARBUDS 10
#define QUICKSALE_THREE_DOLLAR_EARBUDS 11
#define QUICKSALE_SHARPIE 12
#define QUICKSALE_ERASER 13
#define QUICKSALE_DRY_ERASE_MARKER 14
#define QUICKSALE_HIGHLIGHTER 15
#define QUICKSALE_PADDED_ENVELOPE 16
#define QUICKSALE_TAPE_FLAGS 17
#define QUICKSALE_PENCILREFIL 18
#define QUICKSALE_PENINK 19
#define QUICKSALE_ICARD_CL_L 20
#define QUICKSALE_ICARD_R_L 21
#define QUICKSALE_ICARD_CL_S 22
#define QUICKSALE_ICARD_R_S 23
#define QUICKSALE_UNDO 24

#define NUMBER_OF_QUICKSALES 23

#define TIMER_DAILY_RECEIPTS 300000  // 5 Minutes
