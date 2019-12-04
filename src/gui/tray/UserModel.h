#ifndef USERMODEL_H
#define USERMODEL_H

#include <QAbstractListModel>
#include <QStringList>

#include "accountmanager.h"

namespace OCC {

class User
{
public:
    User(const AccountStatePtr &account);

    bool isConnected() const;
    void login();
    void logout();
    QString name() const;
    QString server() const;
    QString avatar() const;
    QString id() const;

private:
    AccountStatePtr _account;
};

class UserModel : public QAbstractListModel
{
    Q_OBJECT

public:
    static UserModel *instance();
    virtual ~UserModel() {};

    void addUser(const User &user);
    void addCurrentUser(const User &user);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE int numUsers();
    Q_INVOKABLE bool isCurrentUserConnected();
    Q_INVOKABLE QString currentUserAvatar();
    Q_INVOKABLE QString currentUserName();
    Q_INVOKABLE QString currentUserServer();
    Q_INVOKABLE void switchUser(const int id);

    enum UserRoles {
        NameRole = Qt::UserRole + 1,
        ServerRole,
        AvatarRole
    };

signals:
    Q_INVOKABLE void login();
    Q_INVOKABLE void logout();
    Q_INVOKABLE void addAccount();
    Q_INVOKABLE void removeAccount();

    Q_INVOKABLE void refreshCurrentUserGui();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    static UserModel *_instance;
    UserModel(QObject *parent = 0);
    QList<User> _users;
    User *_currentUser;

    void refreshUserList();
};

}
#endif // USERMODEL_H