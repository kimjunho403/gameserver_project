#include <sqlext.h>  
#define NAME_LEN 50  
#define PHONE_LEN 60

class DB {
private:
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt = 0;// sql ����� �����ϰ� �ڵ��� ����ؼ� 
    SQLRETURN retcode; // �����ڵ带 ������ ���� 

public:
    mutex db_l;
    DB();
    ~DB();
    void show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
    bool update_db(char* name, short& xPos, short& yPos, short& level, int& hp, int& exp);
    bool load_info(char* name, short& xPos, short& yPos, short& level, int& hp, int& exp);
    bool add_user(char* name);
    bool exist_id(char* name, int* is_exist);
};