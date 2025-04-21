#pragma once
#ifndef SHOPPINGSYSTEM_H
#define SHOPPINGSYSTEM_H

#include "User.h"
#include "Goods.h"
#include "../sqlite3/sqlite3.h"

class ShoppingSystem
{
private:
    Admin admin;
    Customer *currentCustomer;
    Viewer viewer;
    bool adminLogIn ;
    sqlite3 *manage;

public:
    void adminLog();
    void cusReg();
    void cusLog();
    void cusChgPwd();
    void cusCartAdd(Goods goods);
    void cusCartSet(Goods goods, int setNum);
    void cusCartDel(Goods goods);
    void cusCartShow();
    void cusPur();
    void cusInvest();
    void cusFind();
    void cusAdd();
    void viewerFind();

    void showMainMenu();
    void showCustomerMenu();
    void showCartMenu();
    void showAdminMenu();
    void cartMenu(int choice);

    void run();
};

#endif // SHOPPINGSYSTEM_H