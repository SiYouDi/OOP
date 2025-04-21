#include "ShoppingSystem.h"
#include <iostream>

void ShoppingSystem::adminLog() {
    std::string uname, pwd;
    std::cout << "Admin Login" << std::endl
        << "Username: ";
    std::cin >> uname;
    std::cout << "Password: ";
    std::cin >> pwd;
    if (uname == admin.getUserName() && admin.checkPassword(pwd)) {
        std::cout << "Admin login successful!" << std::endl;
        adminLogIn = true;
    }
    else {
        std::cout << "Admin login failed!" << std::endl;
    }
}

void ShoppingSystem::cusReg() {
    std::string uname, pwd;
    std::cout << "Customer Registration" << std::endl << "Username: ";
    std::cin >> uname;

    sqlite3_stmt* stmt;
    const char* checkSQL = "SELECT username FROM customers WHERE username = ?;";
    int rc = sqlite3_prepare_v2(manage, checkSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::cout << "Username already exists, please choose another" << std::endl;
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);

    std::cout << "Password: ";
    std::cin >> pwd;

    const char* insertSQL = "INSERT INTO customers (username, password) VALUES (?, ?);";
    rc = sqlite3_prepare_v2(manage, insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pwd.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Insert failed: " << sqlite3_errmsg(manage) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
    std::cout << "Registration successful! Welcome " << uname << std::endl;

    currentCustomer = new Customer(uname, pwd);
    currentCustomer->cartInit(manage);
}

void ShoppingSystem::cusLog()
{
    std::string uname, pwd;
    std::cout << "顾客登录" << std::endl
        << "用户名: ";
    std::cin >> uname;
    std::cout << "密码: ";
    std::cin >> pwd;

    sqlite3_stmt* stmt;
    const char* selectSQL = "SELECT username, password FROM customers WHERE username = ?;";
    int rc = sqlite3_prepare_v2(manage, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "准备 SQL 语句失败: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char* dbUsername = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* dbPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (dbUsername && dbPassword && (std::string(dbUsername) == uname) && (std::string(dbPassword) == pwd)) {
            std::cout << "登录成功！欢迎 " << uname << std::endl;
            // 创建 Customer 
            currentCustomer = new Customer(uname, pwd);
            currentCustomer->cartInit(manage);
        }
        else {
            std::cout << "用户名或密码错误！" << std::endl;
        }
    }
    else {
        std::cout << "用户名不存在！" << std::endl;
    }

    sqlite3_finalize(stmt);
}

void ShoppingSystem::cusChgPwd()
{
    std::string uname, oldPwd, newPwd;
    std::cout << "修改密码" << std::endl
        << "用户名: ";
    std::cin >> uname;
    std::cout << "旧密码: ";
    std::cin >> oldPwd;

    sqlite3_stmt* stmt;
    const char* selectSQL = "SELECT username, password FROM customers WHERE username = ? AND password = ?;";
    int rc = sqlite3_prepare_v2(manage, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "准备 SQL 语句失败: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, oldPwd.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::cout << "新密码: ";
        std::cin >> newPwd;

        const char* updateSQL = "UPDATE customers SET password = ? WHERE username = ?;";
        sqlite3_stmt* updateStmt;
        rc = sqlite3_prepare_v2(manage, updateSQL, -1, &updateStmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "准备 SQL 语句失败: " << sqlite3_errmsg(manage) << std::endl;
            sqlite3_finalize(stmt);
            return;
        }

        sqlite3_bind_text(updateStmt, 1, newPwd.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(updateStmt, 2, uname.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(updateStmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "更新密码失败: " << sqlite3_errmsg(manage) << std::endl;
            sqlite3_finalize(updateStmt);
        }
        else {
            std::cout << "密码修改成功！" << std::endl;
        }
        sqlite3_finalize(updateStmt);
    }
    else {
        std::cout << "用户名或旧密码错误，修改失败！" << std::endl;
    }

    sqlite3_finalize(stmt);
}

void ShoppingSystem::cusCartAdd(Goods goods)
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "请先登录！" << std::endl;
        return;
    }
    currentCustomer->cartAdd(manage,goods);
}

