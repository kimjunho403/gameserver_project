#include "global.h"  
#include "DB.h"

using namespace std;

DB::DB()
{
    setlocale(LC_ALL, "korean");

    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
    retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2017182010DSN", SQL_NTS, (SQLWCHAR*)NULL, SQL_NTS, NULL, SQL_NTS);
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        cout << "DB Connect\n";
    }
    else {
        cout << "DB Connect Fail \n";
    }
}

DB::~DB()
{
    SQLCancel(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void DB::show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
    SQLSMALLINT iRec = 0;
    SQLINTEGER iError;
    WCHAR wszMessage[1000];
    WCHAR wszState[SQL_SQLSTATE_SIZE + 1];

    if (RetCode == SQL_INVALID_HANDLE) {
        fwprintf(stderr, L"Invalid handle!\n");
        return;
    }

    while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
        (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
        // Hide data truncated..
        if (wcsncmp(wszState, L"01004", 5)) {
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
        }
    }

}

bool DB::update_db(char* name, short& xPos, short& yPos, short& level, int& hp, int& exp)
{
    wchar_t exec[256];
    wchar_t wname[16];
    size_t len;
    mbstowcs_s(&len, wname, 16, name, 16);
    wsprintf(exec, L"EXEC update_info %ls, %ls, %ls, %ls, %ls, %ls", wname, to_wstring(xPos), to_wstring(yPos), to_wstring(level), to_wstring(hp), to_wstring(exp));
    wcout << exec << endl;


    /*  wstring temp(&name[0],&name[sizeof(name)]);
      wstring qu{ L"EXEC update_info "  + temp };
      qu += L", " + to_wstring(xPos) + L", " + to_wstring(yPos);*/
      /* qu += tmp;
       qu += L", " + to_wstring(xPos) + L", " + to_wstring(yPos);*/
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    retcode = SQLExecDirect(hstmt, (SQLWCHAR*)exec, SQL_NTS);
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        std::cout << "[LOG] Update " << name << "'s Information Successful!" << std::endl;
      

        if (retcode == SQL_ERROR)
            show_error(hstmt, SQL_HANDLE_STMT, retcode);

        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
         
        }

        return true;

    }
    else {
        show_error(hstmt, SQL_HANDLE_STMT, retcode);
    }
    return false;
}

bool DB::load_info(char* name, short& xPos, short& yPos,short& level,int& hp,int& exp)
{
    wstring qu{};
    qu += L"EXEC user_info ";

    wstring tmp{};
    tmp.assign(&name[0], &name[sizeof(name)]);
    qu += tmp;
    SQLWCHAR szName[NAME_LEN];
    SQLINTEGER user_xPos, user_yPos, user_level, user_hp, user_exp;
    SQLLEN cbxPos = 0, cbyPos = 0, cb_id = 0, cb_level = 0, cb_hp = 0, cb_exp = 0;
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    retcode = SQLExecDirect(hstmt, (SQLWCHAR*)qu.c_str(), SQL_NTS);
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        // Bind columns 1, 2, and 3  
        retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &szName, NAME_LEN, &cb_id);
        retcode = SQLBindCol(hstmt, 2, SQL_INTEGER, &user_xPos, 4, &cbxPos);
        retcode = SQLBindCol(hstmt, 3, SQL_INTEGER, &user_yPos, 4, &cbyPos);
        retcode = SQLBindCol(hstmt, 4, SQL_INTEGER, &user_level, 4, &cb_level);
        retcode = SQLBindCol(hstmt, 5, SQL_INTEGER, &user_hp, 4, &cb_hp);
        retcode = SQLBindCol(hstmt, 6, SQL_INTEGER, &user_exp, 4, &cb_exp);
        // Fetch and print each row of data. On an error, display a message and exit.  
        retcode = SQLFetch(hstmt);
        if (retcode == SQL_ERROR) {
            show_error(hstmt, SQL_HANDLE_STMT, retcode);
        }

        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
        {
            xPos = user_xPos;
            yPos = user_yPos;
            level = user_level;
            hp = user_hp;
            exp = user_exp;
            
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    
            return true;
        }
    }
    else {
        show_error(hstmt, SQL_HANDLE_STMT, retcode);
    }
    return false;

}

bool DB::add_user(char* name)
{
    wstring qu{};
    qu += L"EXEC add_newuser ";

    wstring tmp{};
    tmp.assign(&name[0], &name[sizeof(name)]);
    qu += tmp;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    retcode = SQLExecDirect(hstmt, (SQLWCHAR*)qu.c_str(), SQL_NTS);

    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
        return true;
    else
        return false;

  /*  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

        if (retcode == SQL_ERROR)
            show_error(hstmt, SQL_HANDLE_STMT, retcode);

        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        
        }

        return true;

    }
    else {
        show_error(hstmt, SQL_HANDLE_STMT, retcode);
    }

    return false;*/
}

bool DB::exist_id(char* name, int* is_exist)
{
    wstring qu{};
    qu += L"EXEC check_id ";

    wstring tmp{};
    tmp.assign(&name[0], &name[sizeof(name)]);
    qu += tmp;

    SQLINTEGER nCount;
    SQLLEN cbCount = 0;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    retcode = SQLExecDirect(hstmt, (SQLWCHAR*)qu.c_str(), SQL_NTS);
  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
    {
        retcode = SQLBindCol(hstmt, 1, SQL_INTEGER, &nCount, 4, &cbCount);
      
        retcode = SQLFetch(hstmt);	//SQLFetch를 사용해서 리턴값을 받는다
        if (retcode == SQL_ERROR) {
      
            show_error(hstmt, SQL_HANDLE_STMT, retcode);
        }
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
        {
          
            *is_exist = nCount;
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            return true;
        }
        else
        {
            return false;
        }
    }
    else {
        show_error(hstmt, SQL_HANDLE_STMT, retcode);
    }
    return false;
}
