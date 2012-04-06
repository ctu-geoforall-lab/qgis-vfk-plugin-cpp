#ifndef VFKTEXTBROWSER_H
#define VFKTEXTBROWSER_H

#include <QTextBrowser>
#include <QUrl>
#include <QPair>

#include "documentbuilder.h"

typedef QMap<QString, QString > TaskMap;

class VfkTextBrowser : public QTextBrowser
{
    Q_OBJECT


public:
  struct HistoryRecord
  {
    QString html;
    QStringList parIds;
    QStringList budIds;
    Coordinates definitionPoint;
  };

  typedef QList<HistoryRecord> History;

  enum ExportFormat { Html, RichText, Latex };

  explicit VfkTextBrowser( QWidget *parent = 0 );
  ~VfkTextBrowser();
  void startPage();
  QUrl currentUrl() { return mCurrentUrl; }
  bool exportDocument( const QUrl task, const QString fileName, ExportFormat format );
  void setConnectionName( const QString &connectionName );
  QStringList currentParIds() { return mCurrentRecord.parIds; }
  QStringList currentBudIds() { return mCurrentRecord.budIds; }
  Coordinates currentDefinitionPoint() { return mCurrentRecord.definitionPoint; }
  void showInfoAboutSelection( QStringList parIds, QStringList budIds );

signals:
  void updateHistory( HistoryRecord );
  void showParcely( QStringList );
  void showBudovy( QStringList );
  void currentParIdsChanged( bool );
  void currentBudIdsChanged( bool );
  void historyBefore( bool );
  void historyAfter( bool );
  void definitionPointAvailable( bool );

public slots:
  void processAction( const QUrl task );

  void onLinkClicked( const QUrl task );
  void goBack();
  void goForth();
  void saveHistory( HistoryRecord record );

private:

  DocumentBuilder *mDocumentBuilder;
  HistoryRecord mCurrentRecord;
  History mUrlHistory;
  History::iterator mHistoryIt;
  QUrl mCurrentUrl;

  static VfkDocument *documentFactory( ExportFormat format );
  void updateButtonEnabledState();

  TaskMap parseTask(const QUrl task);
  QString documentContent( const TaskMap taskMap, ExportFormat format );
};

#endif // VFKTEXTBROWSER_H
