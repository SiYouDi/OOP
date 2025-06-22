#include "User.h"
#include <map>

time_t parseDate(const std::string &dateStr)
{
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    return mktime(&tm);
}

// Viewer implementation
bool Viewer::goodsCheck(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char *checkTableSQL = "SELECT name FROM sqlite_master WHERE type='table' AND name='goods';";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, checkTableSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        std::cerr << "Table 'goods' doesn't exist or is empty!" << std::endl;
        return false;
    }
    sqlite3_finalize(stmt);

    const char *checkContentSQL = "SELECT COUNT(*) FROM goods;";
    rc = sqlite3_prepare_v2(db, checkContentSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        std::cerr << "Table 'goods' content is empty!" << std::endl;
        return false;
    }

    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (count == 0)
    {
        std::cerr << "Table 'goods' content is empty!" << std::endl;
        return false;
    }
    return true;
}

bool Viewer::goodsCreate(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char *createTableSQL =
        "CREATE TABLE IF NOT EXISTS goods ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type TEXT NOT NULL,"
        "name TEXT NOT NULL,"
        "price INTEGER NOT NULL,"
        "text TEXT NOT NULL,"
        "stock INTEGER NOT NULL);";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Create table failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Viewer::customerCreate(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char *createTableSQL =
        "CREATE TABLE IF NOT EXISTS customers ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL);";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Create table failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Viewer::shoppingCartCreate(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char *createTableSQL =
        "CREATE TABLE IF NOT EXISTS shopping_cart ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "goods_id INTEGER NOT NULL,"
        "quantity INTEGER NOT NULL,"
        "FOREIGN KEY(goods_id) REFERENCES goods(id),"
        "FOREIGN KEY(username) REFERENCES customers(username));";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Create shopping_cart table failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Viewer::goodsShow(sqlite3 *db)
{
    if (goodsCreate(db))
    {
        std::cout << "Table exists" << std::endl;
        const char *querySQL = "SELECT id, type, name, price, text, stock FROM goods;";
        sqlite3_stmt *stmt;

        int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        std::cout << "Product list:" << std::endl;
        // std::cout << "ID\tType\tName\tPrice\tDescription\tStock" << std::endl;

        // while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        // {
        //     int id = sqlite3_column_int(stmt, 0);
        //     const char *type = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        //     const char *name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        //     int price = sqlite3_column_int(stmt, 3);
        //     const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        //     int stock = sqlite3_column_int(stmt, 5);

        //     std::cout << id << "\t" << type << "\t" << name << "\t" << price << "\t" << text << "\t" << stock << std::endl;
        // }

        std::cout << "ID\tType\tName\tPrice\tPromotion\tStock\tDescription" << std::endl;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            const char *type = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            const char *name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            int price = sqlite3_column_int(stmt, 3);
            const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            int stock = sqlite3_column_int(stmt, 5);

            Goods goods(id, std::string(type), std::string(name), price, std::string(text), stock);

            std::cout << "正在查找折扣信息" << std::endl;
            std::string promoInfo = goods.getPromotionInfo(db);
            if (promoInfo.empty())
            {
                std::cerr << "Failed to get promotion info" << std::endl;
                continue;
            }
            std::cout << promoInfo << "折扣信息查找完成:" << promoInfo << std::endl;

            std::cout << id << "\t" << type << "\t" << name << "\t"
                      << price << "\t" << promoInfo << "\t"
                      << stock << "\t" << text << std::endl;
        }

        if (rc != SQLITE_DONE)
        {
            std::cerr << "Query failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        return true;
    }
    return false;
}

bool Viewer::goodsFind(sqlite3 *db, std::string text)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    if (!goodsCheck(db))
    {
        return false;
    }

    text.erase(0, text.find_first_not_of(" \t\n\r"));
    text.erase(text.find_last_not_of(" \t\n\r") + 1);

    if (text.empty())
    {
        std::cout << "Search content cannot be empty!" << std::endl;
        return false;
    }

    std::cout << "\nChoose matching mode:" << std::endl;
    std::cout << "1. Prefix match (starting with '" << text << "')" << std::endl;
    std::cout << "2. Suffix match (ending with '" << text << "')" << std::endl;
    std::cout << "3. Contains match (containing '" << text << "')" << std::endl;
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore();

    std::string pattern;
    std::string modeDescription;

    switch (choice)
    {
    case 1:
        pattern = text + "%";
        modeDescription = "Prefix match";
        break;
    case 2:
        pattern = "%" + text;
        modeDescription = "Suffix match";
        break;
    case 3:
    default:
        pattern = "%" + text + "%";
        modeDescription = "Contains match";
    }

    const char *querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE "
                           "name LIKE ? OR text LIKE ? OR type LIKE ? "
                           "ORDER BY name;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, pattern.c_str(), -1, SQLITE_STATIC);

    std::cout << "\n========== Search Results (" << modeDescription << ") ==========" << std::endl;
    std::cout << "Search term: \"" << text << "\"" << std::endl;
    std::cout << "Match pattern: " << pattern << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "ID\tType\tName\tPrice\tStock" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    bool found = false;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        found = true;
        int id = sqlite3_column_int(stmt, 0);
        const char *type = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        int price = sqlite3_column_int(stmt, 3);
        const char *desc = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        int stock = sqlite3_column_int(stmt, 5);

        std::cout << id << "\t" << type << "\t" << name << "\t" << price << "\t" << stock << std::endl;
        std::cout << "Description: " << desc << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }

    if (rc != SQLITE_DONE)
    {
        std::cerr << "Query failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    if (!found)
    {
        std::cout << "No matching products found." << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }

    return true;
}

bool Viewer::goodsFind(sqlite3 *db, int num)
{

    if (!goodsCheck(db))
    {
        return false;
    }

    std::cout << "Choose search method:" << std::endl;
    std::cout << "1. Exact price match " << num << std::endl;
    std::cout << "2. Price <= " << num << std::endl;
    std::cout << "3. Price >= " << num << std::endl;
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;

    std::string querySQL;
    switch (choice)
    {
    case 1:
        querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE price = ?";
        break;
    case 2:
        querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE price <= ?";
        break;
    case 3:
        querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE price >= ?";
        break;
    default:
        std::cout << "Invalid choice" << std::endl;
        return false;
    }

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, querySQL.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, num);

    std::cout << "Search results (price: " << num << "):" << std::endl;
    std::cout << "ID\tType\tName\tPrice\tDescription\tStock" << std::endl;

    bool found = false;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        found = true;
        int id = sqlite3_column_int(stmt, 0);
        const char *type = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        int price = sqlite3_column_int(stmt, 3);
        const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        int stock = sqlite3_column_int(stmt, 5);

        std::cout << id << "\t" << type << "\t" << name << "\t" << price << "\t" << text << "\t" << stock << std::endl;
    }

    if (rc != SQLITE_DONE)
    {
        std::cerr << "Query failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    if (!found)
    {
        std::cout << "No products found with price " << num << std::endl;
    }

    return true;
}

bool Viewer::orderCreate(sqlite3 *db)
{
    const char *sql =
        "CREATE TABLE IF NOT EXISTS orders (" // 添加 IF NOT EXISTS 避免重复创建
        "orderId INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT,"
        "goodsIds TEXT,"
        "quantities TEXT,"
        "totalAmount INTEGER,"
        "address TEXT,"
        "status INTEGER,"
        "createTime INTEGER);";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Viewer::promotionCreate(sqlite3 *db)
{
    const char *sql =
        "CREATE TABLE IF NOT EXISTS promotions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "type INTEGER NOT NULL,"   // 1: 折扣, 2: 满减
        "discount_rate REAL,"      // 折扣率(0.8表示8折)
        "min_amount INTEGER,"      // 满减最低金额
        "discount_amount INTEGER," // 满减金额
        "start_time INTEGER,"      // 开始时间(时间戳)
        "end_time INTEGER);";      // 结束时间(时间戳)

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg);
        return false;
    }

    sql =
        "CREATE TABLE IF NOT EXISTS goods_promotions ("
        "goods_id INTEGER NOT NULL,"
        "promotion_id INTEGER NOT NULL,"
        "PRIMARY KEY (goods_id, promotion_id),"
        "FOREIGN KEY (goods_id) REFERENCES goods(id),"
        "FOREIGN KEY (promotion_id) REFERENCES promotions(id));";

    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

static Goods getGoodsByIdStatic(sqlite3 *db, int goodsId)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return Goods(0, "", "", 0, "", 0);
    }

    sqlite3_stmt *stmt;
    const char *querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE id = ?;";
    int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return Goods(0, "", "", 0, "", 0);
    }

    sqlite3_bind_int(stmt, 1, goodsId);

    Goods goods(0, "", "", 0, "", 0);
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char *type = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        int price = sqlite3_column_int(stmt, 3);
        const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        int stock = sqlite3_column_int(stmt, 5);

        goods = Goods(id, std::string(type), std::string(name), price, std::string(text), stock);
    }
    else
    {
        std::cerr << "Product with ID " << goodsId << " not found!" << std::endl;
    }

    sqlite3_finalize(stmt);
    return goods;
}

