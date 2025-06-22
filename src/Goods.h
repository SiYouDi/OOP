#pragma once
#ifndef GOODS_H
#define GOODS_H

#include <string>
#include "../sqlite3/sqlite3.h"

class Goods
{
private:
    int id;
    std::string type;
    std::string name;
    int price;
    std::string text;
    int stock;

public:
    Goods(int id, std::string type, std::string name, int price, std::string text, int stock = 0);

    // Getter方法
    int getGoodsId() const;
    std::string getGoodsType() const;
    std::string getGoodsName() const;
    int getGoodsPrice() const;
    std::string getGoodsText() const;
    int getGoodsStock() const;

    bool operator==(const Goods &other) const;

    // 促销相关方法
    double getCurrentPrice(sqlite3 *db) const;       // 获取当前促销价
    bool hasPromotion(sqlite3 *db) const;            // 是否有促销活动
    std::string getPromotionInfo(sqlite3 *db) const; // 获取促销信息
};

class cartGoods
{
private:
    Goods goods;
    int num;
    int totalPrice;

public:
    cartGoods(Goods goods, int num, int totalPrice);

    // Getter方法
    Goods getGoods() const;
    int getNum() const;
    int getTotalPrice();

    // Setter方法
    int setNum(int setnum);

    bool operator==(const cartGoods &other) const;
};

#endif // GOODS_H