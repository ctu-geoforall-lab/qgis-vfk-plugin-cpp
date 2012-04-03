#ifndef VFKTEXTBROWSER_H
#define VFKTEXTBROWSER_H

#include <QTextBrowser>
#include <QUrl>

#include "documentbuilder.h"

typedef QMap<QString, QString > TaskMap;


class VfkTextBrowser : public QTextBrowser
{
//    Q_OBJECT

public:
  enum ExportFormat { Html, Latex };

  explicit VfkTextBrowser( QWidget *parent = 0 );
  ~VfkTextBrowser();
  void startPage();
  QUrl currentUrl() { return mCurrentUrl; }
  bool exportDocument( const QUrl task, const QString fileName, ExportFormat format );
  void setConnectionName( const QString &connectionName );

signals:
  void updateHistory( QString );

public slots:
  void processAction( const QUrl task );

  void onLinkClicked( const QUrl task );
  void goBack();
  void goForth();
  void saveHistory( QString html );

private:
  DocumentBuilder *mDocumentBuilder;
  QStringList mUrlHistory;
  QStringList::iterator mHistoryIt;
  QUrl mCurrentUrl;

  static VfkDocument *documentFactory( ExportFormat format );

  TaskMap parseTask(const QUrl task);
  QString documentContent( const TaskMap taskMap, ExportFormat format );
};

#endif // VFKTEXTBROWSER_H