// User implementation
User::User(std::string tp, std::string uname, std::string pwd)
    : type(tp), account(uname), password(pwd) {}

std::string User::getUserName() const { return account; }
bool User::checkPassword(const std::string &pwd) const { return password == pwd; }
void User::changePassword(const std::string &newPwd) { password = newPwd; }

// Admin implementation
Admin::Admin() : User("admin", "account", "password") {}

bool Admin::goodsAdd(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    std::string type, name, text;
    int price, stock;
    std::cout << "Enter product type, name, price, description, and stock:" << std::endl;
    std::cin >> type >> name >> price >> text >> stock;

    const char *insertSQL = "INSERT INTO goods (type, name, price, text, stock) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, price);
    sqlite3_bind_text(stmt, 4, text.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, stock);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    std::cout << "Product added successfully!" << std::endl;
    return true;
}

bool Admin::goodsDel(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    int id;
    std::cout << "Enter product ID to delete:" << std::endl;
    std::cin >> id;

    const char *deleteSQL = "DELETE FROM goods WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Delete failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    std::cout << "Product deleted successfully!" << std::endl;
    return true;
}

bool Admin::goodsUpdate(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char *checkTableSQL = "SELECT name FROM sqlite_master WHERE type='table' AND name='goods';";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, checkTableSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        std::cerr << "Table 'goods' doesn't exist or is empty!" << std::endl;
        return false;
    }
    sqlite3_finalize(stmt);

    const char *checkContentSQL = "SELECT COUNT(*) FROM goods;";
    rc = sqlite3_prepare_v2(db, checkContentSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        std::cerr << "Table 'goods' content is empty!" << std::endl;
        return false;
    }

    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (count == 0)
    {
        std::cerr << "Table 'goods' content is empty!" << std::endl;
        return false;
    }

    int id;
    std::string type, name, text;
    int price, stock;
    std::cout << "Enter product ID to update:" << std::endl;
    std::cin >> id;
    std::cout << "Enter new product type, name, price, description, and stock:" << std::endl;
    std::cin >> type >> name >> price >> text >> stock;

    const char *updateSQL = "UPDATE goods SET type = ?, name = ?, price = ?, text = ?, stock = ? WHERE id = ?;";
    rc = sqlite3_prepare_v2(db, updateSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, price);
    sqlite3_bind_text(stmt, 4, text.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, stock);
    sqlite3_bind_int(stmt, 6, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Update failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    std::cout << "Product updated successfully!" << std::endl;
    return true;
}

