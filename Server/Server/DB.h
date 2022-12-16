#include <sqlext.h>  
#define NAME_LEN 50  
#define PHONE_LEN 60

class DB {
private:
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt = 0;// sql 명령을 저장하고 핸들을 사용해서 
    SQLRETURN retcode; // 리턴코드를 저장할 변수 

public:
    DB();
    ~DB();
    void show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
    bool update_db(char* name, short& xPos, short& yPos, short& level, int& hp, int& exp);
    bool check_id(char* name, short& xPos, short& yPos, short& level, int& hp, int& exp);
    bool add_user(char* name, short& xPos, short& yPos, short& level, int& hp, int& exp);
};