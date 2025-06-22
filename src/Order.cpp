// Order.cpp
#include "Order.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "User.h"
#include <iostream>

int Order::nextId = 1;

// 新建订单构造函数
Order::Order(std::vector<cartGoods> goodsList, std::string addr)
    : orderId(nextId++), items(goodsList), address(addr), status(PENDING)
{
    time(&createTime);
    totalAmount = 0;
    for (auto &item : items)
    {
        totalAmount += item.getTotalPrice();
    }
}

// 从数据库加载构造
Order::Order(int id, const std::string &goodsIds, const std::string &quantities,
             int total, const std::string &addr, int st, time_t ct)
    : orderId(id), totalAmount(total), address(addr), createTime(ct)
{
    status = static_cast<OrderStatus>(st);

    std::vector<int> gIds = parseStringToList(goodsIds);
    std::vector<int> qts = parseStringToList(quantities);

    for (size_t i = 0; i < gIds.size(); ++i)
    {
        Goods g = Viewer::getGoodsByIdStatic(nullptr, gIds[i]); // 需要静态方法
        items.emplace_back(g, qts[i], g.getGoodsPrice() * qts[i]);
    }
}

void Order::showOrderDetails() const
{
    std::cout << "\n=== 订单详情 ==="
              << "\n订单号: " << orderId
              << "\n创建时间: " << getCreateTime()
              << "\n收货地址: " << address
              << "\n状态: " << getStatusString()
              << "\n商品列表:";

    for (const auto &item : items)
    {
        Goods g = item.getGoods();
        std::cout << "\n"
                  << g.getGoodsName()
                  << "\t单价:" << g.getGoodsPrice()
                  << "\t数量:" << item.getNum();
    }
    std::cout << "\n总金额: " << totalAmount << std::endl;
}

// 状态字符串转换
std::string Order::getStatusString() const
{
    switch (status)
    {
    case PENDING:
        return "待发货";
    case SHIPPED:
        return "已发货";
    case RECEIVED:
        return "已签收";
    default:
        return "未知状态";
    }
}

// 格式化时间
std::string Order::getCreateTime() const
{
    std::tm *ptm = localtime(&createTime);
    char buffer[32];
    strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", ptm);
    return buffer;
}

// 取消订单（库存回滚）
bool Order::cancelOrder(sqlite3 *db, const std::string &username)
{
    if (status != PENDING)
    {
        std::cout << "只有待处理订单可以取消" << std::endl;
        return false;
    }

    status = CANCELLED;

    // updateStock(db, 1); // 回滚库存
    // status = RECEIVED;  // 标记为已取消

    const char *sql = "DELETE FROM orders WHERE orderId = ? AND username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, orderId);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// 修改地址（仅限待发货）
bool Order::modifyAddress(sqlite3 *db, const std::string &newAddr)
{
    if (status != PENDING)
    {
        std::cout << "仅可修改待发货订单地址！" << std::endl;
        return false;
    }

    address = newAddr;
    const char *sql = "UPDATE orders SET address = ? WHERE orderId = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, orderId);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool Order::saveToDB(sqlite3 *db, const std::string &username)
{
    // 验证数据库连接有效性
    if (!db)
    {
        std::cerr << "无效的数据库连接" << std::endl;
        return false;
    }

    // 开始事务
    char *errMsg = nullptr;
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "事务开始失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // 准备商品ID和数量字符串
    std::string goodsIds, quantities;
    for (const auto &item : items)
    {
        goodsIds += std::to_string(item.getGoods().getGoodsId()) + ",";
        quantities += std::to_string(item.getNum()) + ",";
    }

    // 移除末尾多余的逗号
    if (!goodsIds.empty())
        goodsIds.pop_back();
    if (!quantities.empty())
        quantities.pop_back();

    // 使用显式列名的SQL语句（推荐）
    const char *sql = R"(
    INSERT INTO orders (
        username, goodsIds, quantities, 
        totalAmount, address, status, createTime
    ) VALUES (?,?,?,?,?,?,?);
    )";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "准备语句失败: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr); // 回滚
        return false;
    }

    // 参数绑定（注意索引从1开始）
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, goodsIds.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, quantities.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, totalAmount);
    sqlite3_bind_text(stmt, 5, address.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, static_cast<int>(status));
    sqlite3_bind_int64(stmt, 7, static_cast<sqlite3_int64>(createTime));

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        std::cerr << "订单保存失败: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr); // 回滚
        return false;
    }

    if (!updateStock(db, -1))
    {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr); // 回滚
        return false;
    }

    // 提交事务
    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "事务提交失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}
std::vector<int> Order::parseStringToList(const std::string &str)
{
    std::vector<int> result;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ','))
    { // 明确按逗号分割
        if (!token.empty())
        {
            try
            {
                result.push_back(std::stoi(token));
            }
            catch (const std::exception &e)
            {
                std::cerr << "解析错误: " << token << std::endl;
            }
        }
    }
    return result;
}

bool Order::updateStock(sqlite3 *db, int offset) const
{
    for (const auto &item : items)
    {
        const Goods &goods = item.getGoods();
        int newStock = goods.getGoodsStock() + offset * item.getNum();

        const char *sql = "UPDATE goods SET stock = ? WHERE id = ?;";
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        sqlite3_bind_int(stmt, 1, newStock);
        sqlite3_bind_int(stmt, 2, goods.getGoodsId());

        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE)
        {
            std::cerr << "库存更新失败: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr); // 回滚
            return false;
        }
    }
    return true;
}

std::vector<Order> Order::loadFromDB(sqlite3 *db, const std::string &username)
{
    std::vector<Order> orders;
    std::string sql = "SELECT * FROM orders";
    if (!username.empty())
    {
        sql += " WHERE username = ?;";
    }
    else
    {
        sql += ";"; // 管理员加载全部订单
    }
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return orders;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char *goodsIdsStr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        const char *quantitiesStr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        int total = sqlite3_column_int(stmt, 4);
        const char *addr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        int status = sqlite3_column_int(stmt, 6);
        time_t createTime = sqlite3_column_int64(stmt, 7);

        Order order(id, goodsIdsStr, quantitiesStr, total, addr, status, createTime);
        orders.push_back(order);
    }

    sqlite3_finalize(stmt);
    return orders;
}

int Order::calculateTotalWithPromotions(sqlite3 *db)
{
    totalAmount = 0;
    for (auto &item : items)
    {
        Goods g = item.getGoods();
        totalAmount += g.getCurrentPrice(db) * item.getNum();
    }
    totalAmount = applyFullReduction(db, totalAmount);
    return totalAmount;
}

int Order::applyFullReduction(sqlite3 *db, int total)
{
    const char *sql = "SELECT min_amount, discount_amount FROM promotions "
                      "WHERE type = 2 AND start_time <= ? AND end_time >= ? "
                      "ORDER BY min_amount DESC;";

    sqlite3_stmt *stmt;
    time_t now = time(nullptr);
    int finalAmount = total;

    std::cerr << "SQL: " << sql << std::endl;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return total;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int64(stmt, 1, now);
        sqlite3_bind_int64(stmt, 2, now);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int min = sqlite3_column_int(stmt, 0);
            int discount = sqlite3_column_int(stmt, 1);

            if (total >= min)
            {
                finalAmount -= discount;
                break; // 只应用最高档的满减
            }
        }
        sqlite3_finalize(stmt);
    }
    return finalAmount;
}