bool Admin::addPromotion(sqlite3 *db)
{
    std::string name;
    int type;
    double discountRate = 0;
    int minAmount = 0, discountAmount = 0;
    time_t startTime, endTime;

    std::cout << "输入促销名称: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    std::cout << "选择促销类型 (1-折扣 2-满减): ";
    std::cin >> type;

    if (type == 1)
    {
        std::cout << "输入折扣率 (如0.8表示8折): ";
        std::cin >> discountRate;
    }
    else if (type == 2)
    {
        std::cout << "输入满减最低金额: ";
        std::cin >> minAmount;
        std::cout << "输入满减金额: ";
        std::cin >> discountAmount;
    }
    else
    {
        std::cout << "无效的促销类型!" << std::endl;
        return false;
    }

    std::cout << "输入开始时间 (YYYY-MM-DD): ";
    std::string startStr;
    std::cin >> startStr;
    startTime = parseDate(startStr);

    std::cout << "输入结束时间 (YYYY-MM-DD): ";
    std::string endStr;
    std::cin >> endStr;
    endTime = parseDate(endStr);

    const char *sql = "INSERT INTO promotions (name, type, discount_rate, min_amount, discount_amount, start_time, end_time) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "SQL准备失败: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, type);
    sqlite3_bind_double(stmt, 3, discountRate);
    sqlite3_bind_int(stmt, 4, minAmount);
    sqlite3_bind_int(stmt, 5, discountAmount);
    sqlite3_bind_int64(stmt, 6, startTime);
    sqlite3_bind_int64(stmt, 7, endTime);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "添加促销失败: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    std::cout << "促销活动添加成功!" << std::endl;
    return true;
}

