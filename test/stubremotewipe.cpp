// stub to prevent linker error
#include "accountmanager.h"
#include "accountstate.h"
#include "socketapi.h"
#include "folderman.h"

OCC::AccountManager *OCC::AccountManager::instance() { return static_cast<AccountManager *>(new QObject); }
void OCC::AccountManager::save(bool) { }
OCC::AccountState *OCC::AccountManager::addAccount(const AccountPtr& ac) { return new OCC::AccountState(ac); }
void OCC::AccountManager::deleteAccount(AccountState *) { }
void OCC::AccountManager::accountRemoved(OCC::AccountState*) { }
OCC::AccountStatePtr OCC::AccountManager::account(const QString &){ return AccountStatePtr(); }
const QMetaObject OCC::AccountManager::staticMetaObject = QObject::staticMetaObject;

OCC::FolderMan *OCC::FolderMan::instance() { return static_cast<FolderMan *>(new QObject); }
void OCC::FolderMan::wipeDone(OCC::AccountState*, bool) { }
OCC::Folder* OCC::FolderMan::addFolder(OCC::AccountState* as, OCC::FolderDefinition const &f) { return nullptr; }
void OCC::FolderMan::slotWipeFolderForAccount(OCC::AccountState*) { }
QString OCC::FolderMan::unescapeAlias(QString const&){ return QString(); }
QString OCC::FolderMan::escapeAlias(QString const&){ return QString(); }
void OCC::FolderMan::scheduleFolder(OCC::Folder*){ }
OCC::SocketApi *OCC::FolderMan::socketApi(){ return new SocketApi;  }
OCC::Folder::Map OCC::FolderMan::map() { return OCC::Folder::Map(); }
void OCC::FolderMan::setSyncEnabled(bool) { }
void OCC::FolderMan::slotSyncOnceFileUnlocks(QString const&) { }
void OCC::FolderMan::slotScheduleETagJob(QString const&, OCC::RequestEtagJob*){ }
OCC::Folder *OCC::FolderMan::folderForPath(QString const&) { return nullptr; }
OCC::Folder* OCC::FolderMan::folder(QString const&) { return nullptr; }
void OCC::FolderMan::folderSyncStateChange(OCC::Folder*) { }
const QMetaObject OCC::FolderMan::staticMetaObject = QObject::staticMetaObject;
