#include "SQLite/SQLite.h"
#include "SQLite/Field.h"
#include "Account.h"
#include <iostream>

using namespace std;
namespace bs = beesoft::sqlite;

const char* const CreateAccount =
        "CREATE TABLE account ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL COLLATE NOCASE,"
        "is_default INTEGER NOT NULL CHECK(is_default=0 OR is_default=1)"
        ")";
const char* const CreatePOP3 =
        "CREATE TABLE pop3 ("
        "id INTEGER PRIMARY KEY,"
        "account_id INTEGER NOT NULL,"
        "user TEXT NOT NULL,"
        "password TEXT NOT NULL,"
        "server TEXT NOT NULL,"
        "port INTEGER NOT NULL,"
        "tls INTEGER NOT NULL CHECK(tls=0 OR tls=1) DEFAULT 0,"
        "FOREIGN KEY (account_id) REFERENCES account(id) ON UPDATE RESTRICT ON DELETE RESTRICT"
        ")";
const char* const CreateEmail =
        "CREATE TABLE email("
        "id INTEGER PRIMARY KEY,"
        "account_id INTEGER NOT NULL,"
        // datime: data i godzina napisania lub odebrania z serwera
        "datime REAL NOT NULL,"
        // created: 0 - ktoś przysłal, 1 - ja napisałem,
        "created INTEGER NOT NULL CHECK(created=0 OR created=1),"
        "title TEXT NOT NULL COLLATE NOCASE,"
        "sender_from TEXT NOT NULL COLLATE NOCASE,"
        "recipient_to TEXT NOT NULL COLLATE NOCASE,"
        "status INTEGER NOT NULL DEFAULT 0,"
        "FOREIGN KEY (account_id) REFERENCES account(id) ON DELETE RESTRICT ON UPDATE RESTRICT"
        ")";
const char* const CreateEmailIndex =
        "CREATE INDEX email_created ON email(created);"
        "CREATE INDEX email_account ON email(account_id)";


const char* const CreateEmailContent =
        "CREATE TABLE content("
        "id INTEGER PRIMARY KEY,"
        "email_id INTEGER NOT NULL,"
        "type INTEGER NOT NULL,"
        "content BLOB NOT NULL,"
        "FOREIGN KEY (email_id) REFERENCES email(id) ON UPDATE RESTRICT ON DELETE RESTRICT"
        ")";

/*
 * Status e-maila
 * 0 - active
 * 1 - deleted
 * 2 - spam
 */

/*
 * Type
 * 0 - plain text
 * 1 - html
 */
int main() {

    if (const auto ok = bs::SQLite::shared().open("/home/piotr/.postman/postman.sqlite"); ok) {
        cout << "database opened" << endl;
        const auto result = bs::SQLite::shared().select("SELECT * FROM account");
        if (result.size()) {
            cout << result.size() << endl;
            for (int row = 0; row < int(result.size()); row++) {
                const auto a = Account(result[row]);
                cout << a << endl;
            }
        }
    }


/*
    auto ok = bs::SQLite::shared().create("/home/piotr/.postman/postman.sqlite", [](bs::SQLite& sql) -> bool {
        return
            sql.exec(CreateAccount)      &&
            sql.exec(CreatePOP3)         &&
            sql.exec(CreateEmail)        &&
            sql.exec(CreateEmailIndex)   &&
            sql.exec(CreateEmailContent);

    }, true);
    cout << "database created: " << (ok ? "success" : "error") << endl ;

    Account acc0("piotrek");
    cout << acc0 << endl;
    ok = acc0.save();
    cout << (ok ? "success" : "failed") << endl;
    cout << acc0 << endl;
    cout << "-------------------------------" << endl;

    Account acc1("artur", true);
    cout << acc1 << endl;
    ok = acc1.save();
    cout << (ok ? "success" : "failed") << endl;
    cout << acc1 << endl;
    cout << "-------------------------------" << endl;

    Account acc2("Jolanta", false);
    cout << acc2 << endl;
    ok = acc2.save();
    cout << (ok ? "success" : "failed") << endl;
    cout << acc2 << endl << endl;

    acc2.name("Jola Maria");
    acc2.is_default(true);
    cout << acc2 << endl;
    ok = acc2.save();
    cout << (ok ? "success" : "failed") << endl;


    cout << "======================================================" << endl;
    {
        auto f = bs::Field("NULL");
        cout << f << endl;
    }
    {
        auto f1 = bs::Field("v1", bs::i64(54321));
        cout << f1.as_i64() << endl;
    }
    {
        auto f2 = bs::Field("v2", bs::f64(3.1415));
        auto f3 = bs::Field("v3", false);
        auto f4 = bs::Field("v4", bs::text("piotr Włodzimierz Pszczółkowski"));

        char arr[5] {0x02, 0x04, 0x06, 0x08, 0x0a};
        auto f5 = bs::Field("v5", &arr[0], 5);
        std::vector<char> arr2 {0x02, 0x04, 0x06, 0x08, 0x0a};
        auto f6 = bs::Field("v6", arr2);


        cout << f2.as_f64() << endl;
        cout << (f3.as_bool() ? "true" : "false") << endl;
        cout << f4.as_text() << endl;

        auto v = f5.as_vector();
        for (unsigned int i = 0; i < v.size(); i++) {
            printf("0x%02x, ", v[i]);
        }
        printf("\n");

        auto v2 = f6.as_vector();
        for (unsigned int i = 0; i < v2.size(); i++) {
            printf("0x%02x, ", v2[i]);
        }
        printf("\n");

    }


    {
        auto f1 = bs::Field("v1", bs::i64(54321));
        cout << f1.as_i64() << endl;
        f1.value(bs::f64(5.1413));
        cout << f1.as_f64() << endl;
    }
    */

    /*
    auto w3 = bs::Field("empty");
    cout << w3 << endl;
    {
        auto w1 = bs::Field("w1", bs::i64(98765));
        cout << w1 << endl;
        auto w2 = w1;
        cout << w1 << ", " << w2 << endl;
        w3 = w2;
        cout << w1 << ", " << w2 << ", " << w3 << endl;
    }
    cout << w3 << endl;
    */

    return 0;
}