bool Admin::assignPromotionToGoods(sqlite3 *db)
{
    int goodsId, promotionId;
    std::cout << "输入商品ID: ";
    std::cin >> goodsId;
    std::cout << "输入促销ID: ";
    std::cin >> promotionId;

    const char *sql = "INSERT INTO goods_promotions (goods_id, promotion_id) VALUES (?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "SQL准备失败: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, goodsId);
    sqlite3_bind_int(stmt, 2, promotionId);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "关联促销失败: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    std::cout << "促销活动关联成功!" << std::endl;
    return true;
}

bool Admin::deletePromotion(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    int promotionId;
    std::cout << "Enter promotion ID to delete: ";
    std::cin >> promotionId;

    const char *deleteSQL = "DELETE FROM promotions WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, promotionId);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Delete failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    std::cout << "Promotion deleted successfully!" << std::endl;
    return true;
}

bool Admin::showPromotions(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char *querySQL = "SELECT id, name, type, discount_rate, min_amount, discount_amount, start_time, end_time FROM promotions;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::cout << "Promotions:" << std::endl;
    std::cout << "ID\tName\tType\tDiscount Rate\tMin Amount\tDiscount Amount\tStart Time\tEnd Time" << std::endl;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        int type = sqlite3_column_int(stmt, 2);
        double discountRate = sqlite3_column_double(stmt, 3);
        int minAmount = sqlite3_column_int(stmt, 4);
        int discountAmount = sqlite3_column_int(stmt, 5);
        time_t startTime = sqlite3_column_int64(stmt, 6);
        time_t endTime = sqlite3_column_int64(stmt, 7);

        std::cout << id << "\t" << name << "\t" << type << "\t" << discountRate << "\t" << minAmount << "\t" << discountAmount << "\t" << startTime << "\t" << endTime << std::endl;
    }

    sqlite3_finalize(stmt);
    return true;
}

// Customer implementation
Customer::Customer(std::string uname, std::string pwd)
    : User("Customer", uname, pwd), money(0) {}

bool Customer::investMoney(int num)
{
    money += num;
    return true;
}

bool Customer::cartInit(sqlite3 *db)
{
    if (!db)
    {
        std::cerr << "数据库" << std::endl;
        return false;
    }

    sqlite3_stmt *stmt;
    const char *querySQL = "SELECT goods_id, quantity FROM shopping_cart WHERE username = ?;";
    int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int goodsId = sqlite3_column_int(stmt, 0);
        int quantity = sqlite3_column_int(stmt, 1);

        Goods goods = getGoodsByIdStatic(db, goodsId);
        int num = quantity;
        int totalPrice = goods.getGoodsPrice() * num;
        cartGoods cartgoods = cartGoods(goods, num, totalPrice);
        shoppingCart.push_back(cartgoods);
    }

    sqlite3_finalize(stmt);
    return true;
}

