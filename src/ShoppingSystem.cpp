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
    std::cout << "�˿͵�¼" << std::endl
        << "�û���: ";
    std::cin >> uname;
    std::cout << "����: ";
    std::cin >> pwd;

    sqlite3_stmt* stmt;
    const char* selectSQL = "SELECT username, password FROM customers WHERE username = ?;";
    int rc = sqlite3_prepare_v2(manage, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "׼�� SQL ���ʧ��: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char* dbUsername = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* dbPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (dbUsername && dbPassword && (std::string(dbUsername) == uname) && (std::string(dbPassword) == pwd)) {
            std::cout << "��¼�ɹ�����ӭ " << uname << std::endl;
            // ���� Customer 
            currentCustomer = new Customer(uname, pwd);
            currentCustomer->cartInit(manage);
        }
        else {
            std::cout << "�û������������" << std::endl;
        }
    }
    else {
        std::cout << "�û��������ڣ�" << std::endl;
    }

    sqlite3_finalize(stmt);
}

void ShoppingSystem::cusChgPwd()
{
    std::string uname, oldPwd, newPwd;
    std::cout << "�޸�����" << std::endl
        << "�û���: ";
    std::cin >> uname;
    std::cout << "������: ";
    std::cin >> oldPwd;

    sqlite3_stmt* stmt;
    const char* selectSQL = "SELECT username, password FROM customers WHERE username = ? AND password = ?;";
    int rc = sqlite3_prepare_v2(manage, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "׼�� SQL ���ʧ��: " << sqlite3_errmsg(manage) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, oldPwd.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::cout << "������: ";
        std::cin >> newPwd;

        const char* updateSQL = "UPDATE customers SET password = ? WHERE username = ?;";
        sqlite3_stmt* updateStmt;
        rc = sqlite3_prepare_v2(manage, updateSQL, -1, &updateStmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "׼�� SQL ���ʧ��: " << sqlite3_errmsg(manage) << std::endl;
            sqlite3_finalize(stmt);
            return;
        }

        sqlite3_bind_text(updateStmt, 1, newPwd.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(updateStmt, 2, uname.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(updateStmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "��������ʧ��: " << sqlite3_errmsg(manage) << std::endl;
            sqlite3_finalize(updateStmt);
        }
        else {
            std::cout << "�����޸ĳɹ���" << std::endl;
        }
        sqlite3_finalize(updateStmt);
    }
    else {
        std::cout << "�û��������������޸�ʧ�ܣ�" << std::endl;
    }

    sqlite3_finalize(stmt);
}

void ShoppingSystem::cusCartAdd(Goods goods)
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "���ȵ�¼��" << std::endl;
        return;
    }
    currentCustomer->cartAdd(manage,goods);
}

void ShoppingSystem::cusCartSet(Goods goods, int setNum)
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "���ȵ�¼��" << std::endl;
        return;
    }
    currentCustomer->cartSet(manage, goods, setNum);
}

void ShoppingSystem::cusCartDel(Goods goods)
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "���ȵ�¼��" << std::endl;
        return;
    }
    currentCustomer->cartDel(manage, goods);
}

void ShoppingSystem::cusCartShow()
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "���ȵ�¼��" << std::endl;
        return;
    }
    currentCustomer->cartShow();
}

