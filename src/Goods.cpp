#include "Goods.h"

// Goods类实现
Goods::Goods(int id, std::string type, std::string name, int price, std::string text, int stock)
    : id(id), type(type), name(name), price(price), text(text), stock(stock) {}

int Goods::getGoodsId() const { return id; }
std::string Goods::getGoodsType() const { return type; }
std::string Goods::getGoodsName() const { return name; }
int Goods::getGoodsPrice() const { return price; }
std::string Goods::getGoodsText() const { return text; }
int Goods::getGoodsStock() const { return stock; }

bool Goods::operator==(const Goods& other) const {
    return (id == other.id) && (name == other.name) && (price == other.price) && (text == other.text);
}

// cartGoods类实现
cartGoods::cartGoods(Goods goods, int num, int totalPrice)
    : goods(goods), num(num), totalPrice(totalPrice) {}

Goods cartGoods::getGoods() { return goods; }
int cartGoods::getNum() { return num; }
int cartGoods::getTotalPrice() { return totalPrice; }

int cartGoods::setNum(int setnum) {
    num = setnum >= 0 ? setnum : 0;
    totalPrice = goods.getGoodsPrice() * num;
    return num;
}

bool cartGoods::operator==(const cartGoods& other) const {
    return (goods == other.goods) && (num == other.num) && (totalPrice == other.totalPrice);
}