void Customer::cartAdd(sqlite3 *db, Goods goods)
{
    if (goods.getGoodsId() == 0)
    {
        std::cout << "商品不存在，请重新输入" << std::endl;
        return;
    }

    while (true)
    {
        std::cout << "是否继续添加其他商品[y/n]" << std::endl;
        char choice;
        std::cin >> choice;

        while (choice != 'y' && choice != 'n')
        {
            std::cout << "错误输入，请重新输入[y/n]" << std::endl;
            std::cin >> choice;
        }

        if (choice == 'n')
        {
            break;
        }

        bool found = false;
        for (auto &item : shoppingCart)
        {
            if (item.getGoods().getGoodsId() == goods.getGoodsId())
            {
                std::cout << "商品已在购物车中，是否更新数量[y/n]" << std::endl;
                char choice;
                std::cin >> choice;
                if (choice == 'y')
                {
                    item.setNum(item.getNum() + 1);

                    // Update database
                    const char *updateSQL = "UPDATE shopping_cart SET quantity = ? WHERE username = ? AND goods_id = ?;";
                    sqlite3_stmt *stmt;
                    int rc = sqlite3_prepare_v2(db, updateSQL, -1, &stmt, nullptr);
                    if (rc != SQLITE_OK)
                    {
                        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                        return;
                    }
                    sqlite3_bind_int(stmt, 1, item.getNum());
                    sqlite3_bind_text(stmt, 2, getUserName().c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt, 3, goods.getGoodsId());
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                    if (rc != SQLITE_DONE)
                    {
                        std::cerr << "Update failed: " << sqlite3_errmsg(db) << std::endl;
                        return;
                    }
                    std::cout << "更新成功" << std::endl;
                }
                found = true;
                break;
            }
        }

        if (!found)
        {
            int num = 1;
            int totalPrice = num * goods.getGoodsPrice();
            cartGoods cartgoods = cartGoods(goods, num, totalPrice);
            shoppingCart.push_back(cartgoods);

            // Insert into database
            const char *insertSQL = "INSERT INTO shopping_cart (username, goods_id, quantity) VALUES (?, ?, ?);";
            sqlite3_stmt *stmt;
            int rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
            if (rc != SQLITE_OK)
            {
                std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                return;
            }
            sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, goods.getGoodsId());
            sqlite3_bind_int(stmt, 3, num);
            rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            if (rc != SQLITE_DONE)
            {
                std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
                return;
            }
            std::cout << "添加成功" << std::endl;
        }
    }
}

bool Customer::cartSet(sqlite3 *db, Goods goods, int setNum)
{
    try
    {
        bool found = false;
        for (auto &item : shoppingCart)
        {
            if (item.getGoods() == goods)
            {
                if (setNum <= 0)
                {
                    std::cout << "是否确认删除商品?[y/n]" << std::endl;
                    char choice;
                    std::cin >> choice;
                    if (choice == 'y')
                    {
                        auto it = std::find(shoppingCart.begin(), shoppingCart.end(), item);
                        if (it != shoppingCart.end())
                        {
                            shoppingCart.erase(it);

                            // Delete from database
                            const char *deleteSQL = "DELETE FROM shopping_cart WHERE username = ? AND goods_id = ?;";
                            sqlite3_stmt *stmt;
                            int rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
                            if (rc != SQLITE_OK)
                            {
                                std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                                return false;
                            }
                            sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
                            sqlite3_bind_int(stmt, 2, goods.getGoodsId());
                            rc = sqlite3_step(stmt);
                            sqlite3_finalize(stmt);
                            if (rc != SQLITE_DONE)
                            {
                                std::cerr << "Delete failed: " << sqlite3_errmsg(db) << std::endl;
                                return false;
                            }
                        }
                    }
                }
                else
                {
                    item.setNum(setNum);

                    // Update database
                    const char *updateSQL = "UPDATE shopping_cart SET quantity = ? WHERE username = ? AND goods_id = ?;";
                    sqlite3_stmt *stmt;
                    int rc = sqlite3_prepare_v2(db, updateSQL, -1, &stmt, nullptr);
                    if (rc != SQLITE_OK)
                    {
                        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                        return false;
                    }
                    sqlite3_bind_int(stmt, 1, setNum);
                    sqlite3_bind_text(stmt, 2, getUserName().c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt, 3, goods.getGoodsId());
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                    if (rc != SQLITE_DONE)
                    {
                        std::cerr << "Update failed: " << sqlite3_errmsg(db) << std::endl;
                        return false;
                    }
                }
                found = true;
                break;
            }
        }

        if (found)
        {
            std::cout << "修改成功" << std::endl;
        }
        else
        {
            std::cout << "修改失败，商品不存在" << std::endl;
        }
        return found;
    }
    catch (const std::exception &)
    {
        std::cout << "set失败" << std::endl;
        return false;
    }
}

