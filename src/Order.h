// Order.h
#ifndef ORDER_H
#define ORDER_H

#include "Goods.h"
#include <string>
#include <vector>
#include <ctime>
#include <sqlite3.h>

enum OrderStatus
{
    PENDING,
    SHIPPED,
    RECEIVED,
    CANCELLED // 新增取消状态
};

class Order
{
private:
    static int nextId;
    int orderId;
    std::vector<cartGoods> items;
    time_t createTime;
    std::string address;
    OrderStatus status;
    int totalAmount;

public:
    Order(std::vector<cartGoods> goodsList, std::string addr);
    Order(int id, const std::string &goodsIds, const std::string &quantities,int total, const std::string &addr, int status, time_t createTime);

    // Getter方法
    int getOrderId() const { return orderId; }
    std::string getStatusString() const;
    std::string getCreateTime() const;
    void showOrderDetails() const;

    void setStatus(OrderStatus newStatus) { status = newStatus; }

    // 状态操作
    bool cancelOrder(sqlite3 *db, const std::string &username);
    void updateStatus(sqlite3 *db, OrderStatus newStatus);
    bool modifyAddress(sqlite3 *db, const std::string &newAddr);

    // 数据库操作
    bool saveToDB(sqlite3 *db, const std::string &username);
    static std::vector<Order> loadFromDB(sqlite3 *db, const std::string &username);

    int getTotalAmount() const { return totalAmount; }
    OrderStatus getStatus() const { return status; }
    int calculateTotalWithPromotions(sqlite3 *db);
    static std::vector<int> parseStringToList(const std::string &str);

private:
    bool updateStock(sqlite3 *db, int delta) const;
    int applyFullReduction(sqlite3 *db, int total); // 应用满减
};

#endif