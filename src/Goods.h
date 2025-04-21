#pragma once
#ifndef GOODS_H
#define GOODS_H

#include <string>

class Goods {
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

    bool operator==(const Goods& other) const;
};

class cartGoods {
private:
    Goods goods;
    int num;
    int totalPrice;

public:
    cartGoods(Goods goods, int num, int totalPrice);

    // Getter方法
    Goods getGoods();
    int getNum();
    int getTotalPrice();

    // Setter方法
    int setNum(int setnum);

    bool operator==(const cartGoods& other) const;
};

#endif // GOODS_H