bool Customer::cartDel(sqlite3 *db, Goods goods)
{
    try
    {
        bool found = false;
        for (auto &item : shoppingCart)
        {
            if (item.getGoods() == goods)
            {
                auto it = std::find(shoppingCart.begin(), shoppingCart.end(), item);
                if (it != shoppingCart.end())
                {
                    shoppingCart.erase(it);

                    // Delete from database
                    const char *deleteSQL = "DELETE FROM shopping_cart WHERE username = ? AND goods_id = ?;";
                    sqlite3_stmt *stmt;
                    int rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
                    if (rc != SQLITE_OK)
                    {
                        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                        return false;
                    }
                    sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt, 2, goods.getGoodsId());
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                    if (rc != SQLITE_DONE)
                    {
                        std::cerr << "Delete failed: " << sqlite3_errmsg(db) << std::endl;
                        return false;
                    }
                }
                found = true;
                break;
            }
        }

        if (found)
        {
            std::cout << "删除成功" << std::endl;
        }
        else
        {
            std::cout << "修改失败，删除失败" << std::endl;
        }
        return found;
    }
    catch (const std::exception &)
    {
        std::cout << "del失败" << std::endl;
        return false;
    }
}

bool Customer::cartShow()
{
    try
    {
        if (shoppingCart.empty())
        {
            std::cout << "购物车为空" << std::endl;
            return false;
        }

        std::cout << "\n=== Shopping Cart ===" << std::endl;
        std::cout << "Id\tName\tType\tPrice\tQuantity\tTotal" << std::endl;

        int cartTotalNum = 0;
        int cartTotalPrice = 0;
        for (auto &item : shoppingCart)
        {
            const Goods &goods = item.getGoods();

            std::cout << goods.getGoodsId() << "\t"
                      << goods.getGoodsName() << "\t"
                      << goods.getGoodsType() << "\t"
                      << goods.getGoodsPrice() << "\t"
                      << item.getNum() << "\t\t"
                      << item.getTotalPrice() << std::endl;
            cartTotalPrice += item.getTotalPrice();
            cartTotalNum += item.getNum();
        }

        std::cout << "---------------------------" << std::endl;
        std::cout << "Cart Total Price: " << cartTotalPrice << std::endl;
        std::cout << "Cart Total Num: " << cartTotalNum << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "show失败" << e.what() << std::endl;
        return false;
    }
}

bool Customer::cartClr(sqlite3 *db)
{
    std::cout << "准备清空购物车" << std::endl;
    int sum = 0;
    if (shoppingCart.empty())
    {
        std::cout << "购物车为空" << std::endl;
        return false;
    }

    for (size_t i = 0; i < shoppingCart.size(); i++)
    {
        sum += shoppingCart[i].getGoods().getGoodsPrice() * shoppingCart[i].getNum();
    }
    if (money < sum)
    {
        std::cout << "余额不足" << std::endl;
        return false;
    }
    else
    {
        // Clear cart in database
        const char *deleteSQL = "DELETE FROM shopping_cart WHERE username = ?;";
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE)
        {
            std::cerr << "Delete failed: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        // Clear local cart
        shoppingCart.clear();
        money -= sum;
        std::cout << "购买成功" << std::endl;
        return true;
    }
}
// void Customer::purchase(sqlite3 *db)
// {
//     if (shoppingCart.empty())
//     {
//         std::cout << "购物车为空！" << std::endl;
//         return;
//     }