void ShoppingSystem::cusCartSet(Goods goods, int setNum)
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "请先登录！" << std::endl;
        return;
    }
    currentCustomer->cartSet(manage, goods, setNum);
}

void ShoppingSystem::cusCartDel(Goods goods)
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "请先登录！" << std::endl;
        return;
    }
    currentCustomer->cartDel(manage, goods);
}

void ShoppingSystem::cusCartShow()
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "请先登录！" << std::endl;
        return;
    }
    currentCustomer->cartShow();
}

// 购买商品
void ShoppingSystem::cusPur()
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "请先登录！" << std::endl;
        return;
    }

    currentCustomer->purchase(manage);
}
//充值
void ShoppingSystem::cusInvest()
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "请先登录！" << std::endl;
        return;
    }
    int num;
    std::cout << "请输入充值金额" << std::endl;
    std::cin >> num;
    currentCustomer->investMoney(num);
}
void ShoppingSystem::cusFind()
{
    std::cout << "请输入你想要查找的信息" << std::endl;
    std::string text;
    std::cin >> text;
    try
    {
        int num = stoi(text);
        currentCustomer->goodsFind(manage, num);
    }
    catch (const std::exception&)
    {
        currentCustomer->goodsFind(manage, text);
    }
}
void ShoppingSystem::cusAdd()
{
    while (true)
    {
        currentCustomer->goodsShow(manage);
        std::cout << "请输入你想要添加的商品的id" << std::endl;
        int id;
        std::cin >> id;
        Goods goods = currentCustomer->getGoodsById(manage, id);
        if (goods.getGoodsId() == 0)
        {
            std::cout << "商品不存在，请重新输入" << std::endl;
            continue;
        }
        currentCustomer->cartAdd(manage, goods);
        std::cout << "是否继续添加其他商品[y/n]" << std::endl;
        char choice;
        std::cin >> choice;
        Flag:
        if (choice == 'n')
        {
            break;
        }
        else if (choice == 'y')
        {
            //无事发生
        }
        else
        {
            std::cout << "错误输入" << std::endl;
            goto Flag;
        }
    }
}
void ShoppingSystem::viewerFind()
{
    std::cout << "请输入你想要查找的信息" << std::endl;
    std::string text;
    std::cin >> text;
    try
    {
        int num = stoi(text);
        viewer.goodsFind(manage, num);
    }
    catch (const std::exception&)
    {
        viewer.goodsFind(manage, text);
    }
}


// 显示主菜单
void ShoppingSystem::showMainMenu()
{
    std::cout << "\n=== 网上购物模拟系统 ===" << std::endl;
    std::cout << "1. 管理员登录" << std::endl;
    std::cout << "2. 顾客注册" << std::endl;
    std::cout << "3. 顾客登录" << std::endl;
    std::cout << "4. 查看商品信息" << std::endl;
    std::cout << "5. 购买商品" << std::endl;
    std::cout << "6. 商品查找" << std::endl;
    std::cout << "7. 退出系统" << std::endl;
    std::cout << "请选择操作: ";
}

// 显示顾客菜单

void ShoppingSystem::showCustomerMenu()
{
    std::cout << "\n=== 顾客菜单 ===" << std::endl;
    std::cout << "1. 修改密码" << std::endl;
    std::cout << "2. 购买商品" << std::endl;
    std::cout << "3. 查看商品列表" << std::endl;
    std::cout << "4. 充值" << std::endl;
    std::cout << "5. 商品查找" << std::endl;
    std::cout << "6. 添加商品到购物车" << std::endl;
    std::cout << "7. 购物车菜单" << std::endl;
    std::cout << "8. 返回主菜单" << std::endl;
    std::cout << "请选择操作: ";
}

//显示购物车菜单
void ShoppingSystem::showCartMenu()
{
    std::cout << "\n=== 购物车菜单 ===" << std::endl;
    std::cout << "1.查看当前购物车" << std::endl;
    std::cout << "2.设置商品数量" << std::endl;
    std::cout << "3.删除购物车商品" << std::endl;
    std::cout << "4.退出界面" << std::endl;
}

