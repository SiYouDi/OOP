#include "ShoppingSystem.h"
#include <iostream>
#include <algorithm>

void ShoppingSystem::adminLog()
{
    std::string uname, pwd;
    std::cout << "Admin Login" << std::endl
              << "Username: ";
    std::cin >> uname;
    std::cout << "Password: ";
    std::cin >> pwd;
    if (uname == admin.getUserName() && admin.checkPassword(pwd))
    {
        std::cout << "Admin login successful!" << std::endl;
        adminLogIn = true;
    }
    else
    {
        std::cout << "Admin login failed!" << std::endl;
    }
}

void ShoppingSystem::cusReg()
{
    std::string uname, pwd;
    std::cout << "Customer Registration" << std::endl
              << "Username: ";
    std::cin >> uname;

    sqlite3_stmt *stmt;
    const char *checkSQL = "SELECT username FROM customers WHERE username = ?;";
    int rc = sqlite3_prepare_v2(manage, checkSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        std::cout << "Username already exists, please choose another" << std::endl;
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);

    std::cout << "Password: ";
    std::cin >> pwd;

    const char *insertSQL = "INSERT INTO customers (username, password) VALUES (?, ?);";
    rc = sqlite3_prepare_v2(manage, insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pwd.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
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

    sqlite3_stmt *stmt;
    const char *selectSQL = "SELECT username, password FROM customers WHERE username = ?;";
    int rc = sqlite3_prepare_v2(manage, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "准备 SQL 语句失败: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        const char *dbUsername = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        const char *dbPassword = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));

        if (dbUsername && dbPassword && (std::string(dbUsername) == uname) && (std::string(dbPassword) == pwd))
        {
            std::cout << "登录成功！欢迎 " << uname << std::endl;
            // 创建 Customer
            currentCustomer = new Customer(uname, pwd);
            currentCustomer->cartInit(manage);
        }
        else
        {
            std::cout << "用户名或密码错误！" << std::endl;
        }
    }
    else
    {
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

    sqlite3_stmt *stmt;
    const char *selectSQL = "SELECT username, password FROM customers WHERE username = ? AND password = ?;";
    int rc = sqlite3_prepare_v2(manage, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "准备 SQL 语句失败: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, oldPwd.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        std::cout << "新密码: ";
        std::cin >> newPwd;

        const char *updateSQL = "UPDATE customers SET password = ? WHERE username = ?;";
        sqlite3_stmt *updateStmt;
        rc = sqlite3_prepare_v2(manage, updateSQL, -1, &updateStmt, nullptr);
        if (rc != SQLITE_OK)
        {
            std::cerr << "准备 SQL 语句失败: " << sqlite3_errmsg(manage) << std::endl;
            sqlite3_finalize(stmt);
            return;
        }

        sqlite3_bind_text(updateStmt, 1, newPwd.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(updateStmt, 2, uname.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(updateStmt);
        if (rc != SQLITE_DONE)
        {
            std::cerr << "更新密码失败: " << sqlite3_errmsg(manage) << std::endl;
            sqlite3_finalize(updateStmt);
        }
        else
        {
            std::cout << "密码修改成功！" << std::endl;
        }
        sqlite3_finalize(updateStmt);
    }
    else
    {
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
    currentCustomer->cartAdd(manage, goods);
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
// 充值
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
    catch (const std::exception &)
    {
        currentCustomer->goodsFind(manage, text);
    }
}
void ShoppingSystem::cusAdd()
{
    if (currentCustomer == nullptr)
    {
        std::cout << "请先登录！" << std::endl;
        return;
    }

    // 清空输入缓冲区
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // 检查数据库连接
    if (manage == nullptr || sqlite3_errcode(manage) != SQLITE_OK)
    {
        std::cerr << "Error: Database connection is invalid!" << std::endl;
        return;
    }

    while (true)
    {
        try
        {
            currentCustomer->goodsShow(manage);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception caught: " << e.what() << std::endl;
            return;
        }

        std::cout << "请输入你想要添加的商品的id" << std::endl;
        int id;
        std::cin >> id;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，请重新输入" << std::endl;
            continue;
        }

        Goods goods = currentCustomer->getGoodsByIdStatic(manage, id);
        if (goods.getGoodsId() == 0)
        {
            std::cout << "商品不存在，请重新输入" << std::endl;
            continue;
        }

        currentCustomer->cartAdd(manage, goods);

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
    catch (const std::exception &)
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
    std::cout << "0. 退出系统" << std::endl;
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
    std::cout << "8. 查看我的订单" << std::endl;
    std::cout << "9. 修改订单地址" << std::endl;
    std::cout << "10. 取消订单" << std::endl;
    std::cout << "11. 删除已签收订单" << std::endl;
    std::cout << "12. 查看购买统计分析" << std::endl;
    std::cout << "0. 返回主菜单" << std::endl;
    std::cout << "请选择操作: ";
}

// 显示购物车菜单
void ShoppingSystem::showCartMenu()
{
    std::cout << "\n=== 购物车菜单 ===" << std::endl;
    std::cout << "1.查看当前购物车" << std::endl;
    std::cout << "2.设置商品数量" << std::endl;
    std::cout << "3.删除购物车商品" << std::endl;
    std::cout << "4.清空购物车" << std::endl;
    std::cout << "0.退出界面" << std::endl;
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
        goods = currentCustomer->getGoodsByIdStatic(manage, id);
        currentCustomer->cartSet(manage, goods, num);
        break;
    case 3:
        currentCustomer->cartShow();
        std::cout << "请输入想要删除的商品的编号" << std::endl;
        std::cin >> id;
        goods = currentCustomer->getGoodsByIdStatic(manage, id);
        currentCustomer->cartDel(manage, goods);
        break;
    case 4:
        currentCustomer->cartClr(manage);
        break;
    case 0:
        std::cout << "退出购物车菜单" << std::endl; // 添加提示信息
        return;
    default:
        std::cout << "无效的选择！" << std::endl;
        break;
    }
}

void ShoppingSystem::adminOrderManage()
{
    std::cout << "=== 订单管理 ===" << std::endl;
    std::vector<Order> allOrders;

    // 加载所有订单
    allOrders = Order::loadFromDB(manage, "");

    if (allOrders.empty())
    {
        std::cout << "当前没有订单！" << std::endl;
        return;
    }

    // 显示订单列表
    for (size_t i = 0; i < allOrders.size(); ++i)
    {
        std::cout << "订单ID: " << allOrders[i].getOrderId()
                  << " 状态: " << allOrders[i].getStatusString()
                  << " 总金额: " << allOrders[i].getTotalAmount()
                  << std::endl;
    }

    int choice;
    while (true)
    {
        std::cout << "\n请选择操作：" << std::endl;
        std::cout << "1. 查看订单详情" << std::endl;
        std::cout << "2. 更新订单状态" << std::endl;
        std::cout << "3. 删除订单" << std::endl;
        std::cout << "0. 返回上一级菜单" << std::endl;
        std::cout << "请选择：";
        std::cin >> choice;

        if (std::cin.fail())
        { // 输入验证
            std::cin.clear();
            std::cin.ignore(1024, '\n');
            std::cout << "输入无效，请重新选择！" << std::endl;
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            int orderId;
            std::cout << "请输入订单ID：";
            if (!(std::cin >> orderId))
            {
                std::cin.clear();
                std::cin.ignore(1024, '\n');
                std::cout << "无效的订单ID！" << std::endl;
                break;
            }
            auto it = std::find_if(allOrders.begin(), allOrders.end(),
                                   [orderId](const Order &o)
                                   { return o.getOrderId() == orderId; });
            if (it != allOrders.end())
            {
                it->showOrderDetails();
            }
            else
            {
                std::cout << "未找到订单！" << std::endl;
            }
            break;
        }
        case 2:
        {
            int orderId;
            std::cout << "请输入订单ID：";
            if (!(std::cin >> orderId))
            {
                std::cin.clear();
                std::cin.ignore(1024, '\n');
                std::cout << "无效的订单ID！" << std::endl;
                break;
            }
            auto it = std::find_if(allOrders.begin(), allOrders.end(),
                                   [orderId](const Order &o)
                                   { return o.getOrderId() == orderId; });
            if (it != allOrders.end())
            {
                std::cout << "当前状态：" << it->getStatusString() << std::endl;
                std::cout << "请选择新的状态：" << std::endl;
                std::cout << "1. 待发货" << std::endl;
                std::cout << "2. 已发货" << std::endl;
                std::cout << "3. 已签收" << std::endl;
                int statusChoice;
                if (!(std::cin >> statusChoice) || statusChoice < 1 || statusChoice > 3)
                {
                    std::cin.clear();
                    std::cin.ignore(1024, '\n');
                    std::cout << "无效的选择！" << std::endl;
                    break;
                }

                OrderStatus newStatus;
                switch (statusChoice)
                {
                case 1:
                    newStatus = OrderStatus::PENDING;
                    break;
                case 2:
                    newStatus = OrderStatus::SHIPPED;
                    break;
                case 3:
                    newStatus = OrderStatus::RECEIVED;
                    break;
                }

                // 状态合法性检查
                if (it->getStatus() == OrderStatus::RECEIVED && newStatus != OrderStatus::RECEIVED)
                {
                    std::cout << "已签收的订单不可修改状态！" << std::endl;
                    break;
                }

                const char *sql = "UPDATE orders SET status = ? WHERE orderId = ?;";
                sqlite3_stmt *stmt;
                int rc = sqlite3_prepare_v2(manage, sql, -1, &stmt, nullptr);
                if (rc != SQLITE_OK)
                {
                    std::cerr << "SQL准备失败: " << sqlite3_errmsg(manage) << std::endl;
                    break;
                }
                sqlite3_bind_int(stmt, 1, static_cast<int>(newStatus));
                sqlite3_bind_int(stmt, 2, orderId);
                rc = sqlite3_step(stmt);
                sqlite3_finalize(stmt);
                if (rc != SQLITE_DONE)
                {
                    std::cerr << "更新失败: " << sqlite3_errmsg(manage) << std::endl;
                }
                else
                {
                    std::cout << "订单状态更新成功！" << std::endl;
                    const_cast<Order *>(&(*it))->setStatus(newStatus);
                }
            }
            else
            {
                std::cout << "未找到订单！" << std::endl;
            }
            break;
        }
        case 3:
        {
            int orderId;
            std::cout << "请输入订单ID：";
            if (!(std::cin >> orderId))
            {
                std::cin.clear();
                std::cin.ignore(1024, '\n');
                std::cout << "无效的订单ID！" << std::endl;
                break;
            }
            auto it = std::find_if(allOrders.begin(), allOrders.end(),
                                   [orderId](const Order &o)
                                   { return o.getOrderId() == orderId; });
            if (it != allOrders.end())
            {
                // 管理员强制取消订单，回滚库存
                if (it->cancelOrder(manage, "admin"))
                { // 假设允许管理员绕过状态检查
                    const char *sql = "DELETE FROM orders WHERE orderId = ?;";
                    sqlite3_stmt *stmt;
                    int rc = sqlite3_prepare_v2(manage, sql, -1, &stmt, nullptr);
                    if (rc != SQLITE_OK)
                    {
                        std::cerr << "SQL准备失败: " << sqlite3_errmsg(manage) << std::endl;
                        break;
                    }
                    sqlite3_bind_int(stmt, 1, orderId);
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                    if (rc != SQLITE_DONE)
                    {
                        std::cerr << "删除失败: " << sqlite3_errmsg(manage) << std::endl;
                    }
                    else
                    {
                        std::cout << "订单删除成功！" << std::endl;
                        allOrders.erase(it);
                    }
                }
                else
                {
                    std::cout << "订单取消失败，无法删除！" << std::endl;
                }
            }
            else
            {
                std::cout << "未找到订单！" << std::endl;
            }
            break;
        }
        case 0:
            return;
        default:
            std::cout << "无效的选择！" << std::endl;
        }
    }
}

void ShoppingSystem::showAdminMenu()
{
    std::cout << "\n=== 管理员菜单 ===" << std::endl;
    std::cout << "1. 添加商品" << std::endl;
    std::cout << "2. 删除商品" << std::endl;
    std::cout << "3. 更新商品" << std::endl;
    std::cout << "4. 商品列表" << std::endl;
    std::cout << "5. 订单管理" << std::endl;
    std::cout << "6. 促销管理" << std::endl;
    std::cout << "0. 返回主菜单" << std::endl;
    std::cout << "请选择操作: ";
}

void ShoppingSystem::adminPromotionMenu()
{
    int choice;
    while (true)
    {
        std::cout << "\n=== 促销管理 ===" << std::endl;
        std::cout << "1. 添加促销活动" << std::endl;
        std::cout << "2. 删除促销活动" << std::endl;
        std::cout << "3. 关联商品与促销" << std::endl;
        std::cout << "4. 查看所有促销" << std::endl;
        std::cout << "0. 返回上级菜单" << std::endl;
        std::cout << "请选择操作: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            admin.addPromotion(manage);
            break;
        case 2:
            admin.deletePromotion(manage);
            break;
        case 3:
            admin.assignPromotionToGoods(manage);
            break;
        case 4:
            admin.showPromotions(manage);
            break;
        case 0:
            return;
        default:
            std::cout << "无效选择!" << std::endl;
        }
    }
}

// 运行系统
void ShoppingSystem::run()
{
    // 创建数据库
    int rc = SQLITE_ERROR;
    rc = sqlite3_open("./manage.db", &manage);
    if (rc == SQLITE_ERROR)
    {
        sqlite3_log(sqlite3_errcode(manage), "open field\n");
        return;
    }
    // 创建 goods 表 customers 表和shoppingCart表
    viewer.goodsCreate(manage);
    viewer.customerCreate(manage);
    viewer.shoppingCartCreate(manage);
    viewer.orderCreate(manage);
    viewer.promotionCreate(manage);

    int choice;
    adminLogIn = false;
    currentCustomer = nullptr;
    while (true)
    {
        // 管理员登录
        if (adminLogIn)
        {
            showAdminMenu();
            std::cin >> choice;
            switch (choice)
            {
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
                adminOrderManage();
            case 6: // 新增促销管理菜单
                adminPromotionMenu();
                break;
            case 0:
                adminLogIn = false;
                break;
            default:
                std::cout << "无效选择！" << std::endl;
            }
        }
        // 用户登录
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
                    if (choice == 0)
                    {
                        std::cout << "退出购物车菜单" << std::endl;
                        break;
                    }
                    cartMenu(choice);
                }
                break;
            case 8:
                currentCustomer->showOrders(manage);
                break;
            case 9:
            {
                int orderId;
                std::string newAddr;
                std::cout << "输入订单号：";
                std::cin >> orderId;
                std::cout << "输入新地址：";
                std::cin.ignore();
                std::getline(std::cin, newAddr);
                currentCustomer->modifyOrderAddress(manage, orderId, newAddr);
                break;
            }
            case 10:
            {
                int orderId;
                std::cout << "输入要取消的订单号：";
                std::cin >> orderId;
                currentCustomer->cancelOrder(manage, orderId);
                break;
            }
            case 11:
            {
                int orderId;
                std::cout << "输入要删除的订单号：";
                std::cin >> orderId;
                currentCustomer->deleteReceivedOrder(manage, orderId);
                break;
            }
            case 12:
                currentCustomer->generatePurchaseStatistics(manage);
                break;
            case 0:
                currentCustomer = nullptr;
                std::cout << "已退出顾客账号" << std::endl;
                break;
            default:
                std::cout << "无效的选择！" << std::endl;
            }
        }
        // 游客登录
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
            case 0:
                std::cout << "感谢使用，再见！" << std::endl;
                sqlite3_close(manage);
                return;
            default:
                std::cout << "无效的选择！" << std::endl;
            }
        }
    }
};