//     std::string address;
//     std::cout << "请输入收货地址: ";
//     std::cin.ignore();
//     std::getline(std::cin, address);

//     Order newOrder(shoppingCart, address);
//     if (newOrder.saveToDB(db, getUserName()))
//     {
//         // 清空购物车
//         const char *sql = "DELETE FROM shopping_cart WHERE username = ?;";
//         sqlite3_stmt *stmt;
//         sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
//         sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
//         sqlite3_step(stmt);
//         sqlite3_finalize(stmt);

//         shoppingCart.clear();
//         std::cout << "订单创建成功！" << std::endl;
//     }
// }
void Customer::purchase(sqlite3 *db)
{
    if (shoppingCart.empty())
    {
        std::cout << "购物车为空！" << std::endl;
        return;
    }

    std::string address;
    std::cout << "请输入收货地址: ";
    std::cin.ignore();
    std::getline(std::cin, address);

    Order newOrder(shoppingCart, address);
    newOrder.calculateTotalWithPromotions(db); // 计算促销价格

    if (newOrder.saveToDB(db, getUserName()))
    {
        // 清空购物车
        const char *sql = "DELETE FROM shopping_cart WHERE username = ?;";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        shoppingCart.clear();
        std::cout << "订单创建成功！" << std::endl;
    }
}

// void Customer::showOrders(sqlite3 *db)
// {
//     orders = Order::loadFromDB(db, getUserName());
//     for (auto &order : orders)
//     {
//         order.showOrderDetails();
//     }
// }

// bool Customer::cancelOrder(sqlite3 *db, int orderId)
// {
//     auto it = std::find_if(orders.begin(), orders.end(),
//                            [orderId](const Order &o)
//                            { return o.getOrderId() == orderId; });

//     if (it != orders.end() && it->cancelOrder(db, getUserName()))
//     {
//         orders.erase(it);
//         return true;
//     }
//     return false;
// }

Goods Viewer::getGoodsByIdStatic(sqlite3 *db, int goodsId)
{
    std::cerr << "Getting goods by ID: " << goodsId << std::endl;
    if (!db)
    {
        std::cerr << "Database not initialized!" << std::endl;
        return Goods(0, "", "", 0, "", 0);
    }

    sqlite3_stmt *stmt;
    const char *querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE id = ?;";
    int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return Goods(0, "", "", 0, "", 0);
    }

    sqlite3_bind_int(stmt, 1, goodsId);

    Goods goods(0, "", "", 0, "", 0);
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char *type = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        int price = sqlite3_column_int(stmt, 3);
        const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        int stock = sqlite3_column_int(stmt, 5);

        goods = Goods(id, std::string(type), std::string(name), price, std::string(text), stock);
    }
    else
    {
        std::cerr << "Product with ID " << goodsId << " not found!" << std::endl;
    }

    sqlite3_finalize(stmt);
    return goods;
}

void Customer::showOrders(sqlite3 *db)
{
    orders = Order::loadFromDB(db, getUserName());
    if (orders.empty())
    {
        std::cout << "暂无订单！" << std::endl;
        return;
    }
    for (auto &order : orders)
    {
        order.showOrderDetails();
    }
}

bool Customer::modifyOrderAddress(sqlite3 *db, int orderId, const std::string &newAddr)
{
    auto it = std::find_if(orders.begin(), orders.end(),
                           [orderId](const Order &o)
                           { return o.getOrderId() == orderId; });

    if (it != orders.end() && it->modifyAddress(db, newAddr))
    {
        std::cout << "地址修改成功！" << std::endl;
        return true;
    }
    std::cout << "地址修改失败！" << std::endl;
    return false;
}

