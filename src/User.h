#pragma once
#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include "../sqlite3/sqlite3.h"
#include "Goods.h"
#include "Order.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>

class Viewer {
public:
    bool goodsCheck(sqlite3* db);
    bool goodsCreate(sqlite3* db);
    bool customerCreate(sqlite3* db);
    bool shoppingCartCreate(sqlite3* db);
    bool goodsShow(sqlite3* db);
    bool goodsFind(sqlite3* db, std::string text);
    bool goodsFind(sqlite3* db, int num);
    static Goods getGoodsByIdStatic(sqlite3 *db, int goodsId);
    bool orderCreate(sqlite3 *db);
    bool promotionCreate(sqlite3 *db);
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
    bool addPromotion(sqlite3 *db);
    bool deletePromotion(sqlite3 *db);
    bool assignPromotionToGoods(sqlite3 *db);
    bool showPromotions(sqlite3 *db);
};

class Customer : public User {
private:
    int money;
    std::vector<cartGoods> shoppingCart;
    std::vector<Order> orders;

public:
    Customer(std::string uname, std::string pwd);
    bool investMoney(int num);
    bool cartInit(sqlite3* db);
    void cartAdd(sqlite3* db, Goods goods);
    bool cartSet(sqlite3* db, Goods goods, int setNum);
    bool cartDel(sqlite3* db, Goods goods);
    bool cartShow();
    bool cartClr(sqlite3 *db);
    void purchase(sqlite3 *db);
    bool cancelOrder(sqlite3 *db, int orderId);
    void updateOrderStatus(sqlite3 *db, int orderId, OrderStatus status);
    void showOrders(sqlite3 *db);
    bool modifyOrderAddress(sqlite3 *db, int orderId, const std::string &newAddr);
    bool deleteReceivedOrder(sqlite3 *db, int orderId);
    void generatePurchaseStatistics(sqlite3* db);
};

#endif // USER_H