// ������Ʒ
void ShoppingSystem::cusPur()
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "���ȵ�¼��" << std::endl;
        return;
    }

    currentCustomer->purchase(manage);
}
//��ֵ
void ShoppingSystem::cusInvest()
{
    if (currentCustomer == nullptr)
    {
        std::cout << std::endl
            << "���ȵ�¼��" << std::endl;
        return;
    }
    int num;
    std::cout << "�������ֵ���" << std::endl;
    std::cin >> num;
    currentCustomer->investMoney(num);
}
void ShoppingSystem::cusFind()
{
    std::cout << "����������Ҫ���ҵ���Ϣ" << std::endl;
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
        std::cout << "����������Ҫ��ӵ���Ʒ��id" << std::endl;
        int id;
        std::cin >> id;
        Goods goods = currentCustomer->getGoodsById(manage, id);
        if (goods.getGoodsId() == 0)
        {
            std::cout << "��Ʒ�����ڣ�����������" << std::endl;
            continue;
        }
        currentCustomer->cartAdd(manage, goods);
        std::cout << "�Ƿ�������������Ʒ[y/n]" << std::endl;
        char choice;
        std::cin >> choice;
        Flag:
        if (choice == 'n')
        {
            break;
        }
        else if (choice == 'y')
        {
            //���·���
        }
        else
        {
            std::cout << "��������" << std::endl;
            goto Flag;
        }
    }
}
void ShoppingSystem::viewerFind()
{
    std::cout << "����������Ҫ���ҵ���Ϣ" << std::endl;
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


// ��ʾ���˵�
void ShoppingSystem::showMainMenu()
{
    std::cout << "\n=== ���Ϲ���ģ��ϵͳ ===" << std::endl;
    std::cout << "1. ����Ա��¼" << std::endl;
    std::cout << "2. �˿�ע��" << std::endl;
    std::cout << "3. �˿͵�¼" << std::endl;
    std::cout << "4. �鿴��Ʒ��Ϣ" << std::endl;
    std::cout << "5. ������Ʒ" << std::endl;
    std::cout << "6. ��Ʒ����" << std::endl;
    std::cout << "7. �˳�ϵͳ" << std::endl;
    std::cout << "��ѡ�����: ";
}

// ��ʾ�˿Ͳ˵�

void ShoppingSystem::showCustomerMenu()
{
    std::cout << "\n=== �˿Ͳ˵� ===" << std::endl;
    std::cout << "1. �޸�����" << std::endl;
    std::cout << "2. ������Ʒ" << std::endl;
    std::cout << "3. �鿴��Ʒ�б�" << std::endl;
    std::cout << "4. ��ֵ" << std::endl;
    std::cout << "5. ��Ʒ����" << std::endl;
    std::cout << "6. �����Ʒ�����ﳵ" << std::endl;
    std::cout << "7. ���ﳵ�˵�" << std::endl;
    std::cout << "8. �������˵�" << std::endl;
    std::cout << "��ѡ�����: ";
}

//��ʾ���ﳵ�˵�
void ShoppingSystem::showCartMenu()
{
    std::cout << "\n=== ���ﳵ�˵� ===" << std::endl;
    std::cout << "1.�鿴��ǰ���ﳵ" << std::endl;
    std::cout << "2.������Ʒ����" << std::endl;
    std::cout << "3.ɾ�����ﳵ��Ʒ" << std::endl;
    std::cout << "4.�˳�����" << std::endl;
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
        std::cout << "��������Ҫ���õ���Ʒ�ı��" << std::endl;
        std::cin >> id;
        std::cout << "��������Ҫ���õ�����" << std::endl;
        std::cin >> num;
        goods = currentCustomer->getGoodsById(manage, id);
        currentCustomer->cartSet(manage, goods, num);
        break;
    case 3:
        currentCustomer->cartShow();
        std::cout << "��������Ҫɾ������Ʒ�ı��" << std::endl;
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
    std::cout << "\n=== ����Ա�˵� ===" << std::endl;
    std::cout << "1. �����Ʒ" << std::endl;
    std::cout << "2. ɾ����Ʒ" << std::endl;
    std::cout << "3. ������Ʒ" << std::endl;
    std::cout << "4. ��Ʒ�б�" << std::endl;
    std::cout << "5. �������˵�" << std::endl;
    std::cout << "��ѡ�����: ";
}

// ����ϵͳ
void ShoppingSystem::run()
{
    //�������ݿ�
    int rc = SQLITE_ERROR;
    rc = sqlite3_open("/manage.db", &manage);
    if (rc == SQLITE_ERROR)
    {
        sqlite3_log(sqlite3_errcode(manage), "open field\n");
        return;
    }
    // ���� goods �� customers ���shoppingCart��
    viewer.goodsCreate(manage);
    viewer.customerCreate(manage);
    viewer.shoppingCartCreate(manage);

    int choice;
    adminLogIn = false;
    currentCustomer = nullptr;
    while (true)
    {
        //����Ա��¼
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
                std::cout << "��Чѡ��" << std::endl;
            }
        }
        //�û���¼
        else if (currentCustomer != nullptr)
        {
            // �˿��ѵ�¼����ʾ�˿Ͳ˵�
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
                std::cout << "���˳��˿��˺�" << std::endl;
                break;
            default:
                std::cout << "��Ч��ѡ��" << std::endl;
            }
        }
        //�ο͵�¼
        else
        {
            // ��ʾ���˵�
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
                std::cout << "��лʹ�ã��ټ���" << std::endl;
                sqlite3_close(manage);
                return;
            default:
                std::cout << "��Ч��ѡ��" << std::endl;
            }
        }
    }
};