bool Customer::cancelOrder(sqlite3 *db, int orderId)
{
    auto it = std::find_if(orders.begin(), orders.end(),
                           [orderId](const Order &o)
                           { return o.getOrderId() == orderId; });

    if (it != orders.end() && it->cancelOrder(db, getUserName()))
    {
        orders.erase(it);
        std::cout << "订单取消成功！" << std::endl;
        return true;
    }
    std::cout << "订单取消失败！" << std::endl;
    return false;
}

bool Customer::deleteReceivedOrder(sqlite3 *db, int orderId)
{
    // 查找订单逻辑保持不变
    auto it = std::find_if(orders.begin(), orders.end(),
                           [orderId](const Order &o)
                           {
                               return o.getOrderId() == orderId && o.getStatus() == OrderStatus::RECEIVED;
                           });

    if (it == orders.end())
    {
        std::cout << "未找到符合条件的订单！" << std::endl;
        return false; // 如果没有找到，直接返回 false
    }

    const char *sql = "DELETE FROM orders WHERE orderId = ? AND username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL 准备失败: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, orderId);
    sqlite3_bind_text(stmt, 2, getUserName().c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE)
    {
        orders.erase(it); // 只有在数据库删除成功后才删除内存中的订单
        std::cout << "订单删除成功！" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "删除失败，错误码: " << rc << std::endl; // 输出更详细的错误信息
        std::cout << "删除失败！" << std::endl;
        return false;
    }
}

void Customer::generatePurchaseStatistics(sqlite3* db) {
    if (!db) {
        std::cerr << "数据库未初始化！" << std::endl;
        return;
    }

    // 查询顾客的所有订单
    const char* sql = R"(
        SELECT o.goodsIds, o.quantities 
        FROM orders o 
        WHERE o.username = ?;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL准备失败: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);

    // 使用map来存储统计结果
    std::map<std::string, std::map<std::string, std::pair<int, int>>> stats; 
    // 结构: 类别 -> (商品名 -> (总购买数量, 购买次数))

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* goodsIdsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* quantitiesStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        std::vector<int> goodsIds = Order::parseStringToList(goodsIdsStr);
        std::vector<int> quantities = Order::parseStringToList(quantitiesStr);

        for (size_t i = 0; i < goodsIds.size(); ++i) {
            Goods goods = getGoodsByIdStatic(db, goodsIds[i]);
            std::string type = goods.getGoodsType();
            std::string name = goods.getGoodsName();
            int quantity = quantities[i];

            // 更新统计信息
            if (stats[type].find(name) == stats[type].end()) {
                stats[type][name] = std::make_pair(quantity, 1);
            } else {
                stats[type][name].first += quantity;
                stats[type][name].second += 1;
            }
        }
    }
    sqlite3_finalize(stmt);

    // 输出统计结果
    std::cout << "\n=== 购买统计分析 ===" << std::endl;
    std::cout << "用户名: " << getUserName() << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    for (const auto& typePair : stats) {
        std::cout << "类别: " << typePair.first << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "商品名称\t购买数量\t购买次数\t平均每次购买量" << std::endl;

        int categoryTotal = 0;
        for (const auto& goodsPair : typePair.second) {
            const std::string& name = goodsPair.first;
            int totalQuantity = goodsPair.second.first;
            int purchaseCount = goodsPair.second.second;
            double avgPerPurchase = static_cast<double>(totalQuantity) / purchaseCount;

            std::cout << name << "\t\t" 
                      << totalQuantity << "\t\t" 
                      << purchaseCount << "\t\t"
                      << std::fixed << std::setprecision(2) << avgPerPurchase 
                      << std::endl;

            categoryTotal += totalQuantity;
        }

        std::cout << "----------------------------------------" << std::endl;
        std::cout << "类别总计: " << categoryTotal << " 件" << std::endl;
        std::cout << "----------------------------------------" << std::endl << std::endl;
    }
}