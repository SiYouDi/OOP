#pragma once
#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include "../sqlite3/sqlite3.h"
#include "Goods.h"

class Viewer {
public:
    bool goodsCheck(sqlite3* db);
    bool goodsCreate(sqlite3* db);
    bool customerCreate(sqlite3* db);
    bool shoppingCartCreate(sqlite3* db);
    bool goodsShow(sqlite3* db);
    bool goodsFind(sqlite3* db, std::string text);
    bool goodsFind(sqlite3* db, int num);
    Goods getGoodsById(sqlite3* db, int goodsId);
};

class User : public Viewer {
protected:
    std::string type;
    std::string account;
    std::string password;

public:
    User(std::string tp, std::string uname, std::string pwd);
    std::string getUserName() const;
    bool checkPassword(const std::string& pwd) const;
    void changePassword(const std::string& newPwd);
};

class Admin : public User {
public:
    Admin();
    bool goodsAdd(sqlite3* db);
    bool goodsDel(sqlite3* db);
    bool goodsUpdate(sqlite3* db);
};

class Customer : public User {
private:
    int money;
    std::vector<cartGoods> shoppingCart;

public:
    Customer(std::string uname, std::string pwd);
    bool investMoney(int num);
    bool cartInit(sqlite3* db);
    void cartAdd(sqlite3* db, Goods goods);
    bool cartSet(sqlite3* db, Goods goods, int setNum);
    bool cartDel(sqlite3* db, Goods goods);
    bool cartShow();
    bool purchase(sqlite3* db);
};

#endif // USER_H