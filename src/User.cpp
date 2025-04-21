#include "User.h"
#include <iostream>
#include <string>
#include <algorithm>

// Viewer implementation
bool Viewer::goodsCheck(sqlite3* db) {
    if (!db) {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char* checkTableSQL = "SELECT name FROM sqlite_master WHERE type='table' AND name='goods';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, checkTableSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        std::cerr << "Table 'goods' doesn't exist or is empty!" << std::endl;
        return false;
    }
    sqlite3_finalize(stmt);

    const char* checkContentSQL = "SELECT COUNT(*) FROM goods;";
    rc = sqlite3_prepare_v2(db, checkContentSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        std::cerr << "Table 'goods' content is empty!" << std::endl;
        return false;
    }

    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (count == 0) {
        std::cerr << "Table 'goods' content is empty!" << std::endl;
        return false;
    }
    return true;
}

bool Viewer::goodsCreate(sqlite3* db) {
    if (!db) {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS goods ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type TEXT NOT NULL,"
        "name TEXT NOT NULL,"
        "price INTEGER NOT NULL,"
        "text TEXT NOT NULL,"
        "stock INTEGER NOT NULL);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Create table failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Viewer::customerCreate(sqlite3* db) {
    if (!db) {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS customers ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Create table failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Viewer::shoppingCartCreate(sqlite3* db) {
    if (!db) {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS shopping_cart ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "goods_id INTEGER NOT NULL,"
        "quantity INTEGER NOT NULL,"
        "FOREIGN KEY(goods_id) REFERENCES goods(id),"
        "FOREIGN KEY(username) REFERENCES customers(username));";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Create shopping_cart table failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Viewer::goodsShow(sqlite3* db) {
    if (goodsCreate(db)) {
        std::cout << "Table exists" << std::endl;
        const char* querySQL = "SELECT id, type, name, price, text, stock FROM goods;";
        sqlite3_stmt* stmt;

        int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        std::cout << "Product list:" << std::endl;
        std::cout << "ID\tType\tName\tPrice\tDescription\tStock" << std::endl;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            int price = sqlite3_column_int(stmt, 3);
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            int stock = sqlite3_column_int(stmt, 5);

            std::cout << id << "\t" << type << "\t" << name << "\t" << price << "\t" << text << "\t" << stock << std::endl;
        }

        if (rc != SQLITE_DONE) {
            std::cerr << "Query failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        return true;
    }
    return false;
}

bool Viewer::goodsFind(sqlite3* db, std::string text) {
    if (!db) {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    if (!goodsCheck(db)) {
        return false;
    }

    text.erase(0, text.find_first_not_of(" \t\n\r"));
    text.erase(text.find_last_not_of(" \t\n\r") + 1);

    if (text.empty()) {
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

    switch (choice) {
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

    const char* querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE "
        "name LIKE ? OR text LIKE ? OR type LIKE ? "
        "ORDER BY name;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
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
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        found = true;
        int id = sqlite3_column_int(stmt, 0);
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int price = sqlite3_column_int(stmt, 3);
        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        int stock = sqlite3_column_int(stmt, 5);

        std::cout << id << "\t" << type << "\t" << name << "\t" << price << "\t" << stock << std::endl;
        std::cout << "Description: " << desc << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Query failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    if (!found) {
        std::cout << "No matching products found." << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }

    return true;
}

bool Viewer::goodsFind(sqlite3* db, int num) {
    if (!goodsCheck(db)) {
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
    switch (choice) {
    case 1: querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE price = ?"; break;
    case 2: querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE price <= ?"; break;
    case 3: querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE price >= ?"; break;
    default:
        std::cout << "Invalid choice" << std::endl;
        return false;
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, querySQL.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, num);

    std::cout << "Search results (price: " << num << "):" << std::endl;
    std::cout << "ID\tType\tName\tPrice\tDescription\tStock" << std::endl;

    bool found = false;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        found = true;
        int id = sqlite3_column_int(stmt, 0);
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int price = sqlite3_column_int(stmt, 3);
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        int stock = sqlite3_column_int(stmt, 5);

        std::cout << id << "\t" << type << "\t" << name << "\t" << price << "\t" << text << "\t" << stock << std::endl;
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Query failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    if (!found) {
        std::cout << "No products found with price " << num << std::endl;
    }

    return true;
}

Goods Viewer::getGoodsById(sqlite3* db, int goodsId) {
    if (!db) {
        std::cerr << "Database not initialized!" << std::endl;
        return Goods(0, "", "", 0, "", 0);
    }

    sqlite3_stmt* stmt;
    const char* querySQL = "SELECT id, type, name, price, text, stock FROM goods WHERE id = ?;";
    int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return Goods(0, "", "", 0, "", 0);
    }

    sqlite3_bind_int(stmt, 1, goodsId);

    Goods goods(0, "", "", 0, "", 0);
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int price = sqlite3_column_int(stmt, 3);
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        int stock = sqlite3_column_int(stmt, 5);

        goods = Goods(id, std::string(type), std::string(name), price, std::string(text), stock);
    }
    else {
        std::cerr << "Product with ID " << goodsId << " not found!" << std::endl;
    }

    sqlite3_finalize(stmt);
    return goods;
}

// User implementation
User::User(std::string tp, std::string uname, std::string pwd)
    : type(tp), account(uname), password(pwd) {}

std::string User::getUserName() const { return account; }
bool User::checkPassword(const std::string& pwd) const { return password == pwd; }
void User::changePassword(const std::string& newPwd) { password = newPwd; }

// Admin implementation
Admin::Admin() : User("admin", "account", "password") {}

bool Admin::goodsAdd(sqlite3* db) {
    if (!db) {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    std::string type, name, text;
    int price, stock;
    std::cout << "Enter product type, name, price, description, and stock:" << std::endl;
    std::cin >> type >> name >> price >> text >> stock;

    const char* insertSQL = "INSERT INTO goods (type, name, price, text, stock) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, price);
    sqlite3_bind_text(stmt, 4, text.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, stock);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    std::cout << "Product added successfully!" << std::endl;
    return true;
}

bool Admin::goodsDel(sqlite3* db) {
    if (!db) {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    int id;
    std::cout << "Enter product ID to delete:" << std::endl;
    std::cin >> id;

    const char* deleteSQL = "DELETE FROM goods WHERE id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Delete failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    std::cout << "Product deleted successfully!" << std::endl;
    return true;
}

bool Admin::goodsUpdate(sqlite3* db) {
    if (!db) {
        std::cerr << "Database not initialized!" << std::endl;
        return false;
    }

    const char* checkTableSQL = "SELECT name FROM sqlite_master WHERE type='table' AND name='goods';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, checkTableSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        std::cerr << "Table 'goods' doesn't exist or is empty!" << std::endl;
        return false;
    }
    sqlite3_finalize(stmt);

    const char* checkContentSQL = "SELECT COUNT(*) FROM goods;";
    rc = sqlite3_prepare_v2(db, checkContentSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        std::cerr << "Table 'goods' content is empty!" << std::endl;
        return false;
    }

    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (count == 0) {
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

    const char* updateSQL = "UPDATE goods SET type = ?, name = ?, price = ?, text = ?, stock = ? WHERE id = ?;";
    rc = sqlite3_prepare_v2(db, updateSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
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
    if (rc != SQLITE_DONE) {
        std::cerr << "Update failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    std::cout << "Product updated successfully!" << std::endl;
    return true;
}

// Customer implementation
Customer::Customer(std::string uname, std::string pwd)
    : User("Customer", uname, pwd), money(0) {}

bool Customer::investMoney(int num) {
    money += num;
    return true;
}

bool Customer::cartInit(sqlite3* db) {
    if (!db) {
        std::cerr << "数据库" << std::endl;
        return false;
    }

    sqlite3_stmt* stmt;
    const char* querySQL = "SELECT goods_id, quantity FROM shopping_cart WHERE username = ?;";
    int rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int goodsId = sqlite3_column_int(stmt, 0);
        int quantity = sqlite3_column_int(stmt, 1);

        Goods goods = getGoodsById(db, goodsId);
        int num = quantity;
        int totalPrice = goods.getGoodsPrice() * num;
        cartGoods cartgoods = cartGoods(goods, num, totalPrice);
        shoppingCart.push_back(cartgoods);
    }

    sqlite3_finalize(stmt);
    return true;
}

void Customer::cartAdd(sqlite3* db,Goods goods) {
    while (true) {
        std::cout << "是否添加到购物车?[y/n]" << std::endl;
        char choice;
        std::cin >> choice;
        if (choice == 'y') {
            Flag:
            try {
                bool found = false;
                for (auto& item : shoppingCart) {
                    if (item.getGoods().getGoodsId() == goods.getGoodsId()) {
                        std::cout << "商品已在购物车中，是否增加数量?[y/n]" << std::endl;
                        char choice;
                        std::cin >> choice;
                        if (choice == 'y') {
                            item.setNum(item.getNum() + 1);
                            // Update database
                            const char* updateSQL = "UPDATE shopping_cart SET quantity = ? WHERE username = ? AND goods_id = ?;";
                            sqlite3_stmt* stmt;
                            int rc = sqlite3_prepare_v2(db, updateSQL, -1, &stmt, nullptr);
                            if (rc != SQLITE_OK) {
                                std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                                return;
                            }
                            sqlite3_bind_int(stmt, 1, item.getNum());
                            sqlite3_bind_text(stmt, 2, getUserName().c_str(), -1, SQLITE_STATIC);
                            sqlite3_bind_int(stmt, 3, goods.getGoodsId());
                            rc = sqlite3_step(stmt);
                            sqlite3_finalize(stmt);
                            if (rc != SQLITE_DONE) {
                                std::cerr << "Update failed: " << sqlite3_errmsg(db) << std::endl;
                                return;
                            }
                            std::cout << "添加成功" << std::endl;
                        }
                        else {
                            std::cout << "取消成功" << std::endl;
                        }
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    int num = 1;
                    int totalPrice = num * goods.getGoodsPrice();
                    cartGoods cartgoods = cartGoods(goods, num, totalPrice);
                    shoppingCart.push_back(cartgoods);

                    // Insert into database
                    const char* insertSQL = "INSERT INTO shopping_cart (username, goods_id, quantity) VALUES (?, ?, ?);";
                    sqlite3_stmt* stmt;
                    int rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
                    if (rc != SQLITE_OK) {
                        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                        return;
                    }
                    sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt, 2, goods.getGoodsId());
                    sqlite3_bind_int(stmt, 3, num);
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                    if (rc != SQLITE_DONE) {
                        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << std::endl;
                        return;
                    }
                    std::cout << "添加成功" << std::endl;
                }
                std::cout << "是否继续添加该商品?[y/n]" << std::endl;
                char choice;
                std::cin >> choice;
                if (choice == 'n')
                    break;
                else if (choice == 'y')
                {
                    goto Flag;
                }
            }
            catch (const std::exception&) {
                std::cout << "添加失败" << std::endl;
                return;
            }
        }
        else if (choice == 'n') {
            return;
        }
        else {
            std::cout << "输入错误，请重新输入" << std::endl;
        }
    }
}

bool Customer::cartSet(sqlite3* db,Goods goods, int setNum) {
    try {
        bool found = false;
        for (auto& item : shoppingCart) {
            if (item.getGoods() == goods) {
                if (setNum <= 0) {
                    std::cout << "是否确认删除商品?[y/n]" << std::endl;
                    char choice;
                    std::cin >> choice;
                    if (choice == 'y') {
                        auto it = std::find(shoppingCart.begin(), shoppingCart.end(), item);
                        if (it != shoppingCart.end()) {
                            shoppingCart.erase(it);

                            // Delete from database
                            const char* deleteSQL = "DELETE FROM shopping_cart WHERE username = ? AND goods_id = ?;";
                            sqlite3_stmt* stmt;
                            int rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
                            if (rc != SQLITE_OK) {
                                std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                                return false;
                            }
                            sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
                            sqlite3_bind_int(stmt, 2, goods.getGoodsId());
                            rc = sqlite3_step(stmt);
                            sqlite3_finalize(stmt);
                            if (rc != SQLITE_DONE) {
                                std::cerr << "Delete failed: " << sqlite3_errmsg(db) << std::endl;
                                return false;
                            }
                        }
                    }
                }
                else {
                    item.setNum(setNum);

                    // Update database
                    const char* updateSQL = "UPDATE shopping_cart SET quantity = ? WHERE username = ? AND goods_id = ?;";
                    sqlite3_stmt* stmt;
                    int rc = sqlite3_prepare_v2(db, updateSQL, -1, &stmt, nullptr);
                    if (rc != SQLITE_OK) {
                        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                        return false;
                    }
                    sqlite3_bind_int(stmt, 1, setNum);
                    sqlite3_bind_text(stmt, 2, getUserName().c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt, 3, goods.getGoodsId());
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                    if (rc != SQLITE_DONE) {
                        std::cerr << "Update failed: " << sqlite3_errmsg(db) << std::endl;
                        return false;
                    }
                }
                found = true;
                break;
            }
        }

        if (found) {
            std::cout << "修改成功" << std::endl;
        }
        else {
            std::cout << "修改失败，商品不存在" << std::endl;
        }
        return found;
    }
    catch (const std::exception&) {
        std::cout << "set失败" << std::endl;
        return false;
    }
}

bool Customer::cartDel(sqlite3* db, Goods goods) {
    try {
        bool found = false;
        for (auto& item : shoppingCart) {
            if (item.getGoods() == goods) {
                auto it = std::find(shoppingCart.begin(), shoppingCart.end(), item);
                if (it != shoppingCart.end()) {
                    shoppingCart.erase(it);

                    // Delete from database
                    const char* deleteSQL = "DELETE FROM shopping_cart WHERE username = ? AND goods_id = ?;";
                    sqlite3_stmt* stmt;
                    int rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
                    if (rc != SQLITE_OK) {
                        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
                        return false;
                    }
                    sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_int(stmt, 2, goods.getGoodsId());
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                    if (rc != SQLITE_DONE) {
                        std::cerr << "Delete failed: " << sqlite3_errmsg(db) << std::endl;
                        return false;
                    }
                }
                found = true;
                break;
            }
        }

        if (found) {
            std::cout << "删除成功" << std::endl;
        }
        else {
            std::cout << "修改失败，删除失败" << std::endl;
        }
        return found;
    }
    catch (const std::exception&) {
        std::cout << "del失败" << std::endl;
        return false;
    }
}

bool Customer::cartShow() {
    try {
        if (shoppingCart.empty()) {
            std::cout << "购物车为空" << std::endl;
            return false;
        }

        std::cout << "\n=== Shopping Cart ===" << std::endl;
        std::cout << "Id\tName\tType\tPrice\tQuantity\tTotal" << std::endl;

        int cartTotalNum = 0;
        int cartTotalPrice = 0;
        for (auto& item : shoppingCart) {
            const Goods& goods = item.getGoods();

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
    catch (const std::exception& e) {
        std::cout << "show失败" << e.what() << std::endl;
        return false;
    }
}

bool Customer::purchase(sqlite3* db) {
    int sum = 0;
    if (shoppingCart.empty()) {
        std::cout << "购物车为空" << std::endl;
        return false;
    }

    for (size_t i = 0; i < shoppingCart.size(); i++) {
        sum += shoppingCart[i].getGoods().getGoodsPrice() * shoppingCart[i].getNum();
    }
    if (money < sum) {
        std::cout << "余额不足" << std::endl;
        return false;
    }
    else {
        // Clear cart in database
        const char* deleteSQL = "DELETE FROM shopping_cart WHERE username = ?;";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL prepare failed: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        sqlite3_bind_text(stmt, 1, getUserName().c_str(), -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) {
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