void ShoppingSystem::cartMenu(int choice)
{
    Goods goods = Goods(0, "", "", 0, "", 0);
    int id;
    switch (choice)
    {
    case 1:
        currentCustomer->cartShow();
        break;
    case 2:
        currentCustomer->cartShow();
        int num;
        std::cout << "请输入想要设置的商品的编号" << std::endl;
        std::cin >> id;
        std::cout << "请输入想要设置的数量" << std::endl;
        std::cin >> num;
        goods = currentCustomer->getGoodsById(manage, id);
        currentCustomer->cartSet(manage, goods, num);
        break;
    case 3:
        currentCustomer->cartShow();
        std::cout << "请输入想要删除的商品的编号" << std::endl;
        std::cin >> id;
        goods = currentCustomer->getGoodsById(manage, id);
        currentCustomer->cartDel(manage, goods);
        break;
    case 4:
        return;
    default:
        break;
    }
}

void ShoppingSystem::showAdminMenu()
{
    std::cout << "\n=== 管理员菜单 ===" << std::endl;
    std::cout << "1. 添加商品" << std::endl;
    std::cout << "2. 删除商品" << std::endl;
    std::cout << "3. 更新商品" << std::endl;
    std::cout << "4. 商品列表" << std::endl;
    std::cout << "5. 返回主菜单" << std::endl;
    std::cout << "请选择操作: ";
}

// 运行系统
void ShoppingSystem::run()
{
    //创建数据库
    int rc = SQLITE_ERROR;
    rc = sqlite3_open("/manage.db", &manage);
    if (rc == SQLITE_ERROR)
    {
        sqlite3_log(sqlite3_errcode(manage), "open field\n");
        return;
    }
    // 创建 goods 表 customers 表和shoppingCart表
    viewer.goodsCreate(manage);
    viewer.customerCreate(manage);
    viewer.shoppingCartCreate(manage);

    int choice;
    adminLogIn = false;
    currentCustomer = nullptr;
    while (true)
    {
        //管理员登录
        if (adminLogIn)
        {
            showAdminMenu();
            std::cin >> choice;
            switch (choice) {
            case 1:
                admin.goodsAdd(manage);
                break;
            case 2:
                admin.goodsDel(manage);
                break;
            case 3:
                admin.goodsUpdate(manage);
                break;
            case 4:
                admin.goodsShow(manage);
                break;
            case 5:
                adminLogIn = false;
                break;
            default:
                std::cout << "无效选择！" << std::endl;
            }
        }
        //用户登录
        else if (currentCustomer != nullptr)
        {
            // 顾客已登录，显示顾客菜单
            showCustomerMenu();
            std::cin >> choice;

            switch (choice)
            {
            case 1:
                cusChgPwd();
                break;
            case 2:
                cusPur();
                break;
            case 3:
                currentCustomer->goodsShow(manage);
                break;
            case 4:
                cusInvest();
                break;
            case 5:
                cusFind();
                break;
            case 6:
                cusAdd();
                break;
            case 7:
                while (true)
                { 
                    showCartMenu();
                    int choice;
                    std::cin >> choice;
                    if (choice == 4)
                        break;
                    cartMenu(choice);   
                }
                break;
            case 8:
                currentCustomer = nullptr;
                std::cout << "已退出顾客账号" << std::endl;
                break;
            default:
                std::cout << "无效的选择！" << std::endl;
            }
        }
        //游客登录
        else
        {
            // 显示主菜单
            showMainMenu();
            std::cin >> choice;

            switch (choice)
            {
            case 1:
                adminLog();
                break;
            case 2:
                cusReg();
                break;
            case 3:
                cusLog();
                break;
            case 4:
                viewer.goodsShow(manage);
                break;
            case 5:
                cusPur();
                break;
            case 6:
                viewerFind();
                break;
            case 7:
                std::cout << "感谢使用，再见！" << std::endl;
                sqlite3_close(manage);
                return;
            default:
                std::cout << "无效的选择！" << std::endl;
            }
        }
    }
};