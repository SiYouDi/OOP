#include "Goods.h"
#include "../sqlite3/sqlite3.h"
#include <time.h>
#include <iostream>

// Goods��ʵ��
Goods::Goods(int id, std::string type, std::string name, int price, std::string text, int stock)
    : id(id), type(type), name(name), price(price), text(text), stock(stock) {}

int Goods::getGoodsId() const { return id; }
std::string Goods::getGoodsType() const { return type; }
std::string Goods::getGoodsName() const { return name; }
int Goods::getGoodsPrice() const { return price; }
std::string Goods::getGoodsText() const { return text; }
int Goods::getGoodsStock() const { return stock; }

bool Goods::operator==(const Goods &other) const
{
    return (id == other.id) && (name == other.name) && (price == other.price) && (text == other.text);
}

// cartGoods��ʵ��
cartGoods::cartGoods(Goods goods, int num, int totalPrice)
    : goods(goods), num(num), totalPrice(totalPrice) {}

Goods cartGoods::getGoods() const { return goods; }
int cartGoods::getNum() const { return num; }
int cartGoods::getTotalPrice() { return totalPrice; }

int cartGoods::setNum(int setnum)
{
    num = setnum >= 0 ? setnum : 0;
    totalPrice = goods.getGoodsPrice() * num;
    return num;
}

bool cartGoods::operator==(const cartGoods &other) const
{
    return (goods == other.goods) && (num == other.num) && (totalPrice == other.totalPrice);
}

double Goods::getCurrentPrice(sqlite3 *db) const
{
    const char *sql = "SELECT p.type, p.discount_rate FROM promotions p "
                      "JOIN goods_promotions gp ON p.id = gp.promotion_id "
                      "WHERE gp.goods_id = ? AND p.start_time <= ? AND p.end_time >= ?;";

    sqlite3_stmt *stmt;
    time_t now = time(nullptr);
    double finalPrice = price;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_bind_int64(stmt, 2, now);
        sqlite3_bind_int64(stmt, 3, now);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int type = sqlite3_column_int(stmt, 0);
            if (type == 1)
            { // �ۿ�����
                double rate = sqlite3_column_double(stmt, 1);
                finalPrice = price * rate;
                break; // ֻӦ�õ�һ���ҵ�����Ч�ۿ�
            }
        }
        sqlite3_finalize(stmt);
    }
    return finalPrice;
}

std::string Goods::getPromotionInfo(sqlite3 *db) const
{
    std::cerr << "Getting promotion info for goods ID: " << id << std::endl;
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return "";
    }

    const char *sql = "SELECT p.type, p.discount_rate, p.min_amount, p.discount_amount, p.name "
                      "FROM promotions p JOIN goods_promotions gp ON p.id = gp.promotion_id "
                      "WHERE gp.goods_id = ? AND p.start_time <= ? AND p.end_time >= ?;";

    sqlite3_stmt *stmt;
    time_t now = time(nullptr);
    std::string info;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return "";
    }

    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_int64(stmt, 2, now);
    sqlite3_bind_int64(stmt, 3, now);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int type = sqlite3_column_int(stmt, 0);
        const char *name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));

        if (type == 1)
        { // �ۿ�
            double rate = sqlite3_column_double(stmt, 1);
            info = "[�ۿ�:" + std::to_string((int)(rate * 100)) + "�� " + std::string(name) + "]";
        }
        else if (type == 2)
        { // ����
            int min = sqlite3_column_int(stmt, 2);
            int discount = sqlite3_column_int(stmt, 3);
            info = "[����:��" + std::to_string(min) + "��" + std::to_string(discount) + " " + std::string(name) + "]";
        }
    }
    else
    {
        std::cerr << "No promotions found for goods ID: " << id << std::endl;
    }

    sqlite3_finalize(stmt);